/* This file contains code for things that should probably be pulled out into their own file, but I don't really have a reason
   to do right now. */

#if 0 /* Chaikin and that cool snake/sperm thing. */

v2 RawPoints[2000];
int NumRawPoints;
float TotalLineLength;

    /* Point insertion. */
    if (NumRawPoints < ArrayCount(RawPoints))
    {
        float CursorX = AppInput->CursorX;
        float CursorY = AppInput->CursorY;
        v2 CursorPos = { CursorX, CursorY };

        float MinDist = 2.5f;
        float Dist = MinDist;
        if (NumRawPoints > 0)
        {
            Dist = GetDistance(RawPoints[0], CursorPos);
        }

        if (Dist >= MinDist)
        {
            /* Add new point. */

            for(int I = NumRawPoints; I >= 1; I--)
            {
                RawPoints[I] = RawPoints[I - 1];
            }

            RawPoints[0] = { CursorX, CursorY };
            NumRawPoints++;

            TotalLineLength += Dist;
        }
    }

    /* Do Chaikin. */
#define NUM_CHAIKIN_ITERATIONS 2
    v2 *ChaikinPoints = NULL;
    int MaxNumChaikinPoints = 0;
    float TotalChaikinLength = 0.0f;
    if (NumRawPoints > 1)
    {
        MaxNumChaikinPoints = NumRawPoints;
        for (int I = 0; I < NUM_CHAIKIN_ITERATIONS; I++)
        {
            MaxNumChaikinPoints = 2*(MaxNumChaikinPoints - 1);
        }

        ChaikinPoints = (v2 *)PushPerFrameMemory(sizeof(v2)*MaxNumChaikinPoints);
        /* TODO(@memory): Figure out exact amount memory needed. */
        v2 *Buffer = (v2 *)PushPerFrameMemory(sizeof(v2)*MaxNumChaikinPoints);

        /* We implement Chaikin as follows: BufferA is the input and BufferB is the output, then BufferB is the 
           input and BufferA is the output, and so on. */

        /* Figure out which buffer should be first. Remember, we switch back and forth, so we want Points to be 
           which ever is the last output of Chaikin. This depends on the number of Chaikin iterations. */
        v2 *InputPoints, *OutputPoints;
        if (NUM_CHAIKIN_ITERATIONS % 2)
        {
            InputPoints = Buffer;
            OutputPoints = ChaikinPoints;
        }
        else
        {
            InputPoints = ChaikinPoints;
            OutputPoints = Buffer;
        }

        for (int I = 0; I < NumRawPoints; I++)
        {
            InputPoints[I] = RawPoints[I];
        }

        int NumInputPoints = NumRawPoints;
        int NumOutputPoints = 0;

        for (int ChaikinIteration = 0; ChaikinIteration < NUM_CHAIKIN_ITERATIONS; ChaikinIteration++)
        {
            line2 LastLine = { -1.0f, -1.0f };
            for (int PointIdx = 0; 
                PointIdx < NumInputPoints - 1 && 
                /* Make sure we don't overflow. */
                NumOutputPoints + 2 <= MaxNumChaikinPoints; 
                PointIdx++)
            {
                line2 Line = { InputPoints[PointIdx], InputPoints[PointIdx + 1] };
                line2 NewLine = { GetPointAtPercentage(Line, 0.25f), GetPointAtPercentage(Line, 0.75f) };

                /* If these two points are equal, then we have a perfectly overlapping line (i.e. points A->B->C where
                   A == C). In that case, just add line {A, B} and ditch {C, A}. If we continue, there will be two
                   overlapping points, which will cause our GetNormal() to crash. */
                if (LastLine.P1 != NewLine.P2)
                {
                    OutputPoints[NumOutputPoints++] = NewLine.P1;
                    if (NumOutputPoints > 1)
                    {
                        Assert(OutputPoints[NumOutputPoints - 1] != OutputPoints[NumOutputPoints - 2]);
                    }
                    OutputPoints[NumOutputPoints++] = NewLine.P2;
                    Assert(OutputPoints[NumOutputPoints - 1] != OutputPoints[NumOutputPoints - 2]);
                }

                LastLine = NewLine;
            }

            v2 *Tmp = InputPoints;
            InputPoints = OutputPoints;
            OutputPoints = Tmp;

            NumInputPoints = NumOutputPoints;
            NumOutputPoints = 0;
        }

        /* At the end, swapped input and output, so input contains output. */
        Assert(NumInputPoints <= MaxNumChaikinPoints);
        MaxNumChaikinPoints = NumInputPoints;

        PopPerFrameMemory();

        /* Get Chaikin length. */
        /* TODO: Could be faster to just do it on the raw points. */
        TotalChaikinLength = 0.0f;
        for (int I = 0; I < MaxNumChaikinPoints - 1; I++)
        {
            v2 Point = ChaikinPoints[I];
            v2 NextPoint = ChaikinPoints[I + 1];
            TotalChaikinLength += GetDistance(Point, NextPoint);
        }
    }

    /* Create Parallels. */
