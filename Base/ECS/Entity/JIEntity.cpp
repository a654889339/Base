#include "JIEntity.h"

JIEntity::JIEntity()
{
    m_dwID = 0;
    m_Context.Clear();
}

JIEntity::~JIEntity()
{
    UnInit();
}

BOOL JIEntity::Init(DWORD dwID, std::list<IJG_Buffer *>* pActivateList, std::list<IJG_Buffer *>* pInactivateList, JContext* pContext)
{
    BOOL                             bResult    = false;
    BOOL                             bRetCode   = false;
    std::list<IJG_Buffer*>::iterator itFind;

    m_dwID = dwID;

    if (pActivateList)
    {
        m_ActiveComponentManager.clear();

        for (itFind = pActivateList->begin(); itFind != pActivateList->end(); ++itFind)
        {
            bRetCode = InsertActiveComponent(*itFind);
            JGLOG_PROCESS_ERROR(bRetCode);
        }
    }

    if (pInactivateList)
    {
        m_InactiveComponentManager.clear();

        for (itFind = pInactivateList->begin(); itFind != pInactivateList->end(); ++itFind)
        {
            bRetCode = InsertInactiveComponent(*itFind);
            JGLOG_PROCESS_ERROR(bRetCode);
        }
    }

    if (pContext)
    {
        m_Context = *pContext;
    }
    else
    {
        m_Context.Clear();
    }

    bResult = true;
Exit0:
    return bResult;
}

void JIEntity::UnInit()
{
    ClearActivateComponents();
    ClearInactivateComponents();
}

// ------------------ Read Only ------------------------

