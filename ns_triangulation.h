#ifndef NS_TRIANGULATION_H
#define NS_TRIANGULATION_H

#include "ns_game_math.h"

/* This function assumes there's already triangles in the triangulation for it to use. For example, a super triangle.
   Note that if you pass a convex hull, the convex hull should contain all points in the convex hull i.e. the shortest distances. */
void CreateTriangulationFrom(tri2 *TriangulationTris, int MaxNumTriangulationTris, int *NumTriangulationTris_InOut, v2 *Points, int NumPoints)
{
    int NumTriangulationTris = *NumTriangulationTris_InOut;

    /* Do regular super-triangle triangulation. */
    for (int PointIdx = 0; PointIdx < NumPoints; PointIdx++)
    {
        v2 Point = Points[PointIdx];
        int DebugPreNumTris = NumTriangulationTris;
        for (int TriangulationTriIdx = 0; TriangulationTriIdx < NumTriangulationTris; TriangulationTriIdx++)
        {
            tri2 TriContainingPoint = TriangulationTris[TriangulationTriIdx];
            bool IsPointOnEdge;
            if (CheckInside(Point, TriContainingPoint, &IsPointOnEdge))
            {
                if (IsPointOnEdge)
                {
                    /* Instead of splitting the triangle into three, we should only split it into two, otherwise we get collinear
                       triangle nightmares. */

                    /* Find other triangle. */
                    int OtherTriIdx = -1;
                    for (int OtherTriTriangulationIdx = TriangulationTriIdx + 1; OtherTriTriangulationIdx < NumTriangulationTris; OtherTriTriangulationIdx++)
                    {
                        tri2 OnEdgeTri = TriangulationTris[OtherTriTriangulationIdx];
                        if (CheckInside(Point, OnEdgeTri, &IsPointOnEdge))
                        {
                            if (IsPointOnEdge)
                            {
                                /* Found it! */
                                OtherTriIdx = OtherTriTriangulationIdx;
                                break;
                            }
                        }
                    }
                    /* There should be another triangle. A point should never be on the outline of the polygon since we're
                       either triangulating the convex hull - in which case, there can't be a point on the super triangle -
                       or we're triangulating the the rest of the points inside the convex hull - there should be no points
                       on the convex hull, because they should've been part of the convex hull points! */
                    Assert(OtherTriIdx != -1);

                    tri2 OtherTri = TriangulationTris[OtherTriIdx]; 

                    /* Now find their common edge. */
                    line2 CommonEdge;
                    bool HasCommonEdge = CheckContainsCommonEdge(TriContainingPoint, OtherTri, &CommonEdge);
                    Assert(HasCommonEdge);

                    /* Now split them. */
                    int TrisToSplit[] = {TriangulationTriIdx, OtherTriIdx};
                    for (int TrisToSplitIdx = 0; TrisToSplitIdx < ArrayCount(TrisToSplit); TrisToSplitIdx++)
                    {
                        int TriToSplitIdx = TrisToSplit[TrisToSplitIdx];
                        tri2 TriToSplit = TriangulationTris[TriToSplitIdx];

                        /* First, remove it from the triangulation. */
                        Assert(NumTriangulationTris > 0);
                        TriangulationTris[TriToSplitIdx] = TriangulationTris[--NumTriangulationTris];
                        /* Did we just swap the next tri? */
                        if (TrisToSplitIdx == 0 && TrisToSplit[1] == NumTriangulationTris)
                        {
                            /* Fix it. */
                            TrisToSplit[1] = TriToSplitIdx;
                        }

                        /* Get sub tris. */
                        tri2 SubTris[3];
                        Split(TriToSplit, Point, SubTris);

                        /* Now remove the sub tri the point is collinear with. */
                        bool FoundCollinear = false;
                        for (int SubTriIdx = 0; SubTriIdx < ArrayCount(SubTris); SubTriIdx++)
                        {
                            tri2 SubTri = SubTris[SubTriIdx];
                            if (CheckContains(SubTri, CommonEdge))
                            {
                                FoundCollinear = true;
                                Assert(CheckCollinear(SubTri));

                                /* Remove it. */
                                SubTris[SubTriIdx] = SubTris[ArrayCount(SubTris) - 1];

                                /* There should only be one. */
                                break;
                            }
                        }
                        Assert(FoundCollinear);

                        /* Now add the sub triangles. */
                        for (int SubTriIdx = 0; SubTriIdx < ArrayCount(SubTris) - 1; SubTriIdx++)
                        {
                            Assert(NumTriangulationTris <= MaxNumTriangulationTris);
                            TriangulationTris[NumTriangulationTris++] = SubTris[SubTriIdx];
                        }
                    }
                }
                else
                {
                    /* Remove element. */
                    TriangulationTris[TriangulationTriIdx] = TriangulationTris[--NumTriangulationTris];

                    /* Split triangles. */
                    tri2 SubTris[3];
                    Split(TriContainingPoint, Point, SubTris);

                    /* Add sub tris. */
                    for (int SubTriIdx = 0; SubTriIdx < ArrayCount(SubTris); SubTriIdx++)
                    {
                        Assert(NumTriangulationTris <= MaxNumTriangulationTris);
                        TriangulationTris[NumTriangulationTris++] = SubTris[SubTriIdx];
                    }
                }

                /* Found triangle point is in. Break. */
                break;
            }
        }

        /* Point should be in at least one triangle. */
        Assert(NumTriangulationTris != DebugPreNumTris);
    }

    *NumTriangulationTris_InOut = NumTriangulationTris;
}

