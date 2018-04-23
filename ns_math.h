#ifndef NS_MATH_H
#define NS_MATH_H

#include "ns_common.h"

#include <math.h>
#include <float.h>


#ifndef M_PI
    #define M_PI 3.14159265358979323846f
    #define M_2PI (2*M_PI)
#endif

#define ROOT2 1.4142135623730950488016887242097f

#define TOLERANCE 0.0001f

#define EXPANDV2(V) V.X, V.Y
#define EXPANDV3(V) V.X, V.Y, V.Z
#define EXPANDV4(V) V.X, V.Y, V.Z, V.W


inline internal float
ToRadians(float Degrees)
{
    float Result = (Degrees * M_PI) / 180.0f;
    return Result;
}

inline internal float
ToDegrees(float Radians)
{
    float Result = (Radians * 180.0f) / M_PI;
    return Result;
}

inline internal float
Tan(float Radians)
{
    float Result = tanf(Radians);
    return Result;
}

inline internal float
Sin(float Radians)
{
    float Result = sinf(Radians);
    return Result;
}

inline internal float
Cos(float Radians)
{
    float Result = cosf(Radians);
    return Result;
}

inline internal float
Acos(float Value)
{
    Assert(Value <= 1.0f && Value >= -1.0f);
    float Result = acosf(Value);
    return Result;
}

inline internal float
Sqrt(float X)
{
    float Result = sqrtf(X);
    return Result;
}

inline internal float
Pow(float Base, float Exponent)
{
    float Result = powf(Base, Exponent);
    return Result;
}

inline internal void
Swap(float *A, float *B)
{
    float Tmp = *A;
    *A = *B;
    *B = Tmp;
}

inline internal float
Clamp01(float X)
{
    if(X < 0.0f)
    {
        X = 0.0f;
    }
    else if(X > 1.0f)
    {
        X = 1.0f;
    }

    return X;
}

inline internal float
Abs(float X)
{
    float Result = X < 0 ? -X : X;
    return Result;
}

inline internal bool
IsWithinTolerance(float ActualValue, float Value)
{
    bool Result = (ActualValue > Value - TOLERANCE) && (ActualValue < Value + TOLERANCE);
    return Result;
}

inline internal int
min(int A, int B)
{
    int Result = A < B ? A : B;
    return Result;
}

inline internal int
max(int A, int B)
{
    int Result = A > B ? A : B;
    return Result;
}

inline internal uint32_t
min(uint32_t A, uint32_t B)
{
    uint32_t Result = A < B ? A : B;
    return Result;
}

inline internal uint32_t
max(uint32_t A, uint32_t B)
{
    uint32_t Result = A > B ? A : B;
    return Result;
}

inline internal float
min(float A, float B)
{
    float Result = A < B ? A : B;
    return Result;
}

inline internal float
max(float A, float B)
{
    float Result = A > B ? A : B;
    return Result;
}

#endif
