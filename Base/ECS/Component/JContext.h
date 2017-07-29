#ifndef _JCONTEXT_H_
#define _JCONTEXT_H_

#include "JBaseDef.h"
#include "JComponentDef.h"

#define JCOMPONENT_BINARY_SIZE ((ect_end - 1) / 8 + 1)

class JContext
{
public:
    JContext();

    void Clear();
    void StartTraverse();
    JCOMPONENT_TYPE GetNext();

    BOOL operator == (const JContext& Context) const;

    JContext operator & (const JContext& Context) const;

    JContext operator | (const JContext& Context) const;

private:
    BYTE szText[JCOMPONENT_BINARY_SIZE];
    DWORD m_dwFind;
};

#endif // _JCONTEXT_H_