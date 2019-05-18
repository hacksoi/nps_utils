#ifndef NS_COLLISION
#define NS_COLLISION

#include "ns_game_math.h"

struct collision
{
    int EntityA;
    int EntityB;

    union
    {
        struct
        {
            bool IntersectedLeft;
            bool IntersectedRight;
            bool IntersectedBottom;
            bool IntersectedTop;
        };
        bool IntersectedValues[4];
    };
};

bool CheckCollidesWith(rect2 ABoundingBox, v2 ADelta,
                       rect2 BBoundingBox, v2 BDelta,
                       float *t, axis *Axis)
{
    return false;
}

/* This function adjusts the entity deltas, which the caller applies after. */
void DoCollisionDetection(collision *Collisions, int CollisionsSize, int *NumCollisionsPtr,
                          rect2 *BoundingBoxes, v2 *InputDeltas, v2 *OutputDeltas, bool *AffectedByCollisions, bool *Skip,
                          int NumEntities)
{
    *NumCollisionsPtr = 0;

    float ClosestT = FLT_MAX;
    axis ClosestAxis;
    int EntityIdxCollidedWith;
    for_ (CurEntityIdx, NumEntities)
    {
        if (AffectedByCollisions[CurEntityIdx])
        {
            for_ (EntityIdx, NumEntities)
            {
                if (EntityIdx == CurEntityIdx)
                {
                    continue;
                }

                if (!Skip[EntityIdx])
                {
                    float t;
                    axis Axis;
                    bool CollidesWith = CheckCollidesWith(BoundingBoxes[CurEntityIdx], InputDeltas[CurEntityIdx],
                                                          BoundingBoxes[EntityIdx], InputDeltas[EntityIdx],
                                                          &t, &Axis);
                    if (CollidesWith)
                    {
                        if (t < ClosestT)
                        {
                            ClosestT = t;
                            ClosestAxis = Axis;
                        }
                    }
                }
            }

            bool CollisionOcurred = ClosestT != FLT_MAX;
            if (CollisionOcurred)
            {
                /* Add the collision. */
                ArrayPtrGetLastAndAddOne(collision *Collision, Collisions, CollisionsSize, *NumCollisionsPtr);
                Collision->EntityA = CurEntityIdx;
                Collision->EntityB = EntityIdxCollidedWith;

                /* Adjust the delta of the axis that the collision happened on. */
                OutputDeltas[CurEntityIdx][ClosestAxis] = ClosestT*InputDeltas[CurEntityIdx][ClosestAxis];

                /* Get how much is left of the delta for the other axis. */
                axis OtherAxis = (axis)(1 - ClosestAxis);
                float InverseT = 1.0f - ClosestT;
                v2 NewDelta = {};
                NewDelta[OtherAxis] = InverseT*InputDeltas[CurEntityIdx][OtherAxis];

                ClosestT = FLT_MAX;
                for_(EntityIdx, NumEntities)
                {
                    if (EntityIdx == CurEntityIdx)
                    {
                        continue;
                    }

                    if (!Skip[EntityIdx])
                    {
                        float t;
                        axis Axis;
                        bool CollidesWith = CheckCollidesWith(BoundingBoxes[CurEntityIdx], NewDelta,
                                                              BoundingBoxes[EntityIdx], InputDeltas[EntityIdx],
                                                              &t, &Axis);
                        if (CollidesWith)
                        {
                            if (t < ClosestT)
                            {
                                ClosestT = t;
                                ClosestAxis = Axis;
                            }
                        }
                    }
                }

                CollisionOcurred = ClosestT != FLT_MAX;
                if (CollisionOcurred)
                {
                    Assert(ClosestAxis == OtherAxis);

                    /* Add the collision. */
                    ArrayPtrGetLastAndAddOne(collision* Collision, Collisions, CollisionsSize, *NumCollisionsPtr);
                    Collision->EntityA = CurEntityIdx;
                    Collision->EntityB = EntityIdxCollidedWith;

                    /* Adjust the delta of the axis that the collision happened on. */
                    OutputDeltas[CurEntityIdx][ClosestAxis] = ClosestT*InputDeltas[CurEntityIdx][ClosestAxis];
                }
            }
        }
    }
}
#endif