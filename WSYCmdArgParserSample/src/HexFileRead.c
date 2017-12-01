//
//  main.c
//  RawDiskIO
//
//  Created by Shiyao Wang on 12-12-20.
//  Copyright (c) 2012 Shiyao Wang. All rights reserved.
//
#include "WSYCmdArgParser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __TIMESTAMP__
#define __TIMESTAMP__ "This compiler does not support __TIMESTAMP__ marco!"
#endif

const char VERSION[] = "1.0.0 Sample";
const char BUILD_TIMESTAMP[] = __TIMESTAMP__;
const char AUTHOR[] = "Shiyao Wang";
const char PROJECT_URL[] = "https://github.com/wsy";

int ProcessingArgs(int argc, const char* argv[]);
void HandleOffset(void);
int ReadLine(void);
void PrintLine(unsigned long long offset, int BytesRead);
void PrintOffSet(unsigned long long offset);
void PrintHexValues(int BytesRead);
void PrintCharValues(int BytesRead);
void PrintUsage(const char *arg0);
void PrintVersion(void);

char Hex[256][3] = {
    "00","01","02","03","04","05","06","07","08","09","0A","0B","0C","0D","0E","0F",
    "10","11","12","13","14","15","16","17","18","19","1A","1B","1C","1D","1E","1F",
    "20","21","22","23","24","25","26","27","28","29","2A","2B","2C","2D","2E","2F",
    "30","31","32","33","34","35","36","37","38","39","3A","3B","3C","3D","3E","3F",
    "40","41","42","43","44","45","46","47","48","49","4A","4B","4C","4D","4E","4F",
    "50","51","52","53","54","55","56","57","58","59","5A","5B","5C","5D","5E","5F",
    "60","61","62","63","64","65","66","67","68","69","6A","6B","6C","6D","6E","6F",
    "70","71","72","73","74","75","76","77","78","79","7A","7B","7C","7D","7E","7F",
    "80","81","82","83","84","85","86","87","88","89","8A","8B","8C","8D","8E","8F",
    "90","91","92","93","94","95","96","97","98","99","9A","9B","9C","9D","9E","9F",
    "A0","A1","A2","A3","A4","A5","A6","A7","A8","A9","AA","AB","AC","AD","AE","AF",
    "B0","B1","B2","B3","B4","B5","B6","B7","B8","B9","BA","BB","BC","BD","BE","BF",
    "C0","C1","C2","C3","C4","C5","C6","C7","C8","C9","CA","CB","CC","CD","CE","CF",
    "D0","D1","D2","D3","D4","D5","D6","D7","D8","D9","DA","DB","DC","DD","DE","DF",
    "E0","E1","E2","E3","E4","E5","E6","E7","E8","E9","EA","EB","EC","ED","EE","EF",
    "F0","F1","F2","F3","F4","F5","F6","F7","F8","F9","FA","FB","FC","FD","FE","FF"
};

/*
 * First, you may put your own control variables anywhere.
 * You may also set default values for them.
 */
struct
{
    int BytesPerLine;
    unsigned long long StartOffset;
    unsigned long long MaxByteToRead;
    bool HelpSelected;
    bool VersionSelected;
} GlobalOptions = { 16, 0, 1024, false, false };

/*
 * Second, you define arg definitions for all expected args.
 */
CmdArg ArgDefinitions[5] =
{
    /* Use WSYCmdArgParser.h as a reference when you define your arg definitions */
    /* For unnecessary options, you may initial them with NULL or '\0' if you don't want an alias for this arg */
    {"start-offset", 's', true, true, NULL, NULL, NULL, (long long *)&GlobalOptions.StartOffset},
    {"length-to-read", 'l', true, true, NULL, NULL, NULL, (long long *)&GlobalOptions.MaxByteToRead},
    {"bytes-per-line", 'w', true, true, NULL, NULL, &GlobalOptions.BytesPerLine},
    {"help", 'h', true, false, &GlobalOptions.HelpSelected},
    {"version", 'v', true, false, &GlobalOptions.VersionSelected}
};

/*
 * Then, define an argCollection with your argDefinitions.
 * Also, you can tell whether parser shall parse unnamed args or unexpected args.
 * Note: For unnamed args and unexpected args, value fields are allocated by parser.
 *       So, you shall initialize those two fields with NULL.
 *       But, REMEMBER TO CALL TEARDOWN FUNCTION if you want unnamed args and unexpected args to be parsed.
 *       Otherwise, a memory leak can occur.
 */
CmdArgCollection argDefinitionCollection = {ARG_STYLE_UNIX, 5, ArgDefinitions, true, };

char temp[256] = { 0 };
FILE *file = NULL;

int main(int argc, const char * argv[])
{
    int BytesRead = 0;
    unsigned long long offset = 0;
    ProcessingArgs(argc, argv);
    offset = GlobalOptions.StartOffset;
    BytesRead = GlobalOptions.BytesPerLine;

    if (!file)
    {
        printf("Open file failed!\n");
        return 1;
    }
    printf("Open succeed!\n");
    HandleOffset();
    while ((offset < (GlobalOptions.MaxByteToRead + GlobalOptions.StartOffset)) && (BytesRead == GlobalOptions.BytesPerLine))
    {
        BytesRead = ReadLine();
        PrintLine(offset, BytesRead);
        offset += GlobalOptions.BytesPerLine;
    }
    return 0;
}

