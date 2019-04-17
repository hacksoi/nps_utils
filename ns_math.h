#ifndef NS_MATH_H
#define NS_MATH_H

#include "ns_common.h"

#include <math.h>
#include <float.h>


#ifndef M_PI
    #define M_PI 3.14159265358979323846f
    #define M_2PI (2.0f*M_PI)
#endif

#define ROOT2 1.4142135623730950488016887242097f

#define TOLERANCE 0.0003f

#define EXPANDV2(V) V.X, V.Y
#define EXPANDV3(V) V.X, V.Y, V.Z
#define EXPANDV4(V) V.X, V.Y, V.Z, V.W

struct Complex
{
    float Real;
    float Imaginary;

    void operator+=(Complex A);
    void operator/=(float A);
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
    Result.Real = A.Real - B.Real;
    Result.Imaginary = A.Imaginary - B.Imaginary;
    return Result;
}

internal Complex
operator*(Complex A, Complex B)
{
    Complex Result;
    Result.Real = A.Real*B.Real - A.Imaginary*B.Imaginary;
    Result.Imaginary = A.Real*B.Imaginary + A.Imaginary*B.Real;
    return Result;
}

internal bool
operator==(Complex A, Complex B)
{
    bool Result = (A.Real == B.Real) && (A.Imaginary == B.Imaginary);
    return Result;
}

internal bool
operator!=(Complex A, Complex B)
{
    bool Result = !(A == B);
    return Result;
}

void
Complex::operator+=(Complex A)
{
    this->Real += A.Real;
    this->Imaginary += A.Imaginary;
}

void
Complex::operator/=(float X)
{
    this->Real /= X;
    this->Imaginary /= X;
}

internal float Sqrt(float X);
internal float
GetMagnitude(Complex A)
{
    float Result = Sqrt(A.Real*A.Real + A.Imaginary*A.Imaginary);
    return Result;
}

inline internal bool
IsWithinTolerance(float ActualValue, float Value)
{
    bool Result = (ActualValue > Value - TOLERANCE) && (ActualValue < Value + TOLERANCE);
    return Result;
}

bool GTE_Tolerance(float A, float B)
{
    bool Result = IsWithinTolerance(A, B) || A > B;
    return Result;
}

bool LTE_Tolerance(float A, float B)
{
    bool Result = IsWithinTolerance(A, B) || A < B;
    return Result;
}

inline internal float
ToRadians(float Degrees)
{
    float Result = (Degrees*M_PI)/180.0f;
    return Result;
}