#define MAX_PARALLEL_LENGTH 50.0f
    /* TODO(@memory): We're not popping ChaikinPoints cause Parallels needs them, _but_ if we pushed Parallels first,
       then we'd be able to free Chaikin! */
    line2 *Parallels = NULL;
    int NumParallels = 0;
    if (MaxNumChaikinPoints > 1)
    {
        NumParallels = MaxNumChaikinPoints;
        Parallels =  (line2 *)PushPerFrameMemory(sizeof(line2)*NumParallels);
        float RunningChaikinLength = 0.0f;
        for (int ChaikinPointIdx = 0; ChaikinPointIdx < MaxNumChaikinPoints - 1; ChaikinPointIdx++)
        {
            v2 Point = ChaikinPoints[ChaikinPointIdx];
            v2 Next = ChaikinPoints[ChaikinPointIdx + 1];
            line2 Line = { Point, Next };

            float HalfLength;
            {
                float Percentage = RunningChaikinLength/TotalChaikinLength;
                float InversePercentage = 1.0f - Percentage;
                float Length = MAX_PARALLEL_LENGTH*InversePercentage;
                HalfLength = Length/2.0f;
            }

            v2 Normal = GetNormal(Line);
            Normal *= HalfLength;

            Parallels[ChaikinPointIdx].P1 = Point - Normal;
            Parallels[ChaikinPointIdx].P2 = Point + Normal;

            RunningChaikinLength += GetDistance(Point, Next);
        }

        v2 LastChaikinPoint = ChaikinPoints[MaxNumChaikinPoints - 1];
        Parallels[NumParallels - 1] = { LastChaikinPoint, LastChaikinPoint };
    }

    /* Tail decay processing. */
    /* TODO: Should we be using Chaikin points instead? Might make a difference when we move through multiple points. */
    if (NumRawPoints > 1)
    {
        /* 1) We decay the tail by moving it in the direction of next point a certain amount. However, if there's not 
           enough room between the tail and the next point for the decay, we must go to the next point, hence the loop. 
           2) We don't want fewer than 1 points. 
           3) We want the decay to end at _some_ point, so we set a minimum of 1 unit. */
        float DecayRemaining = Max(dt*(TotalChaikinLength*TotalChaikinLength)/200.0f, 1.0f);
        v2 NewLastPoint = {};
        while (NumRawPoints > 1)
        {
            v2 LastPoint = RawPoints[NumRawPoints - 1];
            v2 SecondToLastPoint = RawPoints[NumRawPoints - 2];

            float MaxDecay = GetDistance(LastPoint, SecondToLastPoint);
            float ThisIterationDecay = Min(DecayRemaining, MaxDecay);

            v2 Direction = Normalize(SecondToLastPoint - LastPoint);
            NewLastPoint = LastPoint + ThisIterationDecay*Direction;

            DecayRemaining -= ThisIterationDecay;

            /* Are we done? */
            if (IsWithinTolerance(DecayRemaining, 0.0f))
            {
                break;
            }

            NumRawPoints--;
        }

        Assert(NumRawPoints >= 1);
        Assert(NewLastPoint != V2(0.0f, 0.0f));
        RawPoints[NumRawPoints - 1] = NewLastPoint;

        /* Are the last two points the same? */
        if (RawPoints[NumRawPoints - 1] == RawPoints[NumRawPoints - 2])
        {
            NumRawPoints--;
        }
    }

#if 0
    /* Draw snake thing. */
    if (NumParallels > 0)
    {
        /* Body. */
        {
            int MaxNumVertices = 2*NumParallels;
            int VertexDataSize = 2*MaxNumVertices;
            float *VertexData = (float *)PushPerFrameMemory(sizeof(float)*VertexDataSize);
            float *CurVertexData = VertexData;
            for (int ParallelIdx = 0;
                 ParallelIdx < NumParallels &&
                 /* Make sure we don't overflow. */
                 CurVertexData + 4 <= VertexData + VertexDataSize;
                 ParallelIdx++)
            {
                line2 Parallel = Parallels[ParallelIdx];
                *CurVertexData++ = Parallel.P1.X;
                *CurVertexData++ = Parallel.P1.Y;
                *CurVertexData++ = Parallel.P2.X;
                *CurVertexData++ = Parallel.P2.Y;
            }

            /* Make sure vertices are valid. */
            for (int I = 0; I < MaxNumVertices; I += 2)
            {
                Assert(!(VertexData[I] == 0.0f && VertexData[I + 1] == 0.0f));
            }

            glBindBuffer(GL_ARRAY_BUFFER, Vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*VertexDataSize, VertexData, GL_STREAM_DRAW);
            PopPerFrameMemory();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, MaxNumVertices);
        }

#if 1
        /* Head. */
        {
#if 1
            line2 Parallel = Parallels[0];
            v2 HeadPos = GetPointAtPercentage(Parallel, 0.5f);
            AddCircle(&ShapeRenderer, HeadPos, (float)MAX_PARALLEL_LENGTH/2.0f, GLUTILS_WHITE);
            Render(&ShapeRenderer);
#else
            int NumHeadPoints = 4;
            int MaxNumVertices = NumHeadPoints + 1;
            int VertexDataSize = 2*MaxNumVertices;
            float *VertexData = (float *)PushPerFrameMemory(sizeof(float)*VertexDataSize);
            int NumVertexData = 0;
            line2 Parallel = {V2(600.0f, 500.0f), V2(600.0f, 550.0f)};
            v2 FirstParallelMid = GetPointAtPercentage(Parallel, 0.5f);
            VertexData[NumVertexData++] = FirstParallelMid.X;
            VertexData[NumVertexData++] = FirstParallelMid.Y;
            float Step = 360.0f/(float)(NumHeadPoints);
            while (NumHeadPoints--)
            {
                VertexData[NumVertexData++] = Parallel.P1.X;
                VertexData[NumVertexData++] = Parallel.P1.Y;
                RotateAroundCenter(&Parallel, Step);
            }
            glBindBuffer(GL_ARRAY_BUFFER, Vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*VertexDataSize, VertexData, GL_STREAM_DRAW);
            PopPerFrameMemory();
            glDrawArrays(GL_TRIANGLE_FAN, 0, MaxNumVertices);
#endif
        }

#if 0
        /* Eyes. */
        {
            line2 FirstParallel = Parallels[0];
            v2 Eye1 = GetPointAtPercentage(FirstParallel, 0.35f);
            v2 Eye2 = GetPointAtPercentage(FirstParallel, 0.65f);
            AddPoint(&ShapeRenderer, Eye1, 5.5f, GLUTILS_BLACK);
            AddPoint(&ShapeRenderer, Eye2, 5.5f, GLUTILS_BLACK);
            Render(&ShapeRenderer);
        }
#endif
#endif
    }
#endif

#if 0
    /* Draw debug info. */
    if (DrawDebug)
    {
        for (int I = 0; I < NumRawPoints; I++)
        {
            AddPoint(&ShapeRenderer, RawPoints[I], 5.0f, GLUTILS_BLACK);
        }
        Render(&ShapeRenderer);
    }
#endif
#endif