void DoDelaunay(tri2 *Tris, int NumTris)
{
    /* This algorithm is extremely simple. We continuously pass over all the triangles, fixing them as necessary, until we're
       able to pass over all of them without having to fix any. */

    /* Keep passing over the triangles until they all satisfy delaunay. */
    while (1)
    {
        bool WeDelaunayYet = true;
        for (int TriIdx = 0; TriIdx < NumTris; TriIdx++)
        {
            tri2 Tri = Tris[TriIdx];
            for (int NeighborTriIdx = TriIdx + 1; NeighborTriIdx < NumTris; NeighborTriIdx++)
            {
                tri2 NeighborTri = Tris[NeighborTriIdx];
                line2 CommonEdge;
                if (CheckContainsCommonEdge(Tri, NeighborTri, &CommonEdge))
                {

                    /* Get the new common edge now. */
                    v2 TriNonCommonEdgePoint = GetVertexExcluding(Tri, CommonEdge);
                    v2 NeighborTriNonCommonEdgePoint = GetVertexExcluding(NeighborTri, CommonEdge);
                    line2 NewCommonEdge = LINE2(TriNonCommonEdgePoint, NeighborTriNonCommonEdgePoint);

                    /* We need to calculate the sum of the angles opposite the common edge. */
                    float SumOfOppositeEdgeAngles;
                    {
                        line2 TriNonCommonEdges[2];
                        line2 NeighborTriNonCommonEdges[2];
                        GetEdgesExcluding(Tri, CommonEdge, TriNonCommonEdges);
                        GetEdgesExcluding(NeighborTri, CommonEdge, NeighborTriNonCommonEdges);

                        /* Ensure correct order. This means the non common edge should be first. Necessary for direction. */
                        EnsureEndPointIsFirst(&TriNonCommonEdges[0], TriNonCommonEdgePoint);
                        EnsureEndPointIsFirst(&TriNonCommonEdges[1], TriNonCommonEdgePoint);
                        EnsureEndPointIsFirst(&NeighborTriNonCommonEdges[0], NeighborTriNonCommonEdgePoint);
                        EnsureEndPointIsFirst(&NeighborTriNonCommonEdges[1], NeighborTriNonCommonEdgePoint);

                        v2 TriNonCommonEdgeDir0 = GetDirection(TriNonCommonEdges[0]);
                        v2 TriNonCommonEdgeDir1 = GetDirection(TriNonCommonEdges[1]);
                        v2 NeighborTriNonCommonEdgeDir0 = GetDirection(NeighborTriNonCommonEdges[0]);
                        v2 NeighborTriNonCommonEdgeDir1 = GetDirection(NeighborTriNonCommonEdges[1]);

                        float Alpha = GetAngleBetween(TriNonCommonEdgeDir0, TriNonCommonEdgeDir1);
                        float Beta = GetAngleBetween(NeighborTriNonCommonEdgeDir0, NeighborTriNonCommonEdgeDir1);
                        SumOfOppositeEdgeAngles = Alpha + Beta;
                    }

                    /* Does it not satisfy delaunay? */
                    if (SumOfOppositeEdgeAngles > M_PI)
                    {
                        /* We need to flip so they satisfy delaunay. */

                        /* Replace the old triangles. */
                        tri2 TriReplacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P1);
                        tri2 NeightborTriReplacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P2);
                        Tris[TriIdx] = TriReplacement;
                        Tris[NeighborTriIdx] = NeightborTriReplacement;

                        /* This is where things get simple. Instead of doing anything clever, we just pass over all the triangles
                           again. */
                        WeDelaunayYet = false;
                        goto WE_DELAUNAY_CHECK;
                    }
                }
            }
        }
WE_DELAUNAY_CHECK:
        if (WeDelaunayYet)
        {
            break;
        }
    }
}

