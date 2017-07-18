#ifndef _JBASE_DEF_H_
#define _JBASE_DEF_H_

#include <minwindef.h>
#include <stdio.h>
#ifdef __GNUC__
#include <stdint.h>
#endif

#ifndef ASSERT
#include <assert.h>
#define ASSERT(x) assert(x)
#endif

#define JGThread_Sleep(uSeconds) Sleep(uSeconds);

#ifdef WIN32
#define JG_FUNCTION __FUNCTION__
#else
#define JG_FUNCTION __PRETTY_FUNCTION__
#endif

typedef enum JGLogPriority
{
    JGLOG_ERR,
    JGLOG_WARNING,
    JGLOG_INFO,
    JGLOG_DEBUG,
    JGLOG_PRIORITY_MAX
} JGLOG_PRIORITY;

#define JGLogPrintf(Priority, Format, ...) \
    printf(Format, __VA_ARGS__);

#define JG_PROCESS_ERROR(Condition) \
    do   \
    {    \
        if (!(Condition))      \
        {                      \
            goto Exit0;        \
        }                      \
    } while(false);

#define JGLOG_PROCESS_ERROR(Condition) \
    do  \
    {   \
        if (!(Condition))         \
        {                         \
            JGLogPrintf(          \
                JGLOG_DEBUG,      \
                "JGLOG_PROCESS_ERROR(%s) at line %d in %s\n", #Condition, __LINE__, JG_FUNCTION  \
            );                    \
            goto Exit0;           \
        }                         \
    } while(false);

#define JG_PROCESS_SUCCESS(Condition) \
    do   \
    {    \
        if (Condition)      \
            goto Exit1;     \
    } while(false);

#endif // _JBASE_DEF_H_