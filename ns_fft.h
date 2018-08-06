#ifndef NS_FFT
#define NS_FFT

#include <stdlib.h>
#include "ns_common.h"
#include "ns_math.h"

internal float *
DiscreteFourierTransform(uint16_t *Samples, int NumSamples)
{
    float *Fourier = (float *)malloc(2*NumSamples);
    int N = NumSamples;
    for (int k = 0; k < N - 1; k++)
    {
        Fourier[k] = 0;
        for (int n = 0; n < NumSamples - 1; n++)
        {
            float xn = Samples[n];
            float y = 2*M_PI*n*k/N;
            Fourier[k] = xn*Cos(y);
            Fourier[k + 1] = xn*Sin(y);
        }
    }
}

internal void
NsFTSeparate(Complex *X, int HalfN)
{
    /* TODO: Allocate once at start, free at end. */
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
        NsFTSeparate(X, HalfN);
        FastFourierTransformRecursive(X, HalfN);
        FastFourierTransformRecursive(X + HalfN, HalfN);
        for (int k = 0; k < HalfN; k++)
        {
            Complex E = X[k];
            Complex O = X[k + HalfN];
            Complex W = EulerFormula(-2.0f*M_PI*k/N);
            X[k] = E + W*O;
            X[k + HalfN] = E - W*O;
        }
    }
}

#endif