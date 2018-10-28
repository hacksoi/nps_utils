/* This file contains code for things that should probably be pulled out into their own file, but I don't really have a reason
   to do right now. */

#if 0 /* Chaikin */

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