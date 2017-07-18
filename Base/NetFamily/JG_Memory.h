#ifndef _JG_MEMORY_H_
#define _JG_MEMORY_H_

#include <Unknwn.h>
#include "JBaseDef.h"

struct IJG_Buffer : IUnknown
{
public:
    virtual void    *GetData() = 0;
    virtual unsigned GetSize() = 0;
}; 

IJG_Buffer *JG_MemoryCreateBuffer(unsigned uSize);

#define JG_COM_RELEASE(pInterface)      \
    do  \
    {   \
        if (pInterface)                 \
        {                               \
            (pInterface)->Release();    \
            (pInterface) = NULL;        \
        }                               \
    } while (false)

// JG_COM_PROCESS_ERROR_RETURN_ERROR
#define JG_COM_PROC_ERR_RET_ERR(Condition)  \
    do  \
    {   \
        if (FAILED(Condition))      \
        {                           \
            hrResult = Condition;   \
            goto Exit0;             \
        }   \
    } while (false)

#endif // _JG_MEMORY_H_