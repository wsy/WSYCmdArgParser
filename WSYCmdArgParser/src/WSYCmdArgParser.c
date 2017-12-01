//
//  WSYCmdArgParser.c
//  WSYCmdArgParser
//
//  Created by Shiyao Wang on 2017/11/9.
//  Copyright © 2017 Shiyao Wang. All rights reserved.
//

#include "WSYCmdArgParser.h"

#ifndef __TIMESTAMP__
#define __TIMESTAMP__ "__TIMESTAMP__ marco is not available on this compiler!"
#endif
const char WSY_CMD_ARG_PARSER_VERSION[64] = "0.9.0 Beta";
const char WSY_CMD_ARG_PARSER_BUILD_TIMESTAMP[64] = __TIMESTAMP__;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Method definition
 */
int Initialize(PCmdArgCollection argCollection, int argc);
int CheckArgDefinition(PCmdArgCollection argCollection);
void ParseUnixStyleArg(int argc, const char * argv[], PCmdArgCollection argCollection);
void ParseUnixStyleArgWithFullName(PCmdArgCollection *argCollection, int argc, const char **argv, int *i);
void ParseUnixStyleArgWithArgAlias(PCmdArgCollection *argCollection, int argc, const char **argv, int *i);
void ParseUnixStyleArgWithUnnamedArg(PCmdArgCollection argCollection, int argc, const char **argv, int *i);
void ParseKeyValueStyleArg(int argc, const char * argv[], PCmdArgCollection argCollection);
void ParseDOSStyleArg(int argc, const char * argv[], PCmdArgCollection argCollection);
PCmdArg FindArgByName(const char *name, PCmdArgCollection argCollection);
PCmdArg FindArgByAlias(const char alias, PCmdArgCollection argCollection);
int ParseArgValue(const char *argv, PCmdArg arg);

/*
 * Global Variable definition
 */
PCmdArg ArgAliasMap[128];
void (*ArgParser)(int, const char *[], PCmdArgCollection);

/*
 * Method implementations
 */
int ParseArgs(int argc, const char *argv[], PCmdArgCollection argCollection)
{
    if(Initialize(argCollection, argc) != 0)
    {
        // Arg Initialize Failed!
        return -1;
    }
    ArgParser(argc, argv, argCollection);
    return 0;
}

int Initialize(PCmdArgCollection argCollection, int argc)
{
    if(CheckArgDefinition(argCollection)!=0)
    {
        // Invalid arg definition
        return -1;
    }
    if(argCollection->ParseUnnamedArgs)
    {
        argCollection->UnnamedArgCount = 0;
        argCollection->UnnamedArgs = calloc(argc, sizeof(char*));
        if(argCollection->UnnamedArgs == NULL)
        {
            // Insufficient memory;
            return -1;
        }
    }
    if(argCollection->ParseUndefinedArgs)
    {
        argCollection->UndefinedArgCount = 0;
        argCollection->UndefinedArgs = calloc(argc, sizeof(CmdArg));
        if(argCollection->UnnamedArgs == NULL)
        {
            // Insufficient memory;
            return -1;
        }
    }
    switch (argCollection->ArgStyle)
    {
        case ARG_STYLE_UNIX:
            ArgParser = ParseUnixStyleArg;
            break;
        case ARG_STYLE_KEY_VALUE:
            ArgParser = ParseKeyValueStyleArg;
            break;
        case ARG_STYLE_DOS:
            ArgParser = ParseDOSStyleArg;
            break;
        default:
            // Invalid ArgStyle
            return -1;
    }
    return 0;
}

int CheckArgDefinition(PCmdArgCollection argCollection)
{
    int i = 0;
    memset(ArgAliasMap, 128, sizeof(PCmdArg));
    for (i = 0; i < argCollection->ArgDefinitionCount; i++)
    {
        PCmdArg currentArg = &(argCollection->ArgDefinition[i]);
        if(currentArg->ArgPresent != NULL)
        {
            (*(currentArg->ArgPresent)) = false;
        }
        if(currentArg->HasValue
           && currentArg->Value == NULL
           && currentArg->LongValue == NULL
           && currentArg->DoubleValue == NULL
           && currentArg->IntegerValue == NULL
           && currentArg->BooleanValue == NULL)
        {
            // Invalid ArgDefinition;
            return -1;
        }
        if(currentArg->Aliase > 0)
        {
            ArgAliasMap[currentArg->Aliase] = currentArg;
        }
    }
    return 0;
}
/*
 * This function handles Unix style args.
 * Args usually starts with "-" or "--".
 * "--" for args with full name and "-" for args with shor name(alias).
 */
void ParseUnixStyleArg(int argc, const char * argv[], PCmdArgCollection argCollection)
{
    int i = 0;
    for(i = 1; i < argc; i++)
    {
        // This is an arg
        if(argv[i][0] == '-')
        {
            if(argv[i][1] == '-')
            {
                ParseUnixStyleArgWithFullName(&argCollection, argc, argv, &i);
                continue;
            }
            else
            {
                ParseUnixStyleArgWithArgAlias(&argCollection, argc, argv, &i);
                continue;
            }
        }
        else
        {
            ParseUnixStyleArgWithUnnamedArg(argCollection, argc, argv, &i);
        }
    }
}

