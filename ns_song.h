#ifndef NS_SONG
#define NS_SONG

#include <xaudio2.h>
#include "ns_common.h"
#include "ns_wav.h"
#include "ns_memory.h"

struct ns_song
{
    IXAudio2SourceVoice* SourceVoice;
    XAUDIO2_BUFFER XAudio2Buffer;

    ns_wav WavFile;

    void *Samples;
    uint16_t NumChannels;
    uint32_t NumSamples;
    uint32_t SampleRate;
    uint16_t BitsPerSample;
    uint16_t BytesPerSample;
    float Duration;
    void *SamplesAverageBuffer;
    uint32_t LastTotalSamplesPlayed;
    bool IsPlaying;
};

class XAudio2VoiceCallback : public IXAudio2VoiceCallback
{
public:
    STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext)
    {
        ns_song *Song = (ns_song *)pBufferContext;
        Song->IsPlaying = false;
    }

    STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext)
    {
        ns_song *Song = (ns_song *)pBufferContext;
        Song->IsPlaying = true;
    }

    STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) { }
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) { }
    STDMETHOD_(void, OnStreamEnd) (THIS) { }
    STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) { }
    STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) { }
};

/* @race_condition */
global IXAudio2 *GlobalXAudio2;
global IXAudio2MasteringVoice *GlobalMasterVoice;
global bool GlobalXAudio2Created;
global XAudio2VoiceCallback GlobalXAudio2VoiceCallback;

internal bool
HasEnded(ns_song *Song)
{
    XAUDIO2_VOICE_STATE VoiceState;
    Song->SourceVoice->GetState(&VoiceState, 0);
    uint32_t Result = VoiceState.BuffersQueued == 0;
    return Result;
}

internal bool
IsPlaying(ns_song *Song)
{
    return Song->IsPlaying;
}

internal void
Pause(ns_song *Song)
{
    CheckSOK(Song->SourceVoice->Stop(0));
    Song->IsPlaying = false;
}

internal void
Play(ns_song *Song)
{
    if (HasEnded(Song))
    {
        Song->XAudio2Buffer.PlayBegin = 0;
        CheckSOK(Song->SourceVoice->SubmitSourceBuffer(&Song->XAudio2Buffer));
    }
    CheckSOK(Song->SourceVoice->Start(0));
    Song->IsPlaying = true;
}

internal void
CreateSong(ns_song *Song, char *Filename)
{
    if (!GlobalXAudio2Created)
    {
        CheckSOK(XAudio2Create(&GlobalXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR));
        CheckSOK(GlobalXAudio2->CreateMasteringVoice(&GlobalMasterVoice));
        GlobalXAudio2Created = true;
    }

    Song->WavFile = LoadWav(Filename);
    Song->XAudio2Buffer.AudioBytes = Song->WavFile.WavDataChunk->Header.Size;  //size of the audio buffer in bytes
    Song->XAudio2Buffer.pAudioData = (const BYTE *)&Song->WavFile.WavDataChunk->FirstSample;  //buffer containing audio data
    Song->XAudio2Buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
    Song->XAudio2Buffer.pContext = (void *)Song;
    CheckSOK(GlobalXAudio2->CreateSourceVoice(&Song->SourceVoice, (WAVEFORMATEX*)&Song->WavFile.WavFmtChunk->Data, 0, 
                XAUDIO2_DEFAULT_FREQ_RATIO, &GlobalXAudio2VoiceCallback));
    CheckSOK(Song->SourceVoice->SubmitSourceBuffer(&Song->XAudio2Buffer));

    Song->Samples = (void *)&Song->WavFile.WavDataChunk->FirstSample;
    Song->BitsPerSample = Song->WavFile.WavFmtChunk->BitsPerSample;
    Song->BytesPerSample = Song->BitsPerSample/8;
    Song->NumChannels = Song->WavFile.WavFmtChunk->NumChannels;
    Song->NumSamples = (Song->WavFile.WavDataChunk->Header.Size/Song->BytesPerSample)/Song->WavFile.WavFmtChunk->NumChannels;
    Song->SampleRate = Song->WavFile.WavFmtChunk->SampleRate;
    Song->Duration = (float)Song->NumSamples/(float)Song->SampleRate;
    Song->SamplesAverageBuffer = NULL;
    Song->LastTotalSamplesPlayed = 0;
    Song->IsPlaying = false;
}

internal void
CreateSong(ns_song *Song, const char *Filename)
{
    CreateSong(Song, (char *)Filename);
}

