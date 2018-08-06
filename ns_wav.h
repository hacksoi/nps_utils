#ifndef NS_WAV
#define NS_WAV

#include "ns_common.h"

#pragma pack(push, 1)
struct riff_header
{
    uint32_t ChunkID; /* BE */
    uint32_t ChunkSize;
    uint32_t Format; /* BE */
};

struct wav_fmt
{
    uint32_t Subchunk1ID; /* BE */
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
};

struct wav_data
{
    uint32_t Subchunk2ID; /* BE */
    uint32_t Subchunk2Size;
    uint8_t *Data;
};
#pragma pack(pop)

struct decode_wav_result
{
    uint8_t *FileContents;
    uint16_t *Samples;
    int NumSamples;
};

internal decode_wav_result
DecodeWav(const char *Filename)
{
    decode_wav_result Result = { };

    int FileSize;
    riff_header *RiffHeader;
    wav_fmt *WavFmt;
    wav_data *WavData;
    uint8_t *FileContents;
    {
        FILE *File = fopen(Filename, "rb");
        CheckRR(File != NULL);

        FileSize = 0;
        {
            CheckRR(!fseek(File, 0, SEEK_END));
            FileSize = ftell(File);
            CheckRR(FileSize != -1);
            CheckRR(!fseek(File, 0, SEEK_SET));
        }

        FileContents = (uint8_t *)malloc(FileSize);
        size_t BytesRead = fread(FileContents, 1, FileSize, File);
        CheckRR(BytesRead == (size_t)FileSize);

        CheckRR(!fclose(File));

        RiffHeader = (riff_header *)FileContents;
        WavFmt = (wav_fmt *)(RiffHeader + 1);
        WavData = (wav_data *)(WavFmt + 1);
        WavData->Data = (uint8_t *)&WavData->Data;

        ReverseEndianness(&RiffHeader->ChunkID);
        ReverseEndianness(&RiffHeader->Format);
        ReverseEndianness(&WavFmt->Subchunk1ID);
        ReverseEndianness(&WavData->Subchunk2ID);
    }

    /* Verify headers. */
    {
        Assert(RiffHeader->ChunkID == 0x52494646); /* 0x52494646 == "RIFF" */
        Assert(RiffHeader->ChunkSize == (FileSize - (sizeof(RiffHeader->ChunkID) + sizeof(RiffHeader->ChunkSize))));
        Assert(RiffHeader->Format == 0x57415645); /* 0x57415645 == "WAVE" */

        Assert(WavFmt->Subchunk1ID == 0x666d7420);
        Assert(WavFmt->Subchunk1Size == 16);
        Assert(WavFmt->AudioFormat == 1);

        Assert(WavData->Subchunk2ID == 0x64617461);
    }

    /* Limitation verifications. */
    Assert(WavFmt->BitsPerSample == 16)

    int BytesPerSample = WavFmt->BitsPerSample/8;
    Result.FileContents = FileContents;
    Result.Samples = (uint16_t *)WavData->Data;
    Result.NumSamples = WavData->Subchunk2Size/BytesPerSample;

    return Result;
}

#endif