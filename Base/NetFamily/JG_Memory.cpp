#include "JG_Memory.h"

IJG_Buffer *JG_MemoryCreateBuffer(unsigned uSize)
{
    IJG_Buffer *pvBuffer = NULL;

    ASSERT(uSize >= 0);

    pvBuffer = new IJG_Buffer(uSize);

//Exit0:
    return pvBuffer;
}

void JG_COM_RELEASE(void* pBuffer)
{
    if (pBuffer)
    {
        delete pBuffer;
        pBuffer = NULL;
    }
}