struct triangulation_result
{
    tri2 *TriangulationTris;
    int NumTriangulationTris;
};
triangulation_result CreateTriangulation(v2 *Points, int NumPoints,
                                         line2 *ConstraintEdges, int NumConstraintEdges)
{
    Assert(NumPoints >= 3);

    /* Create the convex hull. */
    v2 ConvexHullPoints[128];
    int NumConvexHullPoints;
    /* Note that this function modifies the points array to put all the convex hulls in the front. */
    CreateConvexHull(Points, NumPoints, ConvexHullPoints, ArrayCount(ConvexHullPoints), &NumConvexHullPoints);

    /* TODO(@memory): Figure out exactly... not sure how. */
    int MaxNumTriangulationTris = 10*NumPoints;
    tri2 *TriangulationTris = (tri2 *)MemAlloc(sizeof(tri2)*MaxNumTriangulationTris);
    int NumTriangulationTris = 0;

    /* Triangulate the convex hull. */
    {
        tri2 SuperTriangle = CreateSuperTriangle(ConvexHullPoints, NumConvexHullPoints);
        TriangulationTris[NumTriangulationTris++] = SuperTriangle;

        /* Create triangulation of convex hull. This triangulation method special because it ensures that the polygon is
           closed off from the super triangle vertices, but the outline is not the convex hull. See pic8. I refer to this
           method as "almost-convex". */
        for (int PointIdx = 0; PointIdx < NumConvexHullPoints; PointIdx++)
        {
            v2 Point = Points[PointIdx];
            int DebugPreNumTris = NumTriangulationTris;
            for (int TriangulationTriIdx = 0; TriangulationTriIdx < NumTriangulationTris; TriangulationTriIdx++)
            {
                tri2 TriContainingPoint = TriangulationTris[TriangulationTriIdx];
                bool IsPointOnEdge;
                if (CheckInside(Point, TriContainingPoint, &IsPointOnEdge))
                {
                    if (IsPointOnEdge)
                    {
                        /* Instead of splitting the triangle into three, we should only split it into two, otherwise we get collinear
                           triangle nightmares. */

                           /* Find other triangle. */
                        int OtherTriIdx = -1;
                        for (int OtherTriTriangulationIdx = TriangulationTriIdx + 1; OtherTriTriangulationIdx < NumTriangulationTris; OtherTriTriangulationIdx++)
                        {
                            tri2 OnEdgeTri = TriangulationTris[OtherTriTriangulationIdx];
                            if (CheckInside(Point, OnEdgeTri, &IsPointOnEdge))
                            {
                                if (IsPointOnEdge)
                                {
                                    /* Found it! */
                                    OtherTriIdx = OtherTriTriangulationIdx;
                                    break;
                                }
                            }
                        }
                        /* There should be another triangle. A point should never be on the outline of the polygon since we're
                           either triangulating the convex hull - in which case, there can't be a point on the super triangle -
                           or we're triangulating the the rest of the points inside the convex hull - there should be no points
                           on the convex hull, because they should've been part of the convex hull points! */
                        Assert(OtherTriIdx != -1);

                        tri2 OtherTri = TriangulationTris[OtherTriIdx];

                        /* Now find their common edge. */
                        line2 CommonEdge;
                        bool HasCommonEdge = CheckContainsCommonEdge(TriContainingPoint, OtherTri, &CommonEdge);
                        Assert(HasCommonEdge);

                        /* Now split them. */
                        int TrisToSplit[] ={ TriangulationTriIdx, OtherTriIdx };
                        for (int TrisToSplitIdx = 0; TrisToSplitIdx < ArrayCount(TrisToSplit); TrisToSplitIdx++)
                        {
                            int TriToSplitIdx = TrisToSplit[TrisToSplitIdx];
                            tri2 TriToSplit = TriangulationTris[TriToSplitIdx];

                            /* First, remove it from the triangulation. */
                            Assert(NumTriangulationTris > 0);
                            TriangulationTris[TriToSplitIdx] = TriangulationTris[--NumTriangulationTris];
                            /* Did we just swap the next tri? */
                            if (TrisToSplitIdx == 0 && TrisToSplit[1] == NumTriangulationTris)
                            {
                                /* Fix it. */
                                TrisToSplit[1] = TriToSplitIdx;
                            }

                            /* Get sub tris. */
                            tri2 SubTris[3];
                            Split(TriToSplit, Point, SubTris);

                            /* Now remove the sub tri the point is collinear with. */
                            bool FoundCollinear = false;
                            for (int SubTriIdx = 0; SubTriIdx < ArrayCount(SubTris); SubTriIdx++)
                            {
                                tri2 SubTri = SubTris[SubTriIdx];
                                if (CheckContains(SubTri, CommonEdge))
                                {
                                    FoundCollinear = true;
                                    Assert(CheckCollinear(SubTri));

                                    /* Remove it. */
                                    SubTris[SubTriIdx] = SubTris[ArrayCount(SubTris) - 1];

                                    /* There should only be one. */
                                    break;
                                }
                            }
                            Assert(FoundCollinear);

                            /* Now add the sub triangles. */
                            for (int SubTriIdx = 0; SubTriIdx < ArrayCount(SubTris) - 1; SubTriIdx++)
                            {
                                Assert(NumTriangulationTris <= MaxNumTriangulationTris);
                                TriangulationTris[NumTriangulationTris++] = SubTris[SubTriIdx];
                            }
                        }
                    }
                    else
                    {
                        /* Remove element. */
                        TriangulationTris[TriangulationTriIdx] = TriangulationTris[--NumTriangulationTris];

                        /* Split triangles. */
                        tri2 SubTris[3];
                        Split(TriContainingPoint, Point, SubTris);

                        /* Add sub tris. */
                        for (int SubTriIdx = 0; SubTriIdx < ArrayCount(SubTris); SubTriIdx++)
                        {
                            /* Check to see if we have a non-almost-convex (NAC) case. */

                            /* If we share a super triangle vert, we have a potential NAC case. */
                            tri2 SubTri = SubTris[SubTriIdx];
                            v2 SuperTriCommonVerts[3];
                            int NumCommonVertices = GetCommonVertices(SubTri, SuperTriangle, SuperTriCommonVerts);
                            if (NumCommonVertices == 1)
                            {
                                line2 CommonEdge = GetEdgeExcluding(SubTri, Point);
                                v2 SuperTriCommonVert = SuperTriCommonVerts[0];
                                v2 VertexNotInSuperTri = (CommonEdge.P1 == SuperTriCommonVert) ? CommonEdge.P2 : CommonEdge.P1;

                                /* Find the other triangle that shares this edge. */
                                int NeighborTriIdx;
                                for (NeighborTriIdx = 0; NeighborTriIdx < NumTriangulationTris; NeighborTriIdx++)
                                {
                                    tri2 Tri = TriangulationTris[NeighborTriIdx];
                                    if (CheckContains(Tri, CommonEdge))
                                    {
                                        /* We found it! */
                                        break;
                                    }
                                }
                                /* We should've found one. Remember, we only share one vertex with the super triangle. */
                                Assert(NeighborTriIdx != NumTriangulationTris);
                                tri2 NeighborTri = TriangulationTris[NeighborTriIdx];

                                /* @pull: this angle code is duplicated from way below. */

                                /* Check to see if we have a NAC case. */
                                tri2 Tris[] = { SubTri, NeighborTri };
                                float TotalAngle = 0.0f;
                                for (int I = 0; I < ArrayCount(Tris); I++)
                                {
                                    /* Get the angle between the common edge and the edge that does not contain the
                                       super triangle vertex. */

                                    line2 NoSuperTriVertEdge = GetEdgeExcluding(Tris[I], SuperTriCommonVert);

                                    /* Make sure the shared vertex between the edges is the first for both. */
                                    EnsureEndPointIsFirst(&NoSuperTriVertEdge, VertexNotInSuperTri);
                                    EnsureEndPointIsFirst(&CommonEdge, VertexNotInSuperTri);

                                    v2 NewTriDir = GetDirection(NoSuperTriVertEdge);
                                    v2 CommonEdgeDir = GetDirection(CommonEdge);

                                    float Angle = GetAngleBetween(NewTriDir, CommonEdgeDir);
                                    TotalAngle += Angle;
                                }

                                /* Can we flip this edge and still have a triangle? */
                                if (TotalAngle < M_PI &&
                                    /* It's possible that due to floating point error, the angle is actually PI. It's
                                       pointless to flip if it is PI, and it also introduces collinear triangles, so
                                       don't do it. */
                                    !IsWithinTolerance(TotalAngle, M_PI))
                                {
                                    /* We have a NAC! */

                                    /* Get the non-common edge verts. */
                                    v2 Tri0NonCommonEdgeVert = GetVertexExcluding(SubTri, CommonEdge);
                                    v2 Tri1NonCommonEdgeVert = GetVertexExcluding(NeighborTri, CommonEdge);

                                    /* Make the new common edge */
                                    line2 NewCommonEdge = LINE2(Tri0NonCommonEdgeVert, Tri1NonCommonEdgeVert);

                                    /* Now check if this new common edge contains any super tri verts. If it does, we have a NAC. */
                                    Assert(!CheckContains(SuperTriangle, NewCommonEdge));

                                    /* Before we add the new tris, we need to make them. */
                                    tri2 Tri0Replacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P1);
                                    tri2 Tri1Replacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P2);

                                    /* First, we need to remove the neighbor tri, since it will be replaced. */
                                    Assert(NumTriangulationTris > 0);
                                    TriangulationTris[NeighborTriIdx] = TriangulationTris[--NumTriangulationTris];

                                    /* Now add them. */
                                    Assert(NumTriangulationTris <= MaxNumTriangulationTris);
                                    TriangulationTris[NumTriangulationTris++] = Tri0Replacement;
                                    Assert(NumTriangulationTris <= MaxNumTriangulationTris);
                                    TriangulationTris[NumTriangulationTris++] = Tri1Replacement;
                                }
                                else
                                {
                                    /* We don't have a NAC case. Just add the triangle. */
                                    Assert(NumTriangulationTris <= MaxNumTriangulationTris);
                                    TriangulationTris[NumTriangulationTris++] = SubTris[SubTriIdx];
                                }
                            }
                            else
                            {
                                /* We don't have a NAC case. Just add the triangle. */
                                Assert(NumTriangulationTris <= MaxNumTriangulationTris);
                                TriangulationTris[NumTriangulationTris++] = SubTris[SubTriIdx];
                                Assert(!CheckForDuplicates(TriangulationTris, NumTriangulationTris));
                            }
                        }
                    }

                    /* Found triangle point is in. Break. */
                    break;
                }
            }

            /* Point should be in at least one triangle. */
            Assert(NumTriangulationTris != DebugPreNumTris);
        }
        /* At this point, we have an almost-convex triangulation of the convex hull. */

        /* Time to go from almost-convex to fully convex. We do this by flipping super tri tris until we've got a convex hull. */
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

            /* We need to compare the last triangle of each vertex to the first for the next so that we have continuity.
               Refer to pic4. */
            int FirstTriangles[ArrayCount(SuperTriangle.Verts)];
            int LastTriangles[ArrayCount(SuperTriangle.Verts)];

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
                        if (CheckContains(TriangulationTri, SuperTriVert))
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
                            if (CheckContains(Edges[EdgeIdx], SuperTriVert))
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
                        float TmpAngle = SuperTriVertTrisAngles[NumSort - 1];
                        SuperTriVertTrisAngles[NumSort - 1] = SuperTriVertTrisAngles[MaxIdx];
                        SuperTriVertTrisAngles[MaxIdx] = TmpAngle;

                        /* Swap max and end super triangle vertex tris. */
                        int TmpIndex = SuperTriVertTris[NumSort - 1];
                        SuperTriVertTris[NumSort - 1] = SuperTriVertTris[MaxIdx];
                        SuperTriVertTris[MaxIdx] = TmpIndex;
                    }
                } /* The triangles for this super tri vert are sorted on their angle. */

                /* Save the first and last triangles. Note that these indices should not change because we assume these first
                   and last triangles can't be changed, and we also don't add or remove triangles here (just replace them).
                   If there are two or less triangles, then none of the triangles are candidates. */
                if (NumSuperTriVertTris > 2)
                {
                    FirstTriangles[SuperTriVertsIdx] = SuperTriVertTris[1];
                    LastTriangles[SuperTriVertsIdx] = SuperTriVertTris[NumSuperTriVertTris - 2];
                }
                else
                {
                    FirstTriangles[SuperTriVertsIdx] = -1;
                    LastTriangles[SuperTriVertsIdx] = -1;
                }

                /* Go through each super vert tri. */
                for (int SuperTriVertTrisIdx = 1; SuperTriVertTrisIdx < NumSuperTriVertTris - 2; /* Skip first and last. We know it's not possible. */
                     SuperTriVertTrisIdx++)
                {
                    int Tri0Idx = SuperTriVertTris[SuperTriVertTrisIdx];
                    int Tri1Idx = SuperTriVertTris[SuperTriVertTrisIdx + 1];
                    tri2 Tri0 = TriangulationTris[Tri0Idx];
                    tri2 Tri1 = TriangulationTris[Tri1Idx];

                    line2 CommonEdge;
                    bool HasCommonEdge = CheckContainsCommonEdge(Tri0, Tri1, &CommonEdge);
                    Assert(HasCommonEdge);
                    v2 Tri0NonCommonEdgeVert = GetVertexExcluding(Tri0, CommonEdge);
                    v2 Tri1NonCommonEdgeVert = GetVertexExcluding(Tri1, CommonEdge);
                    line2 NewCommonEdge = LINE2(Tri0NonCommonEdgeVert, Tri1NonCommonEdgeVert);

                    /* We only care if the new common edge contains no super triangle vertices; otherwise,
                       we'd flip into the same situation (one vertex not in super triangle, and one in). */
                    if (!CheckContainsAnySameVertices(SuperTriangle, NewCommonEdge))
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
                            if (TotalAngle < M_PI &&
                                /* It's possible that due to floating point error, the angle is actually PI. It's
                                   pointless to flip if it is PI, and it also introduces collinear triangles, so
                                   don't do it. Refer to pic6 for an example. */
                                !IsWithinTolerance(TotalAngle, M_PI))
                            {
                                /* DJ, flip that shit! */

                                /* Replace the old triangles with new ones. Note that it's very important that the next triangle
                                   (Tri1) is replaced with the triangle that has the super triangle. Play pic7 out and you'll
                                   know why. */
                                tri2 Tri0Replacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P1);
                                tri2 Tri1Replacement = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P2);

                                /* Neither of these triangles should be collinear. */
                                Assert(!CheckCollinear(Tri0Replacement));
                                Assert(!CheckCollinear(Tri1Replacement));

                                /* Remove the triangle. */
                                TriangulationTris[Tri0Idx] = Tri0Replacement;
                                TriangulationTris[Tri1Idx] = Tri1Replacement;

                                /* This is for keeping track of the first and last triangles. */
                                if (SuperTriVertTrisIdx == 1 || SuperTriVertTrisIdx == NumSuperTriVertTris - 3)
                                {
                                    /* We want the super triangle triangle. That will replace the first. */
                                    int SuperTriTriReplacement;
                                    if (CheckContains(Tri0Replacement, SuperTriVert))
                                    {
                                        SuperTriTriReplacement = Tri0Idx;
                                    }
                                    else
                                    {
                                        SuperTriTriReplacement = Tri1Idx;
                                    }

                                    /* Now replace the first or last triangle. */
                                    if (SuperTriVertTrisIdx == 1)
                                    {
                                        FirstTriangles[SuperTriVertsIdx] = SuperTriTriReplacement;
                                    }
                                    else
                                    {
                                        LastTriangles[SuperTriVertsIdx] = SuperTriTriReplacement;
                                    }
                                }
                                Assert(!CheckForDuplicates(TriangulationTris, NumTriangulationTris));
                            }
                        }
                    }
                }
            }

            /* We still have to take into account the discontinuity. At this point, we have the first and last triangles,
               so now is the time to take care of it. */
            for (int SuperTriVertsIdx = 0; SuperTriVertsIdx < ArrayCount(SuperTriangle.Verts); SuperTriVertsIdx++)
            {
                v2 SuperTriVert = SuperTriangle.Verts[SuperTriVertsIdx];
                int NextIdx = (SuperTriVertsIdx + 1) % ArrayCount(SuperTriangle.Verts);
                v2 NextSuperTriVert = SuperTriangle.Verts[NextIdx];

                /* Determine if we're in the condition in pic4. Note that some of these checks may be unnecessary/redundant. 
                   I'm just going off obvious things I see. */
                int FirstTriIdxOfCur = FirstTriangles[SuperTriVertsIdx];
                int LastTriIdxOfNext = LastTriangles[NextIdx];
                if (FirstTriIdxOfCur != -1 && LastTriIdxOfNext != -1)
                {
                    tri2 FirstTriOfCur = TriangulationTris[FirstTriIdxOfCur];
                    tri2 LastTriOfNext = TriangulationTris[LastTriIdxOfNext];

                    /* Neither of the triangles should share a common edge. */
                    bool ShareCommonEdge = CheckContainsCommonEdge(FirstTriOfCur, LastTriOfNext);
                    if (!ShareCommonEdge)
                    {
                        v2 CommonVertices[3];
                        int NumCommonVertices = GetCommonVertices(FirstTriOfCur, LastTriOfNext, CommonVertices);
                        if (NumCommonVertices == 1)
                        {
                            v2 CommonVertex = CommonVertices[0];

                            /* One of the requirements is that angle between the edges opposite the common edge are less than 180 degrees. 
                               The 'discontinuous' common edge is the edge for each triangle that will be merged together if necessary. */
                            line2 FirstTriDiscontinuousCommonEdge = LINE2(CommonVertex, SuperTriVert);
                            line2 LastTriDiscontinuousCommonEdge = LINE2(CommonVertex, NextSuperTriVert);
                            v2 FirstTriNonDiscontinuousCommonEdgeVert = GetVertexExcluding(FirstTriOfCur, FirstTriDiscontinuousCommonEdge);
                            v2 LastTriNonDiscontinuousCommonEdgeVert = GetVertexExcluding(LastTriOfNext, LastTriDiscontinuousCommonEdge);
                            line2 FirstTriNonDiscontinuousCommonEdge = LINE2(CommonVertex, FirstTriNonDiscontinuousCommonEdgeVert);
                            line2 LastTriNonDiscontinuousCommonEdge = LINE2(CommonVertex, LastTriNonDiscontinuousCommonEdgeVert);
                            float AngleBetweenNonDiscCommonEdges = GetAngleBetween(FirstTriNonDiscontinuousCommonEdge, LastTriNonDiscontinuousCommonEdge, AngleDirection_CCW);
                            /* An angle of 0.0f means the directions of the edges are the same. This is possible, the angle is actually 360 degrees, not 0. */
                            if (AngleBetweenNonDiscCommonEdges == 0.0f)
                            {
                                AngleBetweenNonDiscCommonEdges = M_2PI;
                            }
                            if (AngleBetweenNonDiscCommonEdges < M_PI)
                            {
                                /* Merge the three triangles into two triangles. */
                                line2 CommonEdge;
                                line2 NewCommonEdge;
                                v2 SuperTriEdgeMid;
                                {
                                    /* Get the new super triangle vertex. */
                                    line2 SuperTriEdge = LINE2(SuperTriangle.Verts[SuperTriVertsIdx], SuperTriangle.Verts[NextIdx]);
                                    SuperTriEdgeMid = GetCenter(SuperTriEdge);

                                    /* Replace the last triangle's super tri vertex with the new one. */
                                    int LastTriIdxOfSuperTriVert = GetIndexOf(FirstTriOfCur, SuperTriVert);
                                    FirstTriOfCur.Verts[LastTriIdxOfSuperTriVert] = SuperTriEdgeMid;

                                    /* Replace the first triangle's super tri vertex with the new one. */
                                    int FirstTriIdxOfSuperTriVert = GetIndexOf(LastTriOfNext, NextSuperTriVert);
                                    LastTriOfNext.Verts[FirstTriIdxOfSuperTriVert] = SuperTriEdgeMid;

                                    /* Get common and new edges. */
                                    CommonEdge = LINE2(CommonVertex, SuperTriEdgeMid);
                                    NewCommonEdge.P1 = GetVertexExcluding(FirstTriOfCur, CommonEdge);
                                    NewCommonEdge.P2 = GetVertexExcluding(LastTriOfNext, CommonEdge);
                                }

                                /* Do the actual check and possible flip. Note that this is copied and pasted from above. */
                                {
                                    /* Ensure the variable names are similar to the above. */
                                    tri2 Tri0 = FirstTriOfCur;
                                    tri2 Tri1 = LastTriOfNext;
                                    v2 VertexInSuperTri = SuperTriEdgeMid;
                                    v2 VertexNotInSuperTri = CommonVertex;

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

                                        /* Remove the original first and last triangles. Note that there's no need to remove the third
                                           super triangle triangle since it will be removed later. Also note that right after we do this,
                                           we break out and move to the next first/last pair. */
                                        Assert(NumTriangulationTris > 0);
                                        TriangulationTris[FirstTriIdxOfCur] = TriangulationTris[--NumTriangulationTris];
                                        Assert(NumTriangulationTris > 0);
                                        TriangulationTris[LastTriIdxOfNext] = TriangulationTris[--NumTriangulationTris];

                                        /* Now add the triangle that will merge the first and last triangles. */
                                        tri2 NewTri = TRI2(NewCommonEdge.P1, NewCommonEdge.P2, CommonEdge.P1);
                                        Assert(!CheckCollinear(NewTri));
                                        TriangulationTris[NumTriangulationTris++] = NewTri;
                                        Assert(!CheckForDuplicates(TriangulationTris, NumTriangulationTris));

                                        /* We also deviate by assuming this can only happen at most once. I have no proof of this though. We'll farm it. */
                                        goto DISCONTINUITYFORLOOPEND;
                                    }
                                }
                            }
                        }
                    }
                }
            }