#if 0 /* Unfinished delaunay. */
struct delaunay_result
{
    tri2 *TriangulationTris;
    tri2 SuperTriangle;
    int NumTriangulationTris;
};
void
Free(delaunay_result *DelaunayResult)
{
    MemFree(DelaunayResult->TriangulationTris);
}
delaunay_result
DoDelaunay(v2 *Points, int NumPoints)
{
    Assert(NumPoints >= 3);

    /* TODO(@memory): Figure out exactly... not sure how. */
    /* '+ 3*NumPoints' because of the super triangle. */
    int MaxNumTriangulationTris = (NumPoints + 3*NumPoints) - 2;
    tri2 *TriangulationTris = (tri2 *)MemAlloc(sizeof(tri2)*MaxNumTriangulationTris);
    int NumTriangulationTris = 0;
    tri2 SuperTriangle = CreateSuperTriangle(Points, NumPoints);
    AddTriangle(TriangulationTris, NumTriangulationTris++, SuperTriangle,
                SuperTriangle, Points, 0);

    for (int PointIdx = 0; PointIdx < NumPoints; PointIdx++)
    {
        v2 SampleTriPoint = Points[PointIdx];
        int DebugPreNumTris = NumTriangulationTris;
        for (int TriangulationTriIdx = 0; TriangulationTriIdx < NumTriangulationTris; TriangulationTriIdx++)
        {
            tri2 TriContainingPoint = TriangulationTris[TriangulationTriIdx];
            if (IsInside(TriContainingPoint, Points[PointIdx]))
            {
                /* Remove element. */
                RemoveTriangle(TriangulationTris, NumTriangulationTris--, TriangulationTriIdx,
                               SuperTriangle, Points, PointIdx + 1);

                /* Split and add triangle. */
                {
                    tri2 SubTris[3];
                    Split(TriContainingPoint, SampleTriPoint, SubTris);

                    /* Add sub tris. */
                    for (int I = 0; I < 3; I++)
                    {
                        Assert(NumTriangulationTris <= MaxNumTriangulationTris);
                        AddTriangle(TriangulationTris, NumTriangulationTris++, SubTris[I],
                                    SuperTriangle, Points, PointIdx + 1);
                    }
                }

#if 0
                /* Flip algorithm. */
                {
                    int TrisToFlipCheck[256];
                    int NumTrisToFlipCheck = 0;
                    TrisToFlipCheck[NumTrisToFlipCheck++] = NumTriangulationTris - 1;
                    TrisToFlipCheck[NumTrisToFlipCheck++] = NumTriangulationTris - 2;
                    TrisToFlipCheck[NumTrisToFlipCheck++] = NumTriangulationTris - 3;

                    while (NumTrisToFlipCheck > 0)
                    {
                        /* Save the index and remove from queue now. */
                        int TriToFlipCheckIdx = TrisToFlipCheck[0];
                        TrisToFlipCheck[0] = TrisToFlipCheck[--NumTrisToFlipCheck];

                        /* Get triangle. */
                        tri2 TriToFlipCheck = TriangulationTris[TriToFlipCheckIdx];

                        /* Loop through original triangles to find affected ones. */
                        for (int TriangulationTriFlipIdx = 0; TriangulationTriFlipIdx < NumTriangulationTris - 3; TriangulationTriFlipIdx++)
                        {
                            /* Must not be in the queue, and must not be this one. */
                            bool DoSkip = false;
                            {
                                if (TriangulationTriFlipIdx == TriToFlipCheckIdx)
                                {
                                    DoSkip = true;
                                }
                                else
                                {
                                    for (int I = 0; I < NumTrisToFlipCheck; I++)
                                    {
                                        if (TriangulationTriFlipIdx == TrisToFlipCheck[I])
                                        {
                                            DoSkip = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (DoSkip)
                            {
                                continue;
                            }

                            tri2 TriangulationTriFlip = TriangulationTris[TriangulationTriFlipIdx];
                            line2 CommonEdge;
                            if (GetCommonEdge(TriToFlipCheck, TriangulationTriFlip, &CommonEdge))
                            {
                                v2 TriFlipCheckNonCommonEdgePoint = GetPointExcluding(TriToFlipCheck, CommonEdge);
                                v2 TriNonCommonEdgePoint = GetPointExcluding(TriangulationTriFlip, CommonEdge);
                                line2 NewCommonEdge = LINE2(TriNonCommonEdgePoint, TriFlipCheckNonCommonEdgePoint);

                                /* Do we need to flip? */
                                bool DoFlip = false;
                                {
                                    /* Nothing to do with super triangles. Figure out if we need to flip it the normal way. */

                                    /* We need to calculate the angle opposite the common edge. */
                                    line2 NewTriNonCommonEdges[2];
                                    line2 TriangulationTriFlipNonCommonEdges[2];
                                    GetEdgesExcluding(TriToFlipCheck, CommonEdge, NewTriNonCommonEdges);
                                    GetEdgesExcluding(TriangulationTriFlip, CommonEdge, TriangulationTriFlipNonCommonEdges);

                                    /* Ensure correct order. This means the non common edge should be first. Necessary for direction. */
                                    EnsureEndPointIsFirst(&TriangulationTriFlipNonCommonEdges[0], TriNonCommonEdgePoint);
                                    EnsureEndPointIsFirst(&TriangulationTriFlipNonCommonEdges[1], TriNonCommonEdgePoint);
                                    EnsureEndPointIsFirst(&NewTriNonCommonEdges[0], TriFlipCheckNonCommonEdgePoint);
                                    EnsureEndPointIsFirst(&NewTriNonCommonEdges[1], TriFlipCheckNonCommonEdgePoint);

                                    v2 NewTriNonCommonEdgeDir0 = GetDirection(NewTriNonCommonEdges[0]);
                                    v2 NewTriNonCommonEdgeDir1 = GetDirection(NewTriNonCommonEdges[1]);
                                    v2 TriangulationTriFlipNonCommonEdgeDir0 = GetDirection(TriangulationTriFlipNonCommonEdges[0]);
                                    v2 TriangulationTriFlipNonCommonEdgeDir1 = GetDirection(TriangulationTriFlipNonCommonEdges[1]);

                                    float Alpha = GetAngleBetween(NewTriNonCommonEdgeDir0, NewTriNonCommonEdgeDir1);
                                    float Beta = GetAngleBetween(TriangulationTriFlipNonCommonEdgeDir0, TriangulationTriFlipNonCommonEdgeDir1);
                                    float Sum = Alpha + Beta;
                                    if (Sum > M_PI)
                                    {
                                        DoFlip = true;
                                    }
                                }

                                if (DoFlip)
                                {
                                    /* DJ, flip that shit! */

                                    /* Replace the old triangles. */
                                    tri2 OlderTriReplacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P1);
                                    tri2 NewerTriReplacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P2);
                                    ReplaceTriangle(TriangulationTris, NumTriangulationTris, TriangulationTriFlipIdx, OlderTriReplacement,
                                                    SuperTriangle, Points, PointIdx + 1);
                                    ReplaceTriangle(TriangulationTris, NumTriangulationTris, TriToFlipCheckIdx, NewerTriReplacement,
                                                    SuperTriangle, Points, PointIdx + 1);

                                    /* Add these triangles to the queue if not already in. */

                                    if (!CheckArrayContains(TrisToFlipCheck, NumTrisToFlipCheck, TriangulationTriFlipIdx))
                                    {
                                        Assert(NumTrisToFlipCheck < ArrayCount(TrisToFlipCheck));
                                        TrisToFlipCheck[NumTrisToFlipCheck++] = TriangulationTriFlipIdx;
                                    }

                                    if (!CheckArrayContains(TrisToFlipCheck, NumTrisToFlipCheck, TriToFlipCheckIdx))
                                    {
                                        Assert(NumTrisToFlipCheck < ArrayCount(TrisToFlipCheck));
                                        TrisToFlipCheck[NumTrisToFlipCheck++] = TriToFlipCheckIdx;
                                    }
                                }
                            }
                        }
                    }
                }
#endif

                /* Found triangle point is in. Break. */
                break;
            }
        }

        /* Point should be in at least one triangle. */
        Assert(NumTriangulationTris != DebugPreNumTris);
    }

    /* Flip triangles related to super triangle so we have a convex hull. */
    {
        /* Ensure super triangle vertices are in expected order. */
        v2 SuperTriTop, SuperTriBottomLeft, SuperTriBottomRight;
        {
            /* Get top. */
            if (SuperTriangle.P1.Y > SuperTriangle.P2.Y && SuperTriangle.P1.Y > SuperTriangle.P3.Y) { SuperTriTop = SuperTriangle.P1; }
            else if (SuperTriangle.P2.Y > SuperTriangle.P1.Y && SuperTriangle.P2.Y > SuperTriangle.P3.Y) { SuperTriTop = SuperTriangle.P2; }
            else { SuperTriTop = SuperTriangle.P3; }

            /* Get bottom left. */
            if (SuperTriangle.P1.X < SuperTriangle.P2.X && SuperTriangle.P1.X < SuperTriangle.P3.X) { SuperTriBottomLeft = SuperTriangle.P1; }
            else if (SuperTriangle.P2.X < SuperTriangle.P1.X && SuperTriangle.P2.X < SuperTriangle.P3.X) { SuperTriBottomLeft = SuperTriangle.P2; }
            else { SuperTriBottomLeft = SuperTriangle.P3; }

            /* Get bottom right. */
            if (SuperTriangle.P1.X > SuperTriangle.P2.X && SuperTriangle.P1.X > SuperTriangle.P3.X) { SuperTriBottomRight = SuperTriangle.P1; }
            else if (SuperTriangle.P2.X > SuperTriangle.P1.X && SuperTriangle.P2.X > SuperTriangle.P3.X) { SuperTriBottomRight = SuperTriangle.P2; }
            else { SuperTriBottomRight = SuperTriangle.P3; }
        }
#define SUPER_TRIANGLE_TOP 0
#define SUPER_TRIANGLE_BOTTOM_LEFT 1
#define SUPER_TRIANGLE_BOTTOM_RIGHT 2
        SuperTriangle.Verts[SUPER_TRIANGLE_TOP] = SuperTriTop;
        SuperTriangle.Verts[SUPER_TRIANGLE_BOTTOM_LEFT] = SuperTriBottomLeft;
        SuperTriangle.Verts[SUPER_TRIANGLE_BOTTOM_RIGHT] = SuperTriBottomRight;

#define SUPER_TRIANGLE_TOP_TO_BOTTOM_LEFT 0
#define SUPER_TRIANGLE_BOTTOM_LEFT_TO_BOTTOM_RIGHT 1
#define SUPER_TRIANGLE_BOTTOM_RIGHT_TO_TOP 2
        v2 SuperTriEdgesDirs[3];
        SuperTriEdgesDirs[SUPER_TRIANGLE_TOP_TO_BOTTOM_LEFT] = GetDirection(LINE2(SuperTriTop, SuperTriBottomLeft));
        SuperTriEdgesDirs[SUPER_TRIANGLE_BOTTOM_LEFT_TO_BOTTOM_RIGHT] = GetDirection(LINE2(SuperTriBottomLeft, SuperTriBottomRight));
        SuperTriEdgesDirs[SUPER_TRIANGLE_BOTTOM_RIGHT_TO_TOP] = GetDirection(LINE2(SuperTriBottomRight, SuperTriTop));

        for (int SuperTriVertsIdx = 0; SuperTriVertsIdx < 3; SuperTriVertsIdx++)
        {
            v2 SuperTriVert = SuperTriangle.Verts[SuperTriVertsIdx];

            /* Get all triangles containing this vertex sorted on their angle. */
            int SuperTriVertTris[128];
            int NumSuperTriVertTris = 0;
            {
                /* Get all triangles containing this vertex. */
                for (int TriangulationTrisIdx = 0; TriangulationTrisIdx < NumTriangulationTris; TriangulationTrisIdx++)
                {
                    tri2 TriangulationTri = TriangulationTris[TriangulationTrisIdx];
                    if (CheckContainsVertex(TriangulationTri, SuperTriVert))
                    {
                        Assert(NumSuperTriVertTris < ArrayCount(SuperTriVertTris));
                        SuperTriVertTris[NumSuperTriVertTris++] = TriangulationTrisIdx;
                    }
                }
                Assert(NumSuperTriVertTris > 0);

                /* Calculate their angles. */
                float SuperTriVertTrisAngles[128] = {}; /* This array is parallel to the array containing the triangles for this super triangle vertex. */
                for (int SuperTriVertTrisIdx = 0; SuperTriVertTrisIdx < NumSuperTriVertTris; SuperTriVertTrisIdx++)
                {
                    tri2 SuperVertTri = TriangulationTris[SuperTriVertTris[SuperTriVertTrisIdx]];
                    line2 Edges[3];
                    GetEdges(SuperVertTri, Edges);
                    for (int EdgeIdx = 0; EdgeIdx < ArrayCount(Edges); EdgeIdx++)
                    {
                        if (CheckContainsVertex(Edges[EdgeIdx], SuperTriVert))
                        {
                            EnsureEndPointIsFirst(&Edges[EdgeIdx], SuperTriVert);
                            v2 EdgeDir = GetDirection(Edges[EdgeIdx]);
                            float AngleBetween = GetAngleBetween(SuperTriEdgesDirs[SuperTriVertsIdx], EdgeDir); /* We shouldn't have to specify direction because it's never greater than 180 degrees. */
                            if (AngleBetween > SuperTriVertTrisAngles[SuperTriVertTrisIdx])
                            {
                                SuperTriVertTrisAngles[SuperTriVertTrisIdx] = AngleBetween;
                            }
                        }
                    }
                }

                /* Now sort them. */
                for (int NumSort = NumSuperTriVertTris; NumSort > 0; NumSort--)
                {
                    /* Get max. */
                    int MaxIdx = 0;
                    for (int SortIdx = 0; SortIdx < NumSort; SortIdx++)
                    {
                        if (SuperTriVertTrisAngles[SortIdx] > SuperTriVertTrisAngles[MaxIdx])
                        {
                            MaxIdx = SortIdx;
                        }
                    }

                    /* Swap max and end angles. */
                    {
                        float Tmp = SuperTriVertTrisAngles[NumSort - 1];
                        SuperTriVertTrisAngles[NumSort - 1] = SuperTriVertTrisAngles[MaxIdx];
                        SuperTriVertTrisAngles[MaxIdx] = Tmp;
                    }

                    /* Swap max and end super triangle vertex tris. */
                    {
                        int Tmp = SuperTriVertTris[NumSort - 1];
                        SuperTriVertTris[NumSort - 1] = SuperTriVertTris[MaxIdx];
                        SuperTriVertTris[MaxIdx] = Tmp;
                    }
                }
            }

            for (int SuperTriVertTrisIdx = 1; SuperTriVertTrisIdx < NumSuperTriVertTris - 2; /* Skip first and last. We know it's not possible. */
                 SuperTriVertTrisIdx++)
            {
                int Tri0Idx = SuperTriVertTris[SuperTriVertTrisIdx];
                int Tri1Idx = SuperTriVertTris[SuperTriVertTrisIdx + 1];
                tri2 Tri0 = TriangulationTris[Tri0Idx];
                tri2 Tri1 = TriangulationTris[Tri1Idx];

                line2 CommonEdge;
                bool HasCommonEdge = GetCommonEdge(Tri0, Tri1, &CommonEdge);
                Assert(HasCommonEdge);
                v2 Tri0NonCommonEdgeVert = GetVertexExcluding(Tri0, CommonEdge);
                v2 Tri1NonCommonEdgeVert = GetVertexExcluding(Tri1, CommonEdge);
                line2 NewCommonEdge = LINE2(Tri0NonCommonEdgeVert, Tri1NonCommonEdgeVert);

                /* We only care if the new common edge contains no super triangle vertices; otherwise,
                   we'd flip into the same situation (one vertex not in super triangle, and one in). */
                if (!CheckContainsAnyVerticesOf(SuperTriangle, NewCommonEdge))
                {
                    v2 VertexInSuperTri, VertexNotInSuperTri;
                    if (CheckContainsOneVertexOf(SuperTriangle, CommonEdge, &VertexInSuperTri, &VertexNotInSuperTri))
                    {
                        tri2 Tris[] = { Tri0, Tri1 };
                        float TotalAngle = 0.0f;
                        for (int I = 0; I < ArrayCount(Tris); I++)
                        {
                            /* Get the angle between the common edge and the edge that does not contain the
                               super triangle vertex. */

                            line2 NoSuperTriVertEdge = GetEdgeExcluding(Tris[I], VertexInSuperTri);

                            /* Make sure the shared vertex between the edges is the first for both. */
                            EnsureEndPointIsFirst(&NoSuperTriVertEdge, VertexNotInSuperTri);
                            EnsureEndPointIsFirst(&CommonEdge, VertexNotInSuperTri);

                            v2 NewTriDir = GetDirection(NoSuperTriVertEdge);
                            v2 CommonEdgeDir = GetDirection(CommonEdge);

                            float Angle = GetAngleBetween(NewTriDir, CommonEdgeDir);
                            TotalAngle += Angle;
                        }

                        /* Can we flip this edge and still have a triangle? */
                        if (TotalAngle < M_PI)
                        {
                            /* DJ, flip that shit! */

                            /* Replace the old triangles with new ones. */
                            tri2 Tri0Replacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P1);
                            tri2 Tri1Replacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P2);
                            ReplaceTriangle(TriangulationTris, NumTriangulationTris, Tri0Idx, Tri0Replacement,
                                            SuperTriangle, Points, NumPoints);
                            ReplaceTriangle(TriangulationTris, NumTriangulationTris, Tri1Idx, Tri1Replacement,
                                            SuperTriangle, Points, NumPoints);
                        }
                    }
                }
            }
        }
    }

    /* Extra debugging: if we were passed 3 points, make sure that triangle is in the triangulation.  */
    if (NumPoints == 3)
    {
        tri2 OnlyTri = TRI2(Points[0], Points[1], Points[2]);
        bool ContainsOnlyTri = false;
        for (int I = 0; I < NumTriangulationTris; I++)
        {
            if (TriangulationTris[I] == OnlyTri)
            {
                ContainsOnlyTri = true;
                break;
            }
        }
        Assert(ContainsOnlyTri);
    }

    /* Remove super triangle triangles. */
    for (int I = 0; I < NumTriangulationTris;)
    {
        if (HasACommonVertex(TriangulationTris[I], SuperTriangle))
        {
            /* Remove. */
            Assert(NumTriangulationTris > 0);
            RemoveTriangle(TriangulationTris, NumTriangulationTris--, I,
                           SuperTriangle, Points, NumPoints);
        }
        else
        {
            I++;
        }
    }

    /* Make sure we didn't remove all of them. */
    Assert(NumTriangulationTris != 0);

    /* We're done! */

    delaunay_result Result;
    Result.TriangulationTris = TriangulationTris;
    Result.NumTriangulationTris = NumTriangulationTris;
    Result.SuperTriangle = SuperTriangle;
    return Result;
}
#endif