BOOL JIEntity::HasComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL bResult = false;

    JG_PROCESS_ERROR(HasActivateComponent(eComponentType));

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::HasComponents(JContext* pContext)
{
    BOOL bResult = false;

    JG_PROCESS_ERROR(HasActivateComponents(pContext));

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::HasAnyComponent(JContext* pContext)
{
    BOOL bResult = false;

    JG_PROCESS_ERROR(HasAnyActivateComponent(pContext));

    bResult = true;
Exit0:
    return bResult;
}

const IJG_Buffer* JIEntity::GetComponentReadOnly(JCOMPONENT_TYPE eComponentType)
{

}

// ------------------ Read & Write ------------------------

BOOL JIEntity::AddComponent(IJG_Buffer* piBuffer, BOOL bActiveStatus)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    if (bActiveStatus)
    {
        bRetCode = InsertActiveComponent(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        bRetCode = InsertInactiveComponent(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::RemoveComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    bRetCode = EraseComponent(eComponentType);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

void JIEntity::RemoveAllComponent()
{
    ClearActivateComponents();
    ClearInactivateComponents();
}

BOOL JIEntity::InactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL        bResult  = false;
    BOOL        bRetCode = false;
    IJG_Buffer* piBuffer = NULL;

    piBuffer = GetActivateComponent(eComponentType);
    JGLOG_PROCESS_ERROR(piBuffer);

    piBuffer->AddRef();

    bRetCode = EraseActivateComponent(eComponentType);
    JGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = InsertInactiveComponent(piBuffer);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL JIEntity::InactivateComponents(JContext* pContext)
{
    BOOL            bResult        = false;
    BOOL            bRetCode       = false;
    JCOMPONENT_TYPE eComponentType = ect_end;

    JGLOG_PROCESS_ERROR(pContext);

    pContext->StartTraverse();

    while (true)
    {
        eComponentType = pContext->GetNext();
        if (eComponentType == ect_end)
            break;

        bRetCode = InactivateComponent(eComponentType);
        JGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;

}

BOOL JIEntity::ReactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL        bResult  = false;
    BOOL        bRetCode = false;
    IJG_Buffer* piBuffer = NULL;

    piBuffer = GetInactivateComponent(eComponentType);
    JGLOG_PROCESS_ERROR(piBuffer);

    piBuffer->AddRef();

    bRetCode = EraseInactivateComponent(eComponentType);
    JGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = InsertActiveComponent(piBuffer);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL JIEntity::ReactivateComponents(JContext* pContext)
{
    BOOL            bResult        = false;
    BOOL            bRetCode       = false;
    JCOMPONENT_TYPE eComponentType = ect_end;

    JGLOG_PROCESS_ERROR(pContext);

    pContext->StartTraverse();

    while (true)
    {
        eComponentType = pContext->GetNext();
        if (eComponentType == ect_end)
            break;

        bRetCode = ReactivateComponent(eComponentType);
        JGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::ReplaceComponent(IJG_Buffer* piBuffer)
{
    BOOL         bResult    = false;
    BOOL         bRetCode   = false;
    JIComponent* pComponent = NULL;

    pComponent = GetComponentFromBuffer(piBuffer);
    JGLOG_PROCESS_ERROR(pComponent);

    bRetCode = EraseActivateComponent(pComponent->m_eType);
    if (bRetCode)
    {
        bRetCode = InsertActiveComponent(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    bRetCode = EraseInactivateComponent(pComponent->m_eType);
    if (bRetCode)
    {
        bRetCode = InsertInactiveComponent(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    goto Exit0;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

IJG_Buffer* JIEntity::GetComponent(JCOMPONENT_TYPE eComponentType)
{
    IJG_Buffer* piBuffer = NULL;

    piBuffer = GetActivateComponent(eComponentType);
    JG_PROCESS_SUCCESS(piBuffer);

    piBuffer = GetInactivateComponent(eComponentType);
    JG_PROCESS_SUCCESS(piBuffer);

Exit1:
    return piBuffer;
}


BOOL JIEntity::InsertActiveComponent(IJG_Buffer* piBuffer)
{
    BOOL         bResult    = false;
    int          nRetCode   = 0;
    JIComponent* pComponent = NULL;

    pComponent = GetComponentFromBuffer(piBuffer);
    JGLOG_PROCESS_ERROR(pComponent);

    nRetCode = m_ActiveComponentManager.count(pComponent->m_eType);
    JG_PROCESS_ERROR(nRetCode == 0);

    nRetCode = m_InactiveComponentManager.count(pComponent->m_eType);
    JG_PROCESS_ERROR(nRetCode == 0);

    m_ActiveComponentManager[pComponent->m_eType] = piBuffer;
    piBuffer->AddRef();

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::InsertInactiveComponent(IJG_Buffer* piBuffer)
{
    BOOL         bResult    = false;
    int          nRetCode   = 0;
    JIComponent* pComponent = NULL;

    pComponent = GetComponentFromBuffer(piBuffer);
    JGLOG_PROCESS_ERROR(pComponent);

    nRetCode = m_ActiveComponentManager.count(pComponent->m_eType);
    JG_PROCESS_ERROR(nRetCode == 0);

    nRetCode = m_InactiveComponentManager.count(pComponent->m_eType);
    JG_PROCESS_ERROR(nRetCode == 0);

    m_InactiveComponentManager[pComponent->m_eType] = piBuffer;
    piBuffer->AddRef();

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::EraseComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL bResult = false;

    if (HasActivateComponent(eComponentType))
    {
        EraseActivateComponent(eComponentType);
    }
    else if (HasInactivateComponent(eComponentType))
    {
        EraseInactivateComponent(eComponentType);
    }
    else
    {
        goto Exit0;
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::EraseActivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL                                             bResult  = false;
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind;

    itFind = m_ActiveComponentManager.find(eComponentType);
    JG_PROCESS_ERROR(itFind != m_ActiveComponentManager.end());

    JG_COM_RELEASE(itFind->second);

    m_ActiveComponentManager.erase(itFind);

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::EraseInactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL                                             bResult  = false;
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind;

    itFind = m_InactiveComponentManager.find(eComponentType);
    JG_PROCESS_ERROR(itFind != m_InactiveComponentManager.end());

    JG_COM_RELEASE(itFind->second);

    m_InactiveComponentManager.erase(itFind);

    bResult = true;
Exit0:
    return bResult;
}

void JIEntity::ClearActivateComponents()
{
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind;

    for (itFind = m_ActiveComponentManager.begin(); itFind != m_ActiveComponentManager.end(); ++itFind)
    {
        JG_COM_RELEASE(itFind->second);
    }

    m_ActiveComponentManager.clear();
}

void JIEntity::ClearInactivateComponents()
{
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind;

    for (itFind = m_InactiveComponentManager.begin(); itFind != m_InactiveComponentManager.end(); ++itFind)
    {
        JG_COM_RELEASE(itFind->second);
    }

    m_InactiveComponentManager.clear();
}

BOOL JIEntity::HasActivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL bResult = false;

    JG_PROCESS_ERROR(m_ActiveComponentManager.count(eComponentType));

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::HasInactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL bResult = false;

    JG_PROCESS_ERROR(m_InactiveComponentManager.count(eComponentType));

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::HasActivateComponents(JContext* pContext)
{
    BOOL            bResult        = false;
    BOOL            bRetCode       = false;
    JCOMPONENT_TYPE eComponentType = ect_end;

    JGLOG_PROCESS_ERROR(pContext);

    pContext->StartTraverse();

    while (true)
    {
        eComponentType = pContext->GetNext();
        if (eComponentType == ect_end)
            break;

        bRetCode = HasActivateComponent(eComponentType);
        JG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL JIEntity::HasAnyActivateComponent(JContext* pContext)
{
    BOOL            bResult        = false;
    BOOL            bRetCode       = false;
    JCOMPONENT_TYPE eComponentType = ect_end;

    JGLOG_PROCESS_ERROR(pContext);

    pContext->StartTraverse();

    while (true)
    {
        eComponentType = pContext->GetNext();
        JG_PROCESS_ERROR(eComponentType != ect_end);

        bRetCode = HasActivateComponent(eComponentType);
        JG_PROCESS_SUCCESS(bRetCode);
    }

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

IJG_Buffer* JIEntity::GetActivateComponent(JCOMPONENT_TYPE eComponentType)
{
    IJG_Buffer*                                      piBuffer = NULL;
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind   = m_ActiveComponentManager.find(eComponentType);

    JG_PROCESS_ERROR(itFind != m_ActiveComponentManager.end());

    piBuffer = itFind->second;

Exit0:
    return piBuffer;
}

IJG_Buffer* JIEntity::GetInactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    IJG_Buffer*                                      piBuffer = NULL;
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind   = m_InactiveComponentManager.find(eComponentType);

    JG_PROCESS_ERROR(itFind != m_InactiveComponentManager.end());

    piBuffer = itFind->second;

Exit0:
    return piBuffer;
}