DISCONTINUITYFORLOOPEND: int DiscontinuityForLoopEnd = 0;
        }
        /* At this point, we have a proper triangulation of the convex hull, but the super triangle is still in there. */

        /* Now remove the super triangle triangles. */
        for (int TriIdx = 0; TriIdx < NumTriangulationTris;)
        {
            tri2 TriangulationTri = TriangulationTris[TriIdx];
            if (CheckContainsAnySameVertices(TriangulationTri, SuperTriangle))
            {
                /* Remove that bitch nigga. */
                Assert(NumTriangulationTris > 0);
                TriangulationTris[TriIdx] = TriangulationTris[--NumTriangulationTris];
                Assert(!CheckForDuplicates(TriangulationTris, NumTriangulationTris));
            }
            else
            {
                TriIdx++;
            }
        }
    }
    /* At this point, we have a proper triangulation of the convex hull. */

    /* Triangulate rest of points. We don't need a super triangle, since we have the convex hull triangulation. This will
       give us a triangulation of all the points. */
    CreateTriangulationFrom(TriangulationTris, MaxNumTriangulationTris, &NumTriangulationTris, Points + NumConvexHullPoints, NumPoints - NumConvexHullPoints);

    /* Before we jump into the constraint part, we need to ensure the triangulation is delaunay. */
    DoDelaunay(TriangulationTris, NumTriangulationTris);

