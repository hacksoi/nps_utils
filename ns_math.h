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

struct Complex
{
    float Real;
    float Imaginary;
};

internal Complex 
COMPLEX(float Real, float Imaginary)
{
    Complex Result = { Real, Imaginary };
    return Result;
}

internal Complex
operator+(Complex A, Complex B)
{
    Complex Result;
    Result.Real = A.Real + B.Real;
    Result.Imaginary = A.Imaginary + B.Imaginary;
    return Result;
}

internal Complex
operator-(Complex A, Complex B)
{
    Complex Result;
    Result.Real = A.Real + B.Real;
    Result.Imaginary = A.Imaginary + B.Imaginary;
    return Result;
}

internal Complex
operator*(Complex A, Complex B)
{
    Complex Result;
    Result.Real = A.Real * B.Real;
    Result.Imaginary = A.Imaginary * B.Imaginary;
    return Result;
}

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
Exp(float X)
{
    float Result = expf(X);
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
ns_math_min(int A, int B)
{
    int Result = A < B ? A : B;
    return Result;
}

inline internal int
ns_math_max(int A, int B)
{
    int Result = A > B ? A : B;
    return Result;
}

inline internal uint32_t
ns_math_min(uint32_t A, uint32_t B)
{
    uint32_t Result = A < B ? A : B;
    return Result;
}

inline internal uint32_t
ns_math_max(uint32_t A, uint32_t B)
{
    uint32_t Result = A > B ? A : B;
    return Result;
}

inline internal float
Min(float A, float B)
{
    float Result = A < B ? A : B;
    return Result;
}

inline internal float
ns_math_min(float A, float B)
{
    float Result = Min(A, B);
    return Result;
}

inline internal float
ns_math_max(float A, float B)
{
    float Result = A > B ? A : B;
    return Result;
}

inline internal float
Log10f(float X)
{
    float Result = log10f(X);
    return Result;
}

inline internal float
Log2f(float X)
{
    float Result = log2f(X);
    return Result;
}

internal Complex
EulerFormula(float X)
{
    Complex Result;
    Result.Real = Cos(X);
    Result.Imaginary = Sin(X);
    return Result;
}

/* If it's already a POT, returns Value. */
internal int
NextPowerOfTwo(int Value)
{
    int NumBits = 8*sizeof(Value);
    for (int Bit = 0; Bit < NumBits; Bit++)
    {
        int POT = 1 << Bit;
        if (Value <= POT)
        {
            return POT;
        }
    }

    /* Should never get here. */
    Assert(false);
    return -1;
}

internal void 
Swap(uint8_t *A, uint8_t *B)
{
    uint8_t Tmp = *A;
    *A = *B;
    *B = Tmp;
}

internal void 
ReverseEndianness(uint32_t *_Value)
{
    uint8_t *Value = (uint8_t *)_Value;
    Swap(Value + 0, Value + 3);
    Swap(Value + 1, Value + 2);
}

#endif