/*
 * This is an arg with full name.
 * e.g. du --max-depth 0
 * e.g. something --help
 */
void ParseUnixStyleArgWithFullName(PCmdArgCollection *argCollection, int argc, const char **argv, int *i)
{
    PCmdArg arg = FindArgByName(&argv[*i][2], *argCollection);
    // Undefined(unexpected) arg.
    if(arg == NULL)
    {
        if ((*argCollection)->ParseUndefinedArgs)
        {
            arg = &((*argCollection) -> UndefinedArgs[(*argCollection) -> UndefinedArgCount]);
            strcpy(arg->Name, &(argv[*i][2]));
            arg->Aliase = '\0';
            (*argCollection) -> UndefinedArgCount++;
        }
        else
        {
            return;
        }
    }
    if(arg->ArgPresent != NULL)
    {
        *(arg->ArgPresent) = true;
    }
    if(arg->HasValue && *i < argc && argv[*i+1][0] != '-')
    {
        (*i)++;
        ParseArgValue(argv[*i], arg);
    }
}

/*
 * This is an arg with alias.
 * e.g. ls -h
 */
void ParseUnixStyleArgWithArgAlias(PCmdArgCollection *argCollection, int argc, const char **argv, int *i)
{
    PCmdArg arg = FindArgByAlias(argv[*i][1], *argCollection);
    // Undefined(unexpected) arg.
    if(arg == NULL)
    {
        if((*argCollection)->ParseUndefinedArgs)
        {
            arg = &((*argCollection) -> UndefinedArgs[(*argCollection) -> UndefinedArgCount]);
            memset(arg->Name, 32, sizeof(char));
            arg->Aliase = argv[*i][1];
            (*argCollection) -> UndefinedArgCount++;
        }
        else
        {
            return;
        }
    }
    if(arg->ArgPresent != NULL)
    {
        *(arg->ArgPresent) = true;
    }
    // This is an arg alias with combined value
    // e.g. gcc -O2
    if(strlen(argv[*i]) > 2)
    {
        ParseArgValue(&(argv[*i][2]), arg);
    }
    // This is an arg alias with separated value
    // e.g. du -d 1
    else if (arg->HasValue && *i < argc && argv[*i+1][0] != '-')
    {
        (*i)++;
        ParseArgValue(argv[*i], arg);
    }
}

/*
 * This is an unnamed arg
 * e.g. gcc WSYCmdArgParser.c
 */
void ParseUnixStyleArgWithUnnamedArg(PCmdArgCollection argCollection, int argc, const char **argv, int *i)
{
    if (argCollection->ParseUnnamedArgs)
    {
        argCollection->UnnamedArgs[argCollection->UnnamedArgCount] = calloc(strlen(argv[*i]), sizeof(char));
        strcpy(argCollection->UnnamedArgs[argCollection->UnnamedArgCount], argv[*i]);
        argCollection->UnnamedArgCount++;
    }
}

/*
 * This function handles key=value style args.
 * e.g. dd if=/dev/disk0 of=mbr.bin ibs=512 obs=512 count=1 conv=notrunc
 */
void ParseKeyValueStyleArg(int argc, const char * argv[], PCmdArgCollection argCollection)
{
    printf("TODO: Not Implemented!\n");
}

/*
 * This function handles dos style args.
 * Args usually starts with "/"
 * e.g. dir /p /w
 * e.g.
 */
void ParseDOSStyleArg(int argc, const char * argv[], PCmdArgCollection argCollection)
{
    printf("TODO: Not Implemented!\n");
}

PCmdArg FindArgByName(const char *name, PCmdArgCollection argCollection)
{
    int i = 0;
    for(i = 0; i < argCollection->ArgDefinitionCount; i++)
    {
        if(strcmp(argCollection->ArgDefinition[i].Name, name) == 0)
        {
            return &(argCollection->ArgDefinition[i]);
        }
    }
    return NULL;
}

PCmdArg FindArgByAlias(const char alias, PCmdArgCollection argCollection)
{
    if(alias < 1)
    {
        return NULL;
    }
    return ArgAliasMap[alias];
}

int ParseArgValue(const char *argv, PCmdArg arg)
{
    int result = 0;
    if(arg->Value != NULL)
    {
        strcpy(*(arg->Value),argv);
        result++;
    }
    if(arg->LongValue != NULL)
    {
        sscanf(argv, "%lld", arg->LongValue);
        result++;
    }
    if(arg->IntegerValue != NULL)
    {
        sscanf(argv, "%d", arg->IntegerValue);
        result++;
    }
    if(arg->DoubleValue != NULL)
    {
        sscanf(argv, "%lf", arg->DoubleValue);
        result++;
    }
    if(arg->BooleanValue != NULL)
    {
        int temp = 0;
        sscanf(argv, "%d", &temp);
        *(arg->BooleanValue) = temp;
        result++;
    }
    return result;
}

void TeardownCmdArgCollection(PCmdArgCollection argCollection)
{
    int i = 0;
    if(argCollection->ParseUnnamedArgs)
    {
        for(i = 0; i < argCollection->UnnamedArgCount; i++)
        {
            free(argCollection->UnnamedArgs[i]);
        }
        free(argCollection->UnnamedArgs);
    }
    if(argCollection->ParseUndefinedArgs)
    {
        free(argCollection->UndefinedArgs);
    }
}
