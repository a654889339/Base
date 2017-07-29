#ifndef _JICOMPONENT_H_
#define _JICOMPONENT_H_

#include "JG_Memory.h"
#include "JComponentDef.h"

struct JIComponent
{
    JCOMPONENT_TYPE m_eType;
};

static JIComponent* GetComponentFromBuffer(IJG_Buffer* piBuffer)
{
    JIComponent* pResult    = NULL;
    JIComponent* pComponent = NULL;

    JG_PROCESS_ERROR(piBuffer);

    JG_PROCESS_ERROR(piBuffer->GetSize() >= sizeof(JIComponent));

    pComponent = (JIComponent*)piBuffer->GetData();

    JG_PROCESS_ERROR(pComponent->m_eType >= ect_begin && pComponent->m_eType < ect_end);

    pResult = pComponent;

Exit0:
    return pResult;
}

#endif //_JICOMPONENT_H_