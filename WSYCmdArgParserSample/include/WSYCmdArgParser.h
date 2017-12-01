//
//  WSYCmdArgParser.h
//  WSYCmdArgParser
//
//  Created by Shiyao Wang on 2017/11/9.
//  Copyright © 2017 Shiyao Wang. All rights reserved.
//

#ifndef WSYCmdArgParser_h
#define WSYCmdArgParser_h

#include <stdbool.h>

extern const char WSY_CMD_ARG_PARSER_VERSION[64];
extern const char WSY_CMD_ARG_PARSER_BUILD_TIMESTAMP[64];

/* Implemented Since V0.9*/
#define ARG_STYLE_UNIX 1
/* Not yet implemented */
#define ARG_STYLE_KEY_VALUE 2
/* Not yet implemented */
#define ARG_STYLE_DOS 3

typedef struct
{
    char Name[32];
    char Aliase;
    bool Optional;
    bool HasValue;
    bool *ArgPresent;
    char **Value;
    int *IntegerValue;
    long long *LongValue;
    double *DoubleValue;
    bool *BooleanValue;
} CmdArg, *PCmdArg;

typedef struct
{
    int ArgStyle;
    int ArgDefinitionCount;
    PCmdArg ArgDefinition;
    bool ParseUnnamedArgs;
    int UnnamedArgCount;
    char **UnnamedArgs;
    bool ParseUndefinedArgs;
    int UndefinedArgCount;
    PCmdArg UndefinedArgs;
} CmdArgCollection, *PCmdArgCollection;

int ParseArgs(int argc, const char *argv[], PCmdArgCollection argCollection);
void TeardownCmdArgCollection(PCmdArgCollection argCollection);

#endif /* WSYCmdArgParser_h */