inline internal float
ToDegrees(float Radians)
{
    float Result = (Radians*180.0f)/M_PI;
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
    Assert((Value < 1.0f || IsWithinTolerance(Value, 1.0f)) && (Value > -1.0f || IsWithinTolerance(Value, -1.0f)));
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

#if 0 /* When would we ever need to use this? */
internal bool
HasFraction(float A)
{
    bool Result = (A == (int)A);
    return Result;
}
#endif

internal int
Ceil(float A)
{
    int Result = (int)ceilf(A);
    return Result;
}

internal void
Floor(float *A)
{
    *A = (float)floorf(*A);
}

internal int
Floor(float A)
{
    int Result = (int)floorf(A);
    return Result;
}

inline internal float
Clamp01(float X)
{
    if (X < 0.0f)
    {
        X = 0.0f;
    }
    else if (X > 1.0f)
    {
        X = 1.0f;
    }
    return X;
}

void Clamp(float *X, float Min, float Max)
{
    if (*X < Min)
    {
        *X = Min;
    }
    else if (*X > Max)
    {
        *X = Max;
    }
}

inline internal float
Abs(float X)
{
    float Result = X < 0 ? -X : X;
    return Result;
}

int Abs(int X)
{
    int Result = X < 0 ? -X : X;
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

int Max(int A, int B)
{
    int Result = A > B ? A : B;
    return Result;
}

inline internal float
Max(float A, float B)
{
    float Result = A > B ? A : B;
    return Result;
}

int Min(int A, int B)
{
    int Result = A < B ? A : B;
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
    Result.Imaginary = -Sin(X);
    return Result;
}

/* If it's already a POT, returns Value. */
internal int
NextPowerOfTwo(int Value)
{
    /* TODO: Could shift in from MSB until we hit a 1... */
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
ReverseBytes(void *VoidValuePtr, int Size)
{
    uint8_t *ValuePtr = (uint8_t *)VoidValuePtr;
    for (int I = 0; I < Size/2; I++)
    {
        uint8_t Tmp = *(ValuePtr + I);
        *(ValuePtr + I) = *(ValuePtr + (Size - I - 1));
        *(ValuePtr + (Size - I - 1)) = Tmp;
    }
}

internal void 
ReverseBytes(uint8_t *ValuePtr, int Size)
{
    ReverseBytes((void *)ValuePtr, Size);
}

internal void 
ReverseBytes(int16_t *Value)
{
    ReverseBytes((uint8_t *)Value, sizeof(*Value));
}

internal void 
ReverseBytes(uint16_t *Value)
{
    ReverseBytes((uint8_t *)Value, sizeof(*Value));
}

internal void 
ReverseBytes(int32_t *Value)
{
    ReverseBytes((uint8_t *)Value, sizeof(*Value));
}

internal void 
ReverseBytes(uint32_t *Value)
{
    ReverseBytes((uint8_t *)Value, sizeof(*Value));
}

internal bool
IsPowerOfTwo(int X)
{
    bool Result = (X && !(X & (X - 1)));
    return Result;
}

/* TODO: Lookup table of 8-bit chunks. */
/* OnePastMaxBitSet example: if N is 16, then OnePastMaxBitSet is 1000 because for 16, max is 0111.  */
internal uint32_t
ReverseBits(uint32_t X, uint32_t OnePastMaxBitSet)
{
    Assert(X < OnePastMaxBitSet);
    uint32_t OriginalX = X;
    uint32_t Result = 0;
    X |= OnePastMaxBitSet;
    while (1)
    {
        int Bit = X & 1;
        Result |= Bit;
        X >>= 1;
        if (X == 1)
        {
            break;
        }
        Result <<= 1;
    }
    return Result;
}

internal void
ReverseBitsTest()
{
    uint32_t Result;
    Result = ReverseBits(0x01, 0x02);
    Assert(Result == 0x01);
    Result = ReverseBits(0x07, 0x08);
    Assert(Result == 0x07);
    Result = ReverseBits(0x05, 0x08);
    Assert(Result == 0x05);
    Result = ReverseBits(0x06, 0x08);
    Assert(Result == 0x03);
}

internal bool
IsInfinity(float X)
{
    bool Result = isinf(X);
    return Result;
}

float Lerp(float A, float t, float B)
{
    float Result = A + t*(B - A);
    return Result;
}

internal uint32_t
ConvertStringToInt(const char *String)
{
    uint32_t Result = 0;
    for (int Shift = 24; Shift >= 0; Shift -= 8)
    {
        uint32_t Byte = *String++;
        Result |= (Byte << Shift);
    }
    return Result;
}

internal bool
IsLittleEndian()
{
    uint16_t X = 0xaabb;
    uint8_t *XPtr = (uint8_t *)&X;
    bool Result = (*XPtr == (X & 0xff));
    return Result;
}

internal void
GetValueBE(uint8_t *Source, uint8_t *Dest, int Size)
{
    if (IsLittleEndian())
    {
        for (int I = 0; I < Size; I++)
        {
            Dest[I] = Source[Size - I - 1];
        }
    }
    else
    {
        for (int I = 0; I < Size; I++)
        {
            Dest[I] = Source[I];
        }
    }
}

internal uint16_t *
FixBigEndian(uint16_t *Value)
{
    if (IsLittleEndian())
    {
        ReverseBytes(Value, sizeof(*Value));
    }
    return Value;
}

internal int16_t *
FixBigEndian(int16_t *Value)
{
    if (IsLittleEndian())
    {
        ReverseBytes(Value, sizeof(*Value));
    }
    return Value;
}

internal int32_t *
FixBigEndian(int32_t *Value)
{
    if (IsLittleEndian())
    {
        ReverseBytes(Value, sizeof(*Value));
    }
    return Value;
}

internal uint32_t *
FixBigEndian(uint32_t *Value)
{
    if (IsLittleEndian())
    {
        ReverseBytes(Value, sizeof(*Value));
    }
    return Value;
}

internal uint64_t *
FixBigEndian(uint64_t *Value)
{
    if (IsLittleEndian())
    {
        ReverseBytes(Value, sizeof(*Value));
    }
    return Value;
}

internal uint8_t *
FixBigEndian(uint8_t *Value, int Size)
{
    switch (Size)
    {
        case 2:
        {
            FixBigEndian((uint16_t *)Value);
        } break;

        case 4:
        {
            FixBigEndian((uint32_t *)Value);
        } break;

        case 8:
        {
            FixBigEndian((uint64_t *)Value);
        } break;
    }
    return Value;
}

internal void
FixBigEndianStruct(uint8_t *Struct, int *Sizes, int NumMembers)
{
    if (IsLittleEndian())
    {
        uint8_t *Tmp = Struct;
        for (int I = 0; I < NumMembers; I++)
        {
            ReverseBytes(Tmp, Sizes[I]);
            Tmp += Sizes[I];
        }
    }
}

internal void
FixBigEndianArray(void *ArrayVoid, int NumElements, int ElementSize)
{
    if (IsLittleEndian())
    {
        uint8_t *Array = (uint8_t *)ArrayVoid;
        for (int I = 0; I < NumElements; I++)
        {
            ReverseBytes(Array + I*ElementSize, ElementSize);
        }
    }
}

internal uint16_t *
FixBigEndianArray(uint16_t *Array, int Size)
{
    FixBigEndianArray((void *)Array, Size, sizeof(*Array));
    return Array;
}

internal int16_t *
FixBigEndianArray(int16_t *Array, int Size)
{
    FixBigEndianArray((void *)Array, Size, sizeof(*Array));
    return Array;
}

internal uint32_t *
FixBigEndianArray(uint32_t *Array, int Size)
{
    FixBigEndianArray((void *)Array, Size, sizeof(*Array));
    return Array;
}

internal uint8_t *
FixBigEndianArray(uint8_t *Array, int Size)
{
    FixBigEndianArray((void *)Array, Size, sizeof(*Array));
    return Array;
}

void EnsureSmallerIsFirst(float *X, float *Y)
{
    if (*X > *Y)
    {
        Swap(X, Y);
    }
}

float Remap(float A, float FromMin, float FromMax, float ToMin, float ToMax)
{
    float FromRange = FromMax - FromMin;
    float ToRange = ToMax - ToMin;
    float Normalized = (A - FromMin)/FromRange;
    float Result = ToMin + Normalized*ToRange;
    return Result;
}

uint32_t Join(uint8_t A, uint8_t B, uint8_t C, uint8_t D)
{
    uint32_t Result = ((A << 24) |
                       (B << 16) |
                       (C << 8) |
                       (D << 0));
    return Result;
}

int GetLowestBitSet(u32 Value)
{
    int BitIdx;
    for (BitIdx = 0; BitIdx < 8*sizeof(Value); BitIdx++)
    {
        if (Value & (1 << BitIdx))
        {
            break;
        }
    }
    return BitIdx;
}

#endif
