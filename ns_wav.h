#ifndef NS_WAV_H
#define NS_WAV_H

#include "ns_common.h"
#include "ns_file.h"

#pragma pack(push, 1)

struct wav_chunk_header
{
    uint32_t ID; /* BE */
    uint32_t Size;
};

/* RIFF header. */

struct riff_chunk
{
    wav_chunk_header Header;
    uint32_t Format; /* BE */
};

/* WAV fmt chunk. */

struct wav_fmt_chunk_data
{
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
};

struct wav_fmt_chunk
{
    wav_chunk_header Header;
    union
    {
        wav_fmt_chunk_data Data;
        struct
        {
            uint16_t AudioFormat;
            uint16_t NumChannels;
            uint32_t SampleRate;
            uint32_t ByteRate;
            uint16_t BlockAlign;
            uint16_t BitsPerSample;
        };
    };
};

/* WAV data chunk. */

struct wav_data_chunk
{
    wav_chunk_header Header;
    uint8_t FirstSample;
};
#pragma pack(pop)

struct ns_wav
{
    ns_file File;
    riff_chunk *RiffChunk;
    wav_fmt_chunk *WavFmtChunk;
    wav_data_chunk *WavDataChunk;
};

internal wav_chunk_header *
GetNextChunk(wav_chunk_header *Header)
{
    uint8_t *OnePastHeader = (uint8_t *)(Header + 1);
    wav_chunk_header *Result = (wav_chunk_header *)(OnePastHeader + Header->Size);
    return Result;
}

internal ns_wav
LoadWav(const char *Name)
{
    riff_chunk *RiffChunk;
    wav_fmt_chunk *WavFmtChunk;
    wav_data_chunk *WavDataChunk;
    ns_file File = LoadFile(Name);
    uint8_t *FileContents = File.Contents;
    {
        /* We don't reverse endianness cause it's easier to copy to XAudio2. */

        RiffChunk = (riff_chunk *)FileContents;
        Assert(RiffChunk->Header.ID == 0x46464952); /* RIFF */
        Assert(RiffChunk->Format == 0x45564157); /* WAVE */

        WavFmtChunk = (wav_fmt_chunk *)(RiffChunk + 1);
        Assert(WavFmtChunk->Header.ID == 0x20746d66); /* fmt */
        Assert(WavFmtChunk->Header.Size == 16 || WavFmtChunk->Header.Size == 18);
        Assert(WavFmtChunk->AudioFormat == 1);

        WavDataChunk = (wav_data_chunk *)GetNextChunk(&WavFmtChunk->Header);
        while (WavDataChunk->Header.ID != 0x61746164)
        {
            WavDataChunk = (wav_data_chunk *)GetNextChunk(&WavDataChunk->Header);
        }
    }
    /* Limitation verifications. */
    Assert(WavFmtChunk->NumChannels == 1 || WavFmtChunk->NumChannels == 2);
    Assert(WavFmtChunk->BitsPerSample == 16);

    ns_wav Result = {};
    Result.RiffChunk = RiffChunk;
    Result.WavFmtChunk = WavFmtChunk;
    Result.WavDataChunk = WavDataChunk;
    Result.File = File;
    return Result;
}

internal void
Free(ns_wav Wav)
{
    Free(&Wav.File);
}

#endif