#if 1
    /* Add constraint edges. */
    for (int ConstraintEdgesIdx = 0; ConstraintEdgesIdx < NumConstraintEdges; ConstraintEdgesIdx++)
    {
        line2 ConstraintEdge = ConstraintEdges[ConstraintEdgesIdx];

        /* Check if this edge is already an edge. */
        bool AlreadyAnEdge = false;
        for (int TriIdx = 0; TriIdx < NumTriangulationTris; TriIdx++)
        {
            if (CheckContains(TriangulationTris[TriIdx], ConstraintEdge))
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
                    if (CheckIntersects(TriEdge, ConstraintEdge))
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

                        /* It's possible for this to be duplicate. This happens when multiple edges make up this constraint edge (pic3). 
                           Instead of being clever (like merging collinear edges, which I'm not sure is possible in all cases), 
                           we just don't add the duplicate and re-triangulate. */
                        /* TODO: "multiple edges make up this constraint edge" - shouldn't the constraint edge be decomposed into the edges that make it up???
                           AFAIK, the current solution still works, but still... */

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

        /* We're going to add triangles, but we don't want to compare the ones we've added.
           Q: Is this necessary? Why can't we compare them to the ones we've added? */
        int OriginalNumTriangulationTris = NumTriangulationTris;

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
            Assert(ConstraintPolygonTriangulationResult.NumTriangulationTris > 0);

            /* We don't want to introduce any more collinear triangles. I think the triangulation should still be the same 
               with or without the collinear triangles. I just don't want them. */
            for (int NewTriIdx = 0; NewTriIdx < ConstraintPolygonTriangulationResult.NumTriangulationTris; NewTriIdx++)
            {
                tri2 NewTri = ConstraintPolygonTriangulationResult.TriangulationTris[NewTriIdx];
                if (CheckCollinear(NewTri))
                {
                    /* Remove. */
                    Assert(ConstraintPolygonTriangulationResult.NumTriangulationTris > 0);
                    ConstraintPolygonTriangulationResult.TriangulationTris[NewTriIdx] = 
                        ConstraintPolygonTriangulationResult.TriangulationTris[--ConstraintPolygonTriangulationResult.NumTriangulationTris];
                }
            }

            /* Now add them! */
            for (int ConstraintPolygonTriangulationTriIdx = 0; ConstraintPolygonTriangulationTriIdx < ConstraintPolygonTriangulationResult.NumTriangulationTris; ConstraintPolygonTriangulationTriIdx++)
            {
                Assert(NumTriangulationTris < MaxNumTriangulationTris);
                tri2 NewTri = ConstraintPolygonTriangulationResult.TriangulationTris[ConstraintPolygonTriangulationTriIdx];

                bool DoAddTriangle = true;
                {
                    /* We need to make sure it doesn't intersect any existing triangles. */

                    line2 NewTriEdges[3];
                    GetEdges(NewTri, NewTriEdges);

                    for (int NewTriEdgeIdx = 0; NewTriEdgeIdx < ArrayCount(NewTriEdges); NewTriEdgeIdx++)
                    {
                        line2 NewTriEdge = NewTriEdges[NewTriEdgeIdx];
                        for (int TriangulationTriIdx = 0; TriangulationTriIdx < OriginalNumTriangulationTris; TriangulationTriIdx++)
                        {
                            tri2 TriangulationTri = TriangulationTris[TriangulationTriIdx];
                            line2 TriangulationTriEdges[3];
                            GetEdges(TriangulationTri, TriangulationTriEdges);
                            for (int TriangulationTriEdgeIdx = 0; TriangulationTriEdgeIdx < ArrayCount(TriangulationTriEdges); TriangulationTriEdgeIdx++)
                            {
                                line2 TriangulationTriEdge = TriangulationTriEdges[TriangulationTriEdgeIdx];
                                bool Overlaps;

                                /* Are they the same? */
                                if (NewTriEdge == TriangulationTriEdge)
                                {
                                    /* We handle this case in the next for-loop. */
                                }
                                /* Do they intersect? */
                                else if (CheckIntersects(NewTriEdge, TriangulationTriEdge, &Overlaps))
                                {
                                    /* It's possible that the triangle this edge belongs to is completely inside the constraint
                                       polygon half (pic1). If that's the case, then just remove that triangle - it's been
                                       triangulated by this constraint polygon half. */
                                    bool IsTriangleCompletelyInPolygonHalf = true;
                                    {
                                        /* If all vertices of the triangle are inside the constraint polygon, then it's completely
                                           inside the constraint polygon. */
                                        for (int TriangulationTriVertIdx = 0; TriangulationTriVertIdx < ArrayCount(TriangulationTri.Verts); TriangulationTriVertIdx++)
                                        {
                                            v2 TriangulationTriVert = TriangulationTri.Verts[TriangulationTriVertIdx];
                                            if (!CheckArrayContains(ConstraintPolygonHalves[ConstraintPolygonHalfIdx], NumConstraintPolygonVertices[ConstraintPolygonHalfIdx], TriangulationTriVert))
                                            {
                                                IsTriangleCompletelyInPolygonHalf = false;
                                                break;
                                            }
                                        }
                                    }
                                    if (IsTriangleCompletelyInPolygonHalf)
                                    {
                                        /* Remove the triangle from the original set of triangles. */
                                        Assert(OriginalNumTriangulationTris > 0);
                                        TriangulationTris[TriangulationTriIdx] = TriangulationTris[--OriginalNumTriangulationTris];

                                        /* The last original triangle we swapped in to remove the triangle is duplicated
                                           in the overall set of triangles. Get rid of it by replacing it with the last triangle 
                                           in the overall set. */
                                        Assert(NumTriangulationTris > 0);
                                        TriangulationTris[OriginalNumTriangulationTris] = TriangulationTris[--NumTriangulationTris];

                                        /* Make sure triangulation for-loop doesn't advance to next index. */
                                        TriangulationTriIdx--;

                                        Assert(!CheckForDuplicates(TriangulationTris, NumTriangulationTris));

                                        /* Go to next triangle. */
                                        goto TriangulationTriForLoopEnd;
                                    }
                                    else
                                    {
                                        /* It's possible that the edge we intersected belongs to a collinear triangle. It's
                                           also possible that the edge we're adding is itself a collinear triangle. Collinear 
                                           triangles are valid, however, overlapping certain edges shouldn't be counted as
                                           intersections. */
                                        if (Overlaps)
                                        {
                                            /* Ensure one of the triangles are collinear. */
                                            Assert(CheckCollinear(TriangulationTri) || CheckCollinear(NewTri));

                                            /* This isn't a real intersection, so do nothing. */
                                        }
                                        else
                                        {
                                            /* Then we don't want to add it. */
                                            DoAddTriangle = false;
                                            goto AddTriangleEnd;
                                        }
                                    }
                                }
                            }
TriangulationTriForLoopEnd: int TRIANGULATIONTRIFORLOOPEND = 0;
                        }
                    }

                    /* Because of pic2, it's possible that a new triangle shares all the same edges of existing triangles,
                       and doesn't intersect any of the triangulation triangles. In this case, we don't want to add it. We
                       need to keep track with this variable. Note that we must do this after doing all the intersection stuff... TODO: explain this!!!!!!! */
                    int NumEdgesAlreadyExisting = 0;
                    int DebugEdgesAlreadyExisting[3] = {}; /* This keeps track of how many times each edge is in the triangulation. */
                    for (int NewTriEdgeIdx = 0; NewTriEdgeIdx < ArrayCount(NewTriEdges); NewTriEdgeIdx++)
                    {
                        line2 NewTriEdge = NewTriEdges[NewTriEdgeIdx];
                        for (int TriangulationTriIdx = 0; TriangulationTriIdx < OriginalNumTriangulationTris; TriangulationTriIdx++)
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
                                    NumEdgesAlreadyExisting++;
                                    DebugEdgesAlreadyExisting[NewTriEdgeIdx]++;

                                    /* The maximum valid amount is 2 (there must be at least one edge inside the constraint polygon). */
                                    if (NumEdgesAlreadyExisting > 2)
                                    {
                                        DoAddTriangle = false;
                                        goto AddTriangleEnd;
                                    }
                                }
                            }
                        }
                    }
                }
