#ifndef _JBASE_DEF_H_
#define _JBASE_DEF_H_

#include <minwindef.h>

#define JG_PROCESS_ERROR(Condition) do{ if (!Condition) { goto Exit0; } }while(false);

#endif // _JBASE_DEF_H_