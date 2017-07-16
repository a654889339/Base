#include <stdlib.h>
#include "JG_Memory.h"

IJG_Buffer *JG_MemoryCreateBuffer(unsigned uSize)
{
    IJG_Buffer *pvBuffer = NULL;

    ASSERT(uSize >= 0);

    pvBuffer = (IJG_Buffer*)malloc(uSize);

//Exit0:
    return pvBuffer;
}

void JG_COM_RELEASE(IJG_Buffer &Buffer)
{
    if (Buffer)
    {
        delete &Buffer;
        Buffer = NULL;
    }
}