AddTriangleEnd:
                if (DoAddTriangle)
                {
                    Assert(NumTriangulationTris < MaxNumTriangulationTris);
                    TriangulationTris[NumTriangulationTris++] = NewTri;
                    Assert(!CheckForDuplicates(TriangulationTris, NumTriangulationTris));
                }
            }

            /* Now free them! */
            MemFree(ConstraintPolygonTriangulationResult.TriangulationTris);
        }

#if 0 /* Useful code for debugging. */
#if 0
{
glClear(GL_COLOR_BUFFER_BIT);
for (int I = 0; I < NumTriangulationTris; I++)
{
    AddTriangle(&GlobalShapeRenderer, TriangulationTris[I], GLUTILS_WHITE, true);
}
for (int I = 0; I < ConstraintEdgesIdx; I++)
{
    AddLine(&GlobalShapeRenderer, ConstraintEdges[I], 1.0f, GLUTILS_ORANGE);
}
AddLine(&GlobalShapeRenderer, ConstraintEdges[ConstraintEdgesIdx], 1.0f, GLUTILS_GREEN);
for (int I = 0; I < NumPoints; I++)
{
    AddPoint(&GlobalShapeRenderer, Points[I], GLUTILS_YELLOW);
}
for (int I = 0; I < NumConstraintPolygonVertices[ConstraintPolygonHalfIdx]; I++)
{
    AddPoint(&GlobalShapeRenderer, ConstraintPolygonHalves[ConstraintPolygonHalfIdx][I], GLUTILS_RED);
}
#if 1
for (int I = 0; I < ConstraintPolygonTriangulationResult.NumTriangulationTris; I++)
{
    AddTriangle(&GlobalShapeRenderer, ConstraintPolygonTriangulationResult.TriangulationTris[I], GLUTILS_ORANGE, true);
}
#endif
Render(&GlobalShapeRenderer);
DebugPlatformSwapBuffers();
int debug = 0;
}
#endif
#if 0
{
glClear(GL_COLOR_BUFFER_BIT);
for (int I = 0; I < NumTriangulationTris; I++)
{
    AddTriangle(&GlobalShapeRenderer, TriangulationTris[I], GLUTILS_WHITE, true);
}
for (int I = 0; I < NumPoints; I++)
{
    AddPoint(&GlobalShapeRenderer, Points[I], GLUTILS_BLUE);
}
for (int I = 0; I < ConstraintEdgesIdx; I++)
{
    AddLine(&GlobalShapeRenderer, ConstraintEdges[I], 1.0f, GLUTILS_ORANGE);
}
AddLine(&GlobalShapeRenderer, ConstraintEdge, 1.0f, GLUTILS_GREEN);
Render(&GlobalShapeRenderer);
DebugPlatformSwapBuffers();
int shit = 0;
}
#endif
#endif
#endif
    }

    triangulation_result Result;
    Result.TriangulationTris = TriangulationTris;
    Result.NumTriangulationTris = NumTriangulationTris;
    return Result;
}


