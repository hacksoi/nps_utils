#ifndef NS_MUSIC_VISUALIZER_H
#define NS_MUSIC_VISUALIZER_H

/* TODO: Clean this up. Especially the initialization. */

#include "ns_common.h"
#include "ns_ttf.h"
#include "ns_shape_renderer.h"

#if 0 /* Example */
MusicVisualizer.CurrentSampleIdx = 0;
MusicVisualizer.MaxSampleSize = MAX_SAMPLE_SIZE;
MusicVisualizer.Timeout = (float)MusicVisualizer.MaxSampleSize/(float)Song.SampleRate;
MusicVisualizer.TimePassed = MusicVisualizer.Timeout; /* Make it run the first frame. */
MusicVisualizer.RangeCount = ArrayCount(MusicVisualizer.NormalizedAverageMagnitudes);
MusicVisualizer.MaxRangeSize = MusicVisualizer.MaxSampleSize/MusicVisualizer.RangeCount;
MusicVisualizer.MaxBarHeight = (float)WindowHeight*(2.0f/3.0f);
MusicVisualizer.BarWidth = (float)WindowWidth/(float)MusicVisualizer.RangeCount;
#endif

#define MAX_SAMPLE_SIZE Kilobytes(2)
#define MAX_RANGES 128
struct music_visualizer
{
    float TimePassed;
    float Timeout;
    uint32_t CurrentSampleIdx;
    uint32_t MaxSampleSize;
    uint32_t MaxRangeSize;
    float MaxBarHeight;
    float BarWidth;
    float NormalizedAverageMagnitudes[MAX_RANGES];
    uint32_t RangeCount;
};

internal void
Update(music_visualizer *MusicVisualizer, ns_song *Song, float dt)
{
    MusicVisualizer->TimePassed += dt;
    if (MusicVisualizer->TimePassed >= MusicVisualizer->Timeout)
    {
        MusicVisualizer->CurrentSampleIdx = GetCurrentSampleIndex(Song);
        Assert(MusicVisualizer->CurrentSampleIdx < Song->NumSamples);

        uint32_t NumSamples;
        if (MusicVisualizer->CurrentSampleIdx == Song->NumSamples)
        {
            NumSamples = 0;
        }
        else if (MusicVisualizer->CurrentSampleIdx + MusicVisualizer->MaxSampleSize >= Song->NumSamples)
        {
            NumSamples = Song->NumSamples - MusicVisualizer->CurrentSampleIdx;
        }
        else
        {
            NumSamples = MusicVisualizer->MaxSampleSize;
        }
        Assert(MusicVisualizer->CurrentSampleIdx + NumSamples <= Song->NumSamples);

        if (NumSamples > 1)
        {
            Complex *FourierData = (Complex *)PushPerFrameMemory(GetFFTBytesRequired(NumSamples));
            FastFourierTransform(Song, FourierData, MusicVisualizer->CurrentSampleIdx, NumSamples);

            /* Fuck the first one. */
            FourierData[0].Real = 0.0f;
            FourierData[0].Imaginary = 0.0f;

            /* We half it cause the second half of FFT is a reflection of the first half. */
            int OriginalNumSamples = NumSamples;
            NumSamples /= 2;
            {
                uint32_t RangeSize = NumSamples/MusicVisualizer->RangeCount;

                float MaxRangeAverageMagnitude = 0.0f;
                for (uint32_t RangeIdx = 0; RangeIdx < MusicVisualizer->RangeCount; RangeIdx++)
                {
                    Complex *RangePtr = &FourierData[RangeIdx*RangeSize];
                    Complex *RangeEndPtr = RangePtr + RangeSize;
                    if (RangeIdx == MusicVisualizer->RangeCount - 1)
                    {
                        uint32_t Remainder = NumSamples % MusicVisualizer->RangeCount;
                        RangeEndPtr += Remainder;
                    }
                    Assert(RangeEndPtr <= FourierData + NumSamples);

                    float TotalMagnitude = 0.0f;
                    while (RangePtr != RangeEndPtr)
                    {
                        Complex C = *RangePtr++;
                        float Magnitude = GetMagnitude(C);
                        TotalMagnitude += Magnitude;
                        Assert(!IsInfinity(TotalMagnitude));
                    }

                    float AverageMagnitude = TotalMagnitude/MusicVisualizer->MaxRangeSize;
                    Assert(!IsInfinity(AverageMagnitude));

                    if (AverageMagnitude > MaxRangeAverageMagnitude)
                    {
                        MaxRangeAverageMagnitude = AverageMagnitude;
                    }

                    MusicVisualizer->NormalizedAverageMagnitudes[RangeIdx] = AverageMagnitude;
                }

                /* Do scaling. */
                for (uint32_t RangeIdx = 0; RangeIdx < MusicVisualizer->RangeCount; RangeIdx++)
                {
                    MusicVisualizer->NormalizedAverageMagnitudes[RangeIdx] /= MaxRangeAverageMagnitude;
                }
            }
            NumSamples = OriginalNumSamples;

            PopPerFrameMemory();
            MusicVisualizer->TimePassed = 0.0f;
        }
    }
}

internal void
Render(music_visualizer *MusicVisualizer, shape_renderer *ShapeRenderer)
{
	v4 Colors[] = { GLUTILS_BLUE, GLUTILS_RED };

	float BarPos = 0.0f;
	for (uint32_t RangeIdx = 0; RangeIdx < MusicVisualizer->RangeCount; RangeIdx++)
	{
		float BarHeight = MusicVisualizer->MaxBarHeight*MusicVisualizer->NormalizedAverageMagnitudes[RangeIdx];

		quad2 Bar;
		Bar.BottomLeft = V2(BarPos, 0.0f);
		Bar.BottomRight = V2(BarPos + MusicVisualizer->BarWidth, 0.0f);
		Bar.TopLeft = V2(BarPos, BarHeight);
		Bar.TopRight = V2(BarPos + MusicVisualizer->BarWidth, BarHeight);

		AddQuad(ShapeRenderer, Bar, Colors[RangeIdx % ArrayCount(Colors)]);

		BarPos += MusicVisualizer->BarWidth;
	}
	Render(ShapeRenderer);
}

#endif