#if 0 /* Slow way of creating triangulation. This is n^3. Not only slow, but it's wrong too. Doesn't handle all cases. */
triangulation_result CreateTriangulationSlow(v2 *Points, int NumPoints,
                                             line2 *ConstraintEdges, int NumConstraintEdges)
{
    Assert(NumPoints >= 3);
    Assert(!CheckForDuplicates(Points, NumPoints));

    /* TODO(@memory): Figure out exactly... not sure how. */
    int MaxNumTriangulationTris = 10*NumPoints;
    tri2 *TriangulationTris = (tri2 *)MemAlloc(sizeof(tri2)*MaxNumTriangulationTris);
    int NumTriangulationTris = 0;

    /* Copy points since we need to sort them. */
    int PointsSize = sizeof(v2)*NumPoints;
    v2 *SortedPoints = (v2 *)MemAlloc(PointsSize);
    memcpy(SortedPoints, Points, PointsSize);

    /* Sort points. */
    {
        /* Sort on X-coord. */
        SortArray(SortedPoints, NumPoints, [](v2 A, v2 B) -> bool { return A.X > B.X; });

        /* To make things nice, we also want to sort vertices with same X-Coord on Y-Coord. */
        for (int StartIdx = 0; StartIdx < NumPoints - 1;)
        {
            int EndIdx; /* This is exclusive. */
            for (EndIdx = StartIdx + 1; 
                 EndIdx < NumPoints && SortedPoints[EndIdx].X == SortedPoints[StartIdx].X; 
                 EndIdx++);

            int NumPointsSameXCoord = EndIdx - StartIdx;

            /* Did we find more than one? */
            if (NumPointsSameXCoord > 1)
            {
                SortArray(&SortedPoints[StartIdx], NumPointsSameXCoord, [](v2 A, v2 B) -> bool { return A.Y > B.Y; });
            }

            StartIdx = EndIdx;
        }
    }

    /* If the first 3 or more points have the same X coordinate, then if we just create the first triangle with the first 3
       points, we'll have a collinear triangle, which is bad. Instead, we add a point to the left and remove those triangles
       at the end of the algorithm. */
    tri2 FirstTri;
    int FirstPointIdx;
    v2 DummyPoint = {};
    bool HasDummyPoint = SortedPoints[0].X == SortedPoints[1].X && SortedPoints[1].X == SortedPoints[2].X;
    if (HasDummyPoint)
    {
        DummyPoint = V2(SortedPoints[0].X - 50.0f, SortedPoints[0].Y); /* 50.0f is arbitrary. */
        FirstTri = TRI2(DummyPoint, SortedPoints[0], SortedPoints[1]);
        FirstPointIdx = 2;
    }
    else
    {
        /* Add first tri. */
        FirstTri = TRI2(SortedPoints[0], SortedPoints[1], SortedPoints[2]);
        FirstPointIdx = 3;
    }
    TriangulationTris[NumTriangulationTris++] = FirstTri;

    /* Add each point to triangulation. */
    for (int NewPointIdx = FirstPointIdx; NewPointIdx < NumPoints; NewPointIdx++)
    {
        v2 NewPoint = SortedPoints[NewPointIdx];

        int DebugOriginalNumTris = NumTriangulationTris;

        /* @debug: ensure none of the triangles contain this new point. */
        for (int TriangulationTrisIdx = 0; TriangulationTrisIdx < NumTriangulationTris; TriangulationTrisIdx++)
        {
            tri2 TriangulationTri = TriangulationTris[TriangulationTrisIdx];
            Assert(!CheckContainsSameVertex(TriangulationTri, NewPoint));
        }

        /* Go through each triangle so we can go through each vertex. Since we'll be adding triangles, we don't want to process
           the ones we add. */
        for (int TriangulationTrisIdx = 0, OriginalNumTriangulationTris = NumTriangulationTris; TriangulationTrisIdx < OriginalNumTriangulationTris; TriangulationTrisIdx++)
        {
            tri2 TriangulationTri = TriangulationTris[TriangulationTrisIdx];

            /* Ensure none of the triangles contain this new point. */
            Assert(!CheckContainsSameVertex(TriangulationTri, NewPoint));

            /* Get number of points in tri that can see the new point. */
            v2 TriPointsThatCanSee[3];
            int NumTriPointsThatCanSee = 0;
            {
                for (int TriangulationTriVertIdx = 0; TriangulationTriVertIdx < ArrayCount(TriangulationTri.Verts); TriangulationTriVertIdx++)
                {
                    v2 TriangulationTriVert = TriangulationTri.Verts[TriangulationTriVertIdx];
                    line2 NewPointToTriPointEdge = LINE2(NewPoint, TriangulationTriVert);
                    bool ClearPath = true;
                    {
                        for (int CPTriangulationTrisIdx = 0; CPTriangulationTrisIdx < OriginalNumTriangulationTris; CPTriangulationTrisIdx++)
                        {
                            tri2 CPTriangulationTri = TriangulationTris[CPTriangulationTrisIdx];
                            line2 CPTriEdges[3];
                            GetEdges(CPTriangulationTri, CPTriEdges);
                            for (int EdgeIdx = 0; EdgeIdx < ArrayCount(CPTriEdges); EdgeIdx++)
                            {
                                line2 CPTriEdge = CPTriEdges[EdgeIdx];
                                if (Intersects(CPTriEdge, NewPointToTriPointEdge))
                                {
                                    ClearPath = false;
                                    goto ClearPathFinish;
                                }
                            }
                        }
                    }
ClearPathFinish:
                    if (ClearPath)
                    {
                        TriPointsThatCanSee[NumTriPointsThatCanSee++] = TriangulationTriVert;
                    }
                    else
                    {
                        /* Go to next triangle vertex. */
                        continue;
                    }
                }
            }
            if (NumTriPointsThatCanSee <= 1)
            {
                /* Go to next triangle. */
                continue;
            }

            /* Let's make some triangles! */

            tri2 TrisToAdd[2];
            int NumTrisToAdd = 0;
            if (NumTriPointsThatCanSee == 2)
            {
                tri2 NewTri = TRI2(TriPointsThatCanSee[0], TriPointsThatCanSee[1], NewPoint);
                TrisToAdd[NumTrisToAdd++] = NewTri;
            }
            else if (NumTriPointsThatCanSee == 3)
            {
                /* Rather large pain in the ass. */

                v2 NegPathToSomeTriVert = -Normalize(TriPointsThatCanSee[0] - NewPoint);

                /* Sort on angle. */
                {
                    float Angles[3];
                    for (int TriPointsThatCanSeeIdx = 0; TriPointsThatCanSeeIdx < ArrayCount(TriPointsThatCanSee); TriPointsThatCanSeeIdx++)
                    {
                        v2 PathToTriVert = Normalize(TriPointsThatCanSee[TriPointsThatCanSeeIdx] - NewPoint);
                        float Angle = GetAngleBetween(NegPathToSomeTriVert, PathToTriVert, AngleDirection_CW); /* Angle direction doesn't matter, but must be the same. */
                        Angles[TriPointsThatCanSeeIdx] = Angle;
                    }

                    /* Sort on angle. */
                    for (int SortSize = ArrayCount(TriPointsThatCanSee); SortSize > 0; SortSize--)
                    {
                        /* Find largest. */
                        int LargestIdx = 0;
                        for (int I = 0; I < SortSize; I++)
                        {
                            if (Angles[I] > Angles[LargestIdx])
                            {
                                LargestIdx = I;
                            }
                        }

                        /* Swap max and end. */
                        {
                            float Tmp = Angles[SortSize - 1];
                            Angles[SortSize - 1] = Angles[LargestIdx];
                            Angles[LargestIdx] = Tmp;
                        }
                        {
                            v2 Tmp = TriPointsThatCanSee[SortSize - 1];
                            TriPointsThatCanSee[SortSize - 1] = TriPointsThatCanSee[LargestIdx];
                            TriPointsThatCanSee[LargestIdx] = Tmp;
                        }
                    }
                }

                /* Add those bitches. */
                tri2 NewTri1 = TRI2(TriPointsThatCanSee[0], TriPointsThatCanSee[1], NewPoint);
                tri2 NewTri2 = TRI2(TriPointsThatCanSee[1], TriPointsThatCanSee[2], NewPoint);
                TrisToAdd[NumTrisToAdd++] = NewTri1;
                TrisToAdd[NumTrisToAdd++] = NewTri2;
            }

            for (int TrisToAddIdx = 0; TrisToAddIdx < NumTrisToAdd; TrisToAddIdx++)
            {
                tri2 TriToAdd = TrisToAdd[TrisToAddIdx];
                Assert(!CheckCollinear(TriToAdd));

                /* It's possible for the triangle we're adding to contain another triangle (pic0). We don't want to add it
                   if that's the case. */
                bool ContainsAnotherTriangle = false;
                {
                    /* Any single edge shouldn't be shared by more than 2 triangles. */
                    line2 OldEdge = GetEdgeExcluding(TriToAdd, NewPoint);
                    v2 OldEdgeCenter = GetCenter(OldEdge);
                    line2 NewPointToOldEdgeCenter = LINE2(NewPoint, OldEdgeCenter);
                    for (int TrisWithEdgeTriIdx = 0; TrisWithEdgeTriIdx < NumTriangulationTris; TrisWithEdgeTriIdx++)
                    {
                        tri2 Tri = TriangulationTris[TrisWithEdgeTriIdx];
                        line2 Edges[3];
                        GetEdges(Tri, Edges);
                        for (int EdgeIdx = 0; EdgeIdx < ArrayCount(Edges); EdgeIdx++)
                        {
                            line2 Edge = Edges[EdgeIdx];
                            if (Edge != OldEdge)
                            {
                                if (Intersects(NewPointToOldEdgeCenter, Edge))
                                {
                                    ContainsAnotherTriangle = true;
                                    goto ContainsAnotherTriangleEnd;
                                }
                            }
                        }
                    }
                }
ContainsAnotherTriangleEnd:
                if (!ContainsAnotherTriangle)
                {
                    /* Then add it. */

                    Assert(NumTriangulationTris < MaxNumTriangulationTris);
                    /* @debug: ensure each edge is shared by at most 2 triangles. */
                    {
                        for (int TriEdgeIdx = 0; TriEdgeIdx < 3; TriEdgeIdx++)
                        {
                            int DebugNumSimilarEdges = 0;
                            line2 TriEdge = GetEdge(TriToAdd, TriEdgeIdx);
                            for (int DebugTriIdx = 0; DebugTriIdx < NumTriangulationTris; DebugTriIdx++)
                            {
                                tri2 DebugTri = TriangulationTris[DebugTriIdx];
                                for (int DebugTriEdgeIdx = 0; DebugTriEdgeIdx < 3; DebugTriEdgeIdx++)
                                {
                                    line2 DebugTriEdge = GetEdge(DebugTri, DebugTriEdgeIdx);
                                    if (TriEdge == DebugTriEdge)
                                    {
                                        DebugNumSimilarEdges++;
                                        Assert(DebugNumSimilarEdges <= 2);
                                    }
                                }
                            }
                        }
                    }
                    TriangulationTris[NumTriangulationTris++] = TriToAdd;
                    Assert(!CheckForDuplicates(TriangulationTris, NumTriangulationTris));
                }
            }
        }

        /* Ensure at least one triangle was added. Else this point isn't being used at all, which is a bug. */
        Assert(DebugOriginalNumTris != NumTriangulationTris);
    }
    Assert(!CheckForDuplicates(TriangulationTris, NumTriangulationTris));

    /* Process dummy point if necessary */
    if (HasDummyPoint)
    {
        for (int TriIdx = 0; TriIdx < NumTriangulationTris; TriIdx++)
        {
            tri2 Tri = TriangulationTris[TriIdx];
            if (CheckContainsVertex(Tri, DummyPoint))
            {
                /* Remove this triangle. */
                Assert(TriangulationTris > 0);
                TriangulationTris[TriIdx] = TriangulationTris[--NumTriangulationTris];
            }
        }
    }

#if 1
    /* Add constraint edges. */
    {
        /* Do constrained triangulation with convex hull. */
        for (int ConstraintEdgesIdx = 0; ConstraintEdgesIdx < NumConstraintEdges; ConstraintEdgesIdx++)
        {
            line2 ConstraintEdge = ConstraintEdges[ConstraintEdgesIdx];

            /* Check if this edge is already an edge. */
            bool AlreadyAnEdge = false;
            for (int TriIdx = 0; TriIdx < NumTriangulationTris; TriIdx++)
            {
                if (CheckContainsEdge(TriangulationTris[TriIdx], ConstraintEdge))
                {
                    AlreadyAnEdge = true;
                    break;
                }
            }
            if (AlreadyAnEdge)
            {
                /* Go to next edge. */
                continue;
            }

            /* TODO(@memory) */
            v2 ConstraintPolygonHalves[2][128] = {};
            int NumConstraintPolygonVertices[2] = {};
            /* Find each triangle that intersects constraint edge. */
            for (int TriIdx = 0; TriIdx < NumTriangulationTris;)
            {
                tri2 Tri = TriangulationTris[TriIdx];
                line2 TriEdges[3];
                GetEdges(Tri, TriEdges);
                bool DoesIntersect = false;
                for (int TriEdgesIdx = 0; TriEdgesIdx < ArrayCount(TriEdges); TriEdgesIdx++)
                {
                    line2 TriEdge = TriEdges[TriEdgesIdx];
                    if (!HasSameVertex(TriEdge, ConstraintEdge))
                    {
                        if (Intersects(TriEdge, ConstraintEdge))
                        {
                            DoesIntersect = true;
                            break;
                        }
                    }
                }
                if (DoesIntersect)
                {
                    /* Save the vertices of the entire triangle - these are part of constraint polygon. */
                    for (int TriVertsIdx = 0; TriVertsIdx < 3; TriVertsIdx++)
                    {
                        v2 TriVert = Tri.Verts[TriVertsIdx];

                        /* Is this vertex part of the constraint edge? */
                        if (CheckContains(ConstraintEdge, TriVert))
                        {
                            /* Then it belongs to both polygons (equally). */

                            /* It's possible for this to be duplicate. This happens when multiple edges make up this constraint edge. 
                               Instead of being clever (like merging collinear edges, which I'm not sure is possible in all cases), 
                               we just don't add the duplicate and and re-triangulate. */

                            for (int HalfIdx = 0; HalfIdx < ArrayCount(ConstraintPolygonHalves); HalfIdx++)
                            {
                                if (!CheckArrayContains(ConstraintPolygonHalves[HalfIdx], NumConstraintPolygonVertices[HalfIdx], TriVert))
                                {
                                    Assert(NumConstraintPolygonVertices[HalfIdx] < ArrayCount(ConstraintPolygonHalves[HalfIdx]));
                                    ConstraintPolygonHalves[HalfIdx][NumConstraintPolygonVertices[HalfIdx]++] = TriVert;
                                }
                            }
                        }
                        else
                        {
                            /* Determine what half the vertex belongs to. */
                            v2 ConstraintEdgeDir = GetDirection(ConstraintEdge);
                            v2 TriVertDir = Normalize(TriVert - ConstraintEdge.P1);
                            float TriVertCrossConstraintEdge = DoCross(ConstraintEdgeDir, TriVertDir);
                            int PolygonHalfIdx = TriVertCrossConstraintEdge < 0.0f ? 0 : 1;
                            if (!CheckArrayContains(ConstraintPolygonHalves[PolygonHalfIdx], NumConstraintPolygonVertices[PolygonHalfIdx], TriVert))
                            {
                                Assert(NumConstraintPolygonVertices[PolygonHalfIdx] < ArrayCount(ConstraintPolygonHalves[PolygonHalfIdx]));
                                ConstraintPolygonHalves[PolygonHalfIdx][NumConstraintPolygonVertices[PolygonHalfIdx]++] = TriVert;
                            }
                        }
                    }

                    /* Remove the triangle from the triangulation. */
                    Assert(NumTriangulationTris > 0);
                    TriangulationTris[TriIdx] = TriangulationTris[--NumTriangulationTris];
                }
                else
                {
                    TriIdx++;
                }
            }

            /* At this point, the constraint polygons have been created. */

            /* Triangulate polygon halves and add them. */
            for (int ConstraintPolygonHalfIdx = 0; ConstraintPolygonHalfIdx < 2; ConstraintPolygonHalfIdx++)
            {
                /* Check if the edge didn't intersect any other edges - this, sir, is a bug! The triangulation
                   forms a convex hull, so... */
                Assert(NumConstraintPolygonVertices[ConstraintPolygonHalfIdx] != 0);

                Assert(NumConstraintPolygonVertices[ConstraintPolygonHalfIdx] >= 3);
                triangulation_result ConstraintPolygonTriangulationResult = CreateTriangulation(ConstraintPolygonHalves[ConstraintPolygonHalfIdx],
                                                                                                NumConstraintPolygonVertices[ConstraintPolygonHalfIdx],
                                                                                                NULL, 0);

                /* Now add them! */
                for (int ConstraintPolygonTriangulationTriIdx = 0; ConstraintPolygonTriangulationTriIdx < ConstraintPolygonTriangulationResult.NumTriangulationTris; ConstraintPolygonTriangulationTriIdx++)
                {
                    Assert(NumTriangulationTris < MaxNumTriangulationTris);
                    tri2 NewTri = ConstraintPolygonTriangulationResult.TriangulationTris[ConstraintPolygonTriangulationTriIdx];

                    bool AddTriangle = true;
                    {
                        /* We need to make sure it doesn't intersect any existing triangles. */
                        line2 NewTriEdges[3];
                        GetEdges(NewTri, NewTriEdges);
                        for (int NewTriEdgeIdx = 0; NewTriEdgeIdx < ArrayCount(NewTriEdges); NewTriEdgeIdx++)
                        {
                            line2 NewTriEdge = NewTriEdges[NewTriEdgeIdx];
                            for (int TriangulationTriIdx = 0; TriangulationTriIdx < NumTriangulationTris; TriangulationTriIdx++)
                            {
                                tri2 TriangulationTri = TriangulationTris[TriangulationTriIdx];
                                line2 TriangulationTriEdges[3];
                                GetEdges(TriangulationTri, TriangulationTriEdges);
                                for (int TriangulationTriEdgeIdx = 0; TriangulationTriEdgeIdx < ArrayCount(TriangulationTriEdges); TriangulationTriEdgeIdx++)
                                {
                                    line2 TriangulationTriEdge = TriangulationTriEdges[TriangulationTriEdgeIdx];

                                    /* Are they the same? */
                                    if (NewTriEdge == TriangulationTriEdge)
                                    {
                                        /* For valid edges, there should only be at most one other similar edge in the triangulation.
                                           If this is an invalid edge, there may be more - this isn't a good way of checking
                                           for invalid edges though, since it doesn't catch all invalid edges - the intersection does. */
                                    }
                                    /* Do they intersect? */
                                    else if (Intersects(NewTriEdge, TriangulationTriEdge))
                                    {
                                        /* Then we don't want to add it. */
                                        AddTriangle = false;
                                        goto AddTriangleEnd;
                                    }
                                }
                            }
                        }
                    }
AddTriangleEnd:
                    if (AddTriangle)
                    {
                        TriangulationTris[NumTriangulationTris++] = NewTri;
                    }
                }

                /* Now free them! */
                MemFree(ConstraintPolygonTriangulationResult.TriangulationTris);
            }
        }
    }
#endif

    triangulation_result Result;
    Result.TriangulationTris = TriangulationTris;
    Result.NumTriangulationTris = NumTriangulationTris;
    return Result;
}
#endif