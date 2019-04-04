#ifndef NS_LRC
#define NS_LRC

#include "ns_common.h"
#include "ns_string.h"
#include "ns_file.h"

struct lrc_file
{
    char Words[1024][32];
    int NumWords;
    float Times[1024];
};

lrc_file LoadLrcFile(const char *Name)
{
    lrc_file Result;
    Result.NumWords = 0;

    FILE *File = fopen(Name, "rb");
    Assert(File != NULL);

    while (1)
    {
        /* Get time. */
        char TimeBuffer[64];
        int ScanfResult = fscanf(File, "%s", TimeBuffer);

        /* Are we at the end of the file? */
        if (ScanfResult == EOF)
        {
            break;
        }

        int Minute = StringToInt((char *)&TimeBuffer[1], 2);
        int Second = StringToInt((char *)&TimeBuffer[4], 2);
        int Millisecond = StringToInt((char *)&TimeBuffer[7], 3);
        float MinuteInSeconds = (float)Minute*60.0f;
        float MillisecondInSeconds = (float)Millisecond/1000.0f;
        float Time = MinuteInSeconds + (float)Second + MillisecondInSeconds;
        Result.Times[Result.NumWords] = Time;

        /* Get word. */
        ScanfResult = fscanf(File, "%s", Result.Words[Result.NumWords]);
        Assert(ScanfResult != EOF);

        Result.NumWords++;
    }
    fclose(File);

    return Result;
}

#endif