int ProcessingArgs(int argc, const char* argv[])
{
    /*
     * Finally, you invoke ParseArgs function to get your args parsed.
     * Since we're passing pointers of your(my) GlobalOptions, if a certain arg is set,
     * our variables in GlobalOptions will get modified automatically.
     */
    ParseArgs(argc, argv, &argDefinitionCollection);
    /*
     * Unlike what you usually do with while statement and switch statement if you're using getopt fucntion,
     * normally, you don't need to handle these parsing result again.
     * Also, I'm considering to include function pointer inside arg definition,
     * so that you don't need the if statement below.
     */
    if(GlobalOptions.VersionSelected)
    {
        PrintVersion();
    }
    /*
     * Currently, there's no support for arg-value validation.
     * So you still have to validate arg values by yourself for the moment.
     */
    if(GlobalOptions.HelpSelected || argDefinitionCollection.UnnamedArgCount != 1)
    {
        PrintUsage(argv[0]);
    }
    file = fopen(argDefinitionCollection.UnnamedArgs[0], "rb");
    /*
     * Since we used the ParseUnnamedArgs function,
     * we need to INVOKE TeardownCmdArgCollection FUNCTION TO PREVENT FROM MEMORY LEAK.
     */
    TeardownCmdArgCollection(&argDefinitionCollection);
    return 0;
}

/*
 * Finally, do whatever you're going to do with your code.
 * That's all for this sample.
 * I hope you will like this CmdArgParser library.
 */
void HandleOffset()
{
    if (GlobalOptions.StartOffset > 0)
    {
		unsigned long long remainingOffset = GlobalOptions.StartOffset;
		fseek(file, 0, 0);
		while (remainingOffset > 1073741823L)
		{
			int seek = fseek(file, 1073741824L, 1);
			if (seek)
			{
				printf("Skipping %lld bytes failed!\n", GlobalOptions.StartOffset);
				exit(2);
			}
			remainingOffset -= 1073741824L;
		}
        int seek = fseek(file, remainingOffset, 1);
        if (seek)
        {
            printf("Skipping %lld bytes failed!\n", GlobalOptions.StartOffset);
            exit(2);
        }
        printf("%lld bytes skipped!\n", GlobalOptions.StartOffset);
    }
}

int ReadLine()
{
    return (int)fread(temp, sizeof(char), GlobalOptions.BytesPerLine, file);
}

void PrintLine(unsigned long long offset, int BytesRead)
{
    PrintOffSet(offset);
    printf("\t");
    PrintHexValues(BytesRead);
    printf("\t");
    PrintCharValues(BytesRead);
    printf("\n");
}

void PrintOffSet(unsigned long long offset)
{
    printf("%lld", offset);
    if (offset < 1000000000000000 && GlobalOptions.StartOffset + GlobalOptions.MaxByteToRead > 10000000000000000)
    {
        printf("        ");
    }
    if (offset < 10000000 && GlobalOptions.StartOffset + GlobalOptions.MaxByteToRead > 100000000)
    {
        printf("        ");
    }
}

void PrintHexValues(int BytesRead)
{
    int i = 0;
    for (i = 0; i < GlobalOptions.BytesPerLine; i++)
    {
        if (i < BytesRead)
        {
            printf("%s ", Hex[((int)temp[i]) & 0x000000FF]);
        }
        else
        {
            printf("   ");
        }
    }
}

void PrintCharValues(int BytesRead)
{
    int i = 0;
    for (i = 0; i < GlobalOptions.BytesPerLine; i++)
    {
        if (i < BytesRead)
        {
            if (((unsigned int)temp[i]) < 32 || ((unsigned int)temp[i]) > 127)
            {
                printf("%c", '.');
            }
            else
            {
                printf("%c", temp[i]);
            }
        }
        else
        {
            printf(" EOF\n");
            printf("Bytes Read In Last Line: %d\n", BytesRead);
            return;
        }
    }
}

void PrintUsage(const char *arg0)
{
    printf("Usage:\n");
    printf("\t%s fileName [options]\n", arg0);
    printf("\t%s [options] fileName\n", arg0);
    printf("Options:\n");
    printf("\t-l<n> --length-to-read n\n\t\tSet how many bytes to read before exiting this program. Default 1024.\n");
    printf("\t-s<n> --start-offset n\n\t\tSet the offset from where to read the file from. Default 0.\n");
    printf("\t-w<n> --bytes-per-line n\n\t\tSet the line width when printing the result. Default 16.\n");
    printf("\t-h    --help\n\t\tPrint this help message.\n");
    printf("\t-v    --version\n\t\tPrint the version of this program.\n");
    exit(0);
}

void PrintVersion()
{
    printf("Version: %s\n", VERSION);
    printf("CompileTime: %s\n", BUILD_TIMESTAMP);
    printf("Author: %s\n", AUTHOR);
    printf("If you find any bug, please fire an issue on github.\n");
    printf("\t%s\n", PROJECT_URL);
    exit(0);
}