#if 0 /* TODO: WIP. This is supposed to be a faster version of CreateTriangulationFrom, where we do the 'visible' thing and more
         importantly do everything in edges instead of triangles. */
struct convex_hull_edge
{
    convex_hull_edge *NextEdge;
    convex_hull_edge *PrevEdge;

    line2 Edge;
};
convex_hull_edge CreateConvexHullEdge(v2 A, v2 B)
{
    convex_hull_edge Result = {};
    Result.Edge = LINE2(A, B);
    return Result;
}
void Reverse(convex_hull_edge *Edge)
{
    v2 Tmp = Edge->Edge.P1;
    Edge->Edge.P1 = Edge->Edge.P2;
    Edge->Edge.P2 = Tmp;
}

struct tri2_indices
{
    int Indices[3];
};
tri2_indices CreateIdxTri(int A, int B, int C)
{
    tri2_indices Result;
    Result.Indices[0] = A;
    Result.Indices[1] = B;
    Result.Indices[2] = C;
    return Result;
}

struct triangulation_generic_result
{
    line2 *Edges;
    int NumEdges;

    tri2_indices *Tris;
    int NumTris;
};
void CreateTriangulationGeneric(v2 *Points, int NumPoints)
{
    convex_hull_edge Edges[1024];
    int NumEdges = 0;
    convex_hull_edge *HeadEdge = NULL;

    tri2_indices Tris[1024];
    int NumTris = 0;

    /* Copy points since we need to sort them. */

    /* We have a dummy point. */
    int PointsSize = sizeof(v2)*(NumPoints);
    v2 *SortedPoints = (v2 *)MemAlloc(PointsSize);
    int NumSortedPoints = NumPoints + 1;
    memcpy(SortedPoints + 1, Points, PointsSize);

    /* Sort points on X, then Y. */
    {
        v2 *SortedPointsNoDummy = SortedPoints + 1;
        int NumSortedPointsNoDummy = NumSortedPoints - 1;

        /* Sort on X-coord. */
        SortArray(SortedPointsNoDummy, NumSortedPointsNoDummy, [](v2 A, v2 B) -> bool { return A.X > B.X; });

        /* To make things nice, we also want to sort vertices with same X-Coord on Y-Coord. */
        for (int StartIdx = 0; StartIdx < NumSortedPointsNoDummy - 1;)
        {
            int EndIdx; /* This is exclusive. */
            for (EndIdx = StartIdx + 1; 
                 EndIdx < NumSortedPointsNoDummy && SortedPointsNoDummy[EndIdx].X == SortedPointsNoDummy[StartIdx].X; 
                 EndIdx++);

            int NumPointsSameXCoord = EndIdx - StartIdx;

            /* Did we find more than one? */
            if (NumPointsSameXCoord > 1)
            {
                SortArray(&SortedPointsNoDummy[StartIdx], NumPointsSameXCoord, [](v2 A, v2 B) -> bool { return A.Y > B.Y; });
            }

            StartIdx = EndIdx;
        }
    }

    /* If the first 3 or more points have the same X or Y coordinate, then if we just create the first triangle with the first 3
       points, we'll have a collinear triangle, which is bad. So we create a dummy point that prevents that. */
    float DummyPointX = SortedPoints[1].X - 50.0f; /* 50.0f is arbitrary. */
    float DummyPointY = Min(SortedPoints[1].Y, SortedPoints[2].Y) - 50.0f;
    v2 DummyPoint = V2(DummyPointX, DummyPointY);
    SortedPoints[0] = DummyPoint;

    /* Create and add first triangle. */
    {
        /* We want to make sure the edges are in clockwise order. */

        v2 A = SortedPoints[0];
        v2 B = SortedPoints[1];
        v2 C = SortedPoints[2];

        Edges[0] = CreateConvexHullEdge(A, B);
        Edges[1] = CreateConvexHullEdge(B, C);
        Edges[2] = CreateConvexHullEdge(C, A);

        Edges[0].NextEdge = &Edges[1];
        Edges[1].NextEdge = &Edges[2];
        Edges[2].NextEdge = &Edges[0];

        Edges[0].PrevEdge = &Edges[2];
        Edges[1].PrevEdge = &Edges[0];
        Edges[2].PrevEdge = &Edges[1];

        bool IsCCW = CheckCCW(A, B, C);
        if (IsCCW)
        {
            /* Need to reverse them. */
            Reverse(&Edges[0]);
            Reverse(&Edges[1]);
            Reverse(&Edges[2]);
        }

        HeadEdge = &Edges[0];

        Tris[NumTris++] = CreateIdxTri(0, 1, 2);
    }

    for (int PointIdx = 3; PointIdx < NumSortedPoints; PointIdx++)
    {
        v2 Point = SortedPoints[PointIdx];

        /* Go through each convex hull edge and find the min and max angles. */
        int MinEdgeIdx;
        int MaxEdgeIdx;
        {
            v2 Up = V2(0.0f, 1.0f);
            convex_hull_edge *Edge = HeadEdge;

            /* First get min. */
            convex_hull_edge *SmallestAngleEdge = NULL;
            float SmallestAngle = FLT_MAX;
            Edge = HeadEdge;
            do
            {
                v2 Diff = Edge->Edge.P1 - Point;
                v2 DiffDir = Normalize(Diff);
                float Angle = GetAngleBetween(Up, DiffDir);
                if (Angle < SmallestAngle)
                {
                    SmallestAngleEdge = Edge;
                    SmallestAngle = Angle;
                }
                Edge = Edge->NextEdge;
            } while (Edge != HeadEdge);
            Assert(SmallestAngleEdge != NULL);

            /* Now get max. */
            convex_hull_edge *LargestAngleEdge = NULL;
            float LargestAngle = FLT_MIN;
            Edge = HeadEdge;
            do
            {
                v2 Diff = Edge->Edge.P2 - Point;
                v2 DiffDir = Normalize(Diff);
                float Angle = GetAngleBetween(Up, DiffDir);
                if (Angle > LargestAngle)
                {
                    LargestAngleEdge = Edge;
                    LargestAngle = Angle;
                }
                Edge = Edge->NextEdge;
            } while (Edge != HeadEdge);
            Assert(LargestAngleEdge != NULL);

            /* Now we have a strip of the convex hull that is visible to the point. Time to make the new edges. */
            {
                Edge = SmallestAngleEdge;
                while (1)
                {
                    Assert(NumEdges < ArrayCount(Edges));
                    Edges[NumEdges++] = CreateConvexHullEdge(Edge->Edge.P1, Point);

                    Edge = Edge->NextEdge;
                    if (Edge == LargestAngleEdge)
                    {
                        break;
                    }
                }
                /* We also need to add P2 of the largest angle edge. */
                Assert(NumEdges < ArrayCount(Edges));
                Edges[NumEdges++] = CreateConvexHullEdge(Edge->Edge.P2, Point);

                /* The smallest and largest angle edges are part of the convex hull now. So we have to remove the old ones. */
                Edge = SmallestAngleEdge;
            }
        }
    }
}
#endif

#endif
