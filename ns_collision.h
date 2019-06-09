#ifndef NS_COLLISION
#define NS_COLLISION

#include "ns_game_math.h"

struct collision
{
    int EntityA;
    int EntityB;
    direction IntersectionDirection;
    struct collision *Next;
};

bool CheckCollidesWith(rect2 ABoundingBox, v2 ADelta,
                       rect2 BBoundingBox, v2 BDelta,
                       float *t, axis *Axis)
{
    return false;
}

/* This function adjusts the entity deltas, which the caller applies after.
   It returns the collisions in the specified array; each collision involves
   2 entities, and therefore two collisions are generated for each collision -
   one where entity X is EntityA is first, and one where entity Y is EntityB -
   we do this because it is easier on the user.
   
   A touch is not a collision - the definition of a collision is that the two objects
   overlapped due a change in delta; a touch is where the two objects abutt with no
   delta. */
void DoCollisionDetection(collision *Collisions, int CollisionsSize, int *NumCollisionsPtr,
                          rect2 *BoundingBoxes, v2 *InputDeltas, bool *Skip,
                          v2 *OutputDeltas, v2 *Touches,
                          int NumEntities)
{
    *NumCollisionsPtr = 0;

    for_ (CurEntityIdx, NumEntities)
    {
        if (Skip[CurEntityIdx])
        {
            continue;
        }

        float ClosestT = FLT_MAX;
        axis ClosestAxis;
        int EntityIdxCollidedWith;
        for_ (EntityIdx, NumEntities)
        {
            if (EntityIdx == CurEntityIdx ||
                Skip[EntityIdx])
            {
                continue;
            }

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
                    EntityIdxCollidedWith = EntityIdx;
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
            for_ (EntityIdx, NumEntities)
            {
                if (EntityIdx == CurEntityIdx)
                {
                    continue;
                }

                if (!Skip[EntityIdx])
                {
                    float t;
                    axis Axis; /* Note that the axis must be the non-collision-axis, since the delta for the collision-axis should be zero. */
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
                ArrayPtrGetLastAndAddOne(collision *Collision, Collisions, CollisionsSize, *NumCollisionsPtr);
                Collision->EntityA = CurEntityIdx;
                Collision->EntityB = EntityIdxCollidedWith;

                /* Adjust the delta of the axis that the collision happened on. */
                OutputDeltas[CurEntityIdx][ClosestAxis] = ClosestT*InputDeltas[CurEntityIdx][ClosestAxis];
            }
        }
    }
}
#endif
