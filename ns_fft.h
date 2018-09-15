#ifndef NS_FFT
#define NS_FFT

#include <stdlib.h>
#include <complex.h>
#undef I
#include "ns_common.h"
#include "ns_math.h"
#include "ns_memory.h"

internal void
DiscreteFourierTransform(Complex *X, int N)
{
    Complex *Y = (Complex *)MemAllocZero(sizeof(Complex)*N);

    for (int k = 0; k < N; k++)
    {
        for (int n = 0; n < N; n++)
        {
            Complex xn = X[n];
            float y = 2.0f*M_PI*(float)k*(float)n/(float)N;
            Complex w = EulerFormula(y);
            Y[k] += xn*w;
        }
    }

    MemCpy(X, Y, sizeof(Complex)*N);
}

internal void
NsFFTSeparate(Complex *X, int HalfN)
{
    Complex *Temp = (Complex *)malloc(sizeof(Complex)*HalfN);

    for (int I = 0; I < HalfN; I++)
    {
        Temp[I] = X[2*I+1];
    }

    for (int I = 0; I < HalfN; I++)
    {
        X[I] = X[2*I];
    }

    for (int I = 0; I < HalfN; I++)
    {
        X[I + HalfN] = Temp[I];
    }

    free(Temp);
    return;
}

internal void
FastFourierTransformRecursive(Complex *X, int N)
{
    if (N < 2)
    {
        /* Do nothing. */
    }
    else
    {
        int HalfN = N/2;
        NsFFTSeparate(X, HalfN);
        FastFourierTransformRecursive(X, HalfN);
        FastFourierTransformRecursive(X + HalfN, HalfN);
        for (int k = 0; k < HalfN; k++)
        {
            Complex E = X[k];
            Complex O = X[k + HalfN];
            Complex W = EulerFormula(2.0f*M_PI*(float)k/(float)N);
            X[k] = E + W*O;
            X[k + HalfN] = E - W*O;
        }
    }
}

internal void
FastFourierTransformIterative(Complex *X, int N)
{
    int HalfN = N/2;

    /* Reorder data. */
    {
        Complex *Y = (Complex *)PushPerFrameMemory(sizeof(Complex)*N);
        for (uint32_t I = 0; I < (uint32_t)N; I++)
        {
            uint32_t Reversed = ReverseBits(I, (uint32_t)N);
            Y[I] = X[Reversed];
        }
        MemCpy(X, Y, sizeof(Complex)*N);
        PopPerFrameMemory();
    }

    /* We use SizeBelow because Size might overflow, resulting in 0, which is another annoying, unnecessary check. */
    for (int SizeBelow = 1; SizeBelow != N; SizeBelow *= 2)
    {
        int Size = 2*SizeBelow;
        for (int BelowStartIdx = 0; BelowStartIdx < N; BelowStartIdx += Size)
        {
            for (int k = 0; k < SizeBelow; k++)
            {
                int BelowIdx = BelowStartIdx + k;
                Complex E = X[BelowIdx];
                Complex O = X[BelowIdx + SizeBelow];
                Complex W = EulerFormula(2.0f*M_PI*(float)k/(float)Size);
                X[BelowIdx] = E + W*O;
                X[BelowIdx + SizeBelow] = E - W*O;
            }
        }
    }
}

enum fft_flavor
{
    Recursive, Iterative, Discrete
};

internal uint32_t
GetFFTBytesRequired(int N)
{
    int SamplesRequired = NextPowerOfTwo(N);
    uint32_t Result = sizeof(Complex)*SamplesRequired;
    return Result;
}

internal Complex *
FastFourierTransform(Complex *Fourier, void *X, int N, int XSize)
{
    MemSet0(Fourier, sizeof(Complex)*N);
    int NPot = NextPowerOfTwo(N);
    /* We _could_ read byte by byte, but then it's not endian agnostic. */
    switch (XSize)
    {
        case sizeof(uint8_t):
        {
            for (int I = 0; I < N; I++)
            {
                Fourier[I].Real = ((uint8_t *)X)[I];
            }
        } break;

        case sizeof(uint16_t):
        {
            for (int I = 0; I < N; I++)
            {
                Fourier[I].Real = ((uint16_t *)X)[I];
            }
        } break;
    }
    FastFourierTransformIterative(Fourier, NPot);
    return Fourier;
}

internal void
FastFourierTransform(Complex *Fourier, uint8_t *X, int N)
{
    FastFourierTransform(Fourier, X, N, sizeof(*X));
}

internal void
FastFourierTransform(Complex *Fourier, uint16_t *X, int N)
{
    FastFourierTransform(Fourier, X, N, sizeof(*X));
}

#if 0
internal void
FastFourierTest()
{
    uint16_t Test[16];
    for (uint16_t i = 0; i < ArrayCount(Test); i++)
    {
        Test[i] = i;
    }

    Complex *FourierRecursive = FastFourierTransformRecursive(Test, ArrayCount(Test));
    Complex *FourierIterative = FastFourierTransformIterative(Test, ArrayCount(Test));
}
#endif

#endif