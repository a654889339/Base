#ifndef _JG_MEMORY_H_
#define _JG_MEMORY_H_

#include "JBaseDef.h"

typedef  BYTE IJG_Buffer;

IJG_Buffer *JG_MemoryCreateBuffer(unsigned uSize);
void JG_COM_RELEASE(void* pBuffer);

#endif // _JG_MEMORY_H_