internal uint32_t
GetTotalSamplesPlayed(ns_song *Song)
{
    XAUDIO2_VOICE_STATE VoiceState;
    Song->SourceVoice->GetState(&VoiceState, 0);
    uint32_t Result = (uint32_t)VoiceState.SamplesPlayed;
    return Result;
}

internal void
Seek(ns_song *Song, float Time)
{
    Assert(Time <= Song->Duration);
    CheckSOK(Song->SourceVoice->Stop());
    CheckSOK(Song->SourceVoice->FlushSourceBuffers());
    if (Time < Song->Duration)
    {
        Song->LastTotalSamplesPlayed = GetTotalSamplesPlayed(Song);
        uint32_t SampleStart = (uint32_t)(Time*(float)Song->SampleRate);
        Song->XAudio2Buffer.PlayBegin = SampleStart;
        CheckSOK(Song->SourceVoice->SubmitSourceBuffer(&Song->XAudio2Buffer));
        if (Song->IsPlaying)
        {
            CheckSOK(Song->SourceVoice->Start());
        }
    }
}

internal uint32_t
GetCurrentSampleIndex(ns_song *Song)
{
    uint32_t Result;
    if (HasEnded(Song))
    {
        Result = 0;
    }
    else
    {
        uint32_t TotalSamplesPlayed = GetTotalSamplesPlayed(Song);
        uint32_t SamplesPlayedSinceLastSeek = TotalSamplesPlayed - Song->LastTotalSamplesPlayed;
        Result = Song->XAudio2Buffer.PlayBegin + SamplesPlayedSinceLastSeek;
    }
    return Result;
}

internal float
GetTime(ns_song *Song)
{
    uint32_t CurrentSampleIndex = GetCurrentSampleIndex(Song);
    float Result = (float)CurrentSampleIndex/(float)Song->SampleRate;
    return Result;
}

internal void
SetVolume(ns_song *Song, float Volume)
{
    CheckSOK(Song->SourceVoice->SetVolume(Volume));
}

internal void
FastFourierTransform(ns_song *Song, Complex *FourierData, uint32_t StartSampleIdx, uint32_t NumSamples)
{
    if (Song->NumChannels == 2)
    {
        Song->SamplesAverageBuffer = PushPerFrameMemory(Song->BytesPerSample*NumSamples);
    }

    switch (Song->BitsPerSample)
    {
        case 8:
        {
            uint8_t *SongSamples = (uint8_t *)Song->Samples;
            uint8_t *SamplesAverageBuffer = (uint8_t *)Song->SamplesAverageBuffer;
            uint8_t *RangeSamples = NULL;
            switch (Song->NumChannels)
            {
                case 1:
                {
                    RangeSamples = &SongSamples[StartSampleIdx];
                } break;

                case 2:
                {
                    uint8_t *SampleBuffer = (uint8_t *)Song->SamplesAverageBuffer;
                    for (uint32_t I = 0; I < NumSamples; I++)
                    {
                        uint8_t LeftSample = SongSamples[StartSampleIdx + (2*I)];
                        uint8_t RightSample = SongSamples[StartSampleIdx + (2*I + 1)];
                        uint8_t Average = (LeftSample + RightSample)/2;
                        SamplesAverageBuffer[I] = Average;
                    }
                    RangeSamples = SamplesAverageBuffer;
                } break;
            }
            FastFourierTransform(FourierData, RangeSamples, NumSamples);
        } break;

        case 16:
        {
            uint16_t *SongSamples = (uint16_t *)Song->Samples;
            uint16_t *SamplesAverageBuffer = (uint16_t *)Song->SamplesAverageBuffer;
            uint16_t *RangeSamples = NULL;
            switch (Song->NumChannels)
            {
                case 1:
                {
                    RangeSamples = &SongSamples[StartSampleIdx];
                } break;

                case 2:
                {
                    uint16_t *SampleBuffer = (uint16_t *)Song->SamplesAverageBuffer;
                    for (uint32_t I = 0; I < NumSamples; I++)
                    {
                        uint16_t LeftSample = SongSamples[StartSampleIdx + (2*I)];
                        uint16_t RightSample = SongSamples[StartSampleIdx + (2*I + 1)];
                        uint16_t Average = (LeftSample + RightSample)/2;
                        SamplesAverageBuffer[I] = Average;
                    }
                    RangeSamples = SamplesAverageBuffer;
                } break;
            }
            FastFourierTransform(FourierData, RangeSamples, NumSamples);
        } break;
    }

    if (Song->NumChannels == 2)
    {
        PopPerFrameMemory();
    }
}

#endif