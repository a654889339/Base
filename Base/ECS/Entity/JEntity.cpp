#include "JEntity.h"

JEntity::JEntity()
{
    m_dwID = 0;
    m_ActivateMatch.Clear();
    m_InactivateMatch.Clear();
}

JEntity::~JEntity()
{
    UnInit();
}

BOOL JEntity::Init(DWORD dwID, std::list<IJG_Buffer *>* pActivateList, std::list<IJG_Buffer *>* pInactivateList)
{
    BOOL                             bResult    = false;
    BOOL                             bRetCode   = false;
    std::list<IJG_Buffer*>::iterator itFind;

    m_dwID = dwID;

    if (pActivateList)
    {
        m_ActivateMatch.Clear();
        m_ActiveComponentManager.clear();

        for (itFind = pActivateList->begin(); itFind != pActivateList->end(); ++itFind)
        {
            bRetCode = AddActiveComponent(*itFind);
            JGLOG_PROCESS_ERROR(bRetCode);
        }
    }

    if (pInactivateList)
    {
        m_InactivateMatch.Clear();
        m_InactiveComponentManager.clear();

        for (itFind = pInactivateList->begin(); itFind != pInactivateList->end(); ++itFind)
        {
            bRetCode = AddInactiveComponent(*itFind);
            JGLOG_PROCESS_ERROR(bRetCode);
        }
    }

    bResult = true;
Exit0:
    return bResult;
}

void JEntity::UnInit()
{
    ClearActivateComponents();
    ClearInactivateComponents();
}

// ------------------ Read Only ------------------------

BOOL JEntity::HasActivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL bResult = false;

    JG_PROCESS_ERROR(m_ActivateMatch.Contain(eComponentType));

    bResult = true;
Exit0:
    return bResult;
}

BOOL JEntity::HasActivateComponents(JMatch* pMatch)
{
    BOOL            bResult        = false;
    BOOL            bRetCode       = false;
    JCOMPONENT_TYPE eComponentType = ect_end;

    JGLOG_PROCESS_ERROR(pMatch);

    bRetCode = m_ActivateMatch.Contain(pMatch);
    JG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

BOOL JEntity::HasAnyActivateComponent(JMatch* pMatch)
{
    BOOL            bResult        = false;
    BOOL            bRetCode       = false;

    JGLOG_PROCESS_ERROR(pMatch);

    bRetCode = m_ActivateMatch.HasIntersection(pMatch);
    JG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    return bResult;
}

const IJG_Buffer* JEntity::GetComponentReadOnly(JCOMPONENT_TYPE eComponentType)
{
    IJG_Buffer* piBuffer = NULL;

    piBuffer = GetActivateComponent(eComponentType);

    return piBuffer;
}

// ------------------ Read & Write ------------------------

BOOL JEntity::AddComponent(IJG_Buffer* piBuffer, BOOL bActiveStatus)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    if (bActiveStatus)
    {
        bRetCode = AddActiveComponent(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        bRetCode = AddInactiveComponent(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL JEntity::RemoveComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL bResult  = false;
    BOOL bRetCode = false;

    if (HasActivateComponent(eComponentType))
    {
        bRetCode = RemoveActivateComponent(eComponentType);
        JGLOG_PROCESS_ERROR(bRetCode);
    }
    else if (HasInactivateComponent(eComponentType))
    {
        bRetCode = RemoveActivateComponent(eComponentType);
        JGLOG_PROCESS_ERROR(bRetCode);
    }
    else
    {
        goto Exit0;
    }

    bResult = true;
Exit0:
    return bResult;
}

void JEntity::RemoveAllComponent()
{
    ClearActivateComponents();
    ClearInactivateComponents();
}

BOOL JEntity::InactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL        bResult  = false;
    BOOL        bRetCode = false;
    IJG_Buffer* piBuffer = NULL;

    piBuffer = GetActivateComponent(eComponentType);
    JGLOG_PROCESS_ERROR(piBuffer);

    piBuffer->AddRef();

    bRetCode = RemoveActivateComponent(eComponentType);
    JGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = AddInactiveComponent(piBuffer);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL JEntity::InactivateComponents(JMatch* pMatch)
{
    BOOL            bResult        = false;
    BOOL            bRetCode       = false;

    JGLOG_PROCESS_ERROR(pMatch);

    for (MatchTypeManager::iterator itFind = pMatch->m_MatchType.begin(); itFind != pMatch->m_MatchType.end(); ++itFind)
    {
        bRetCode = InactivateComponent(*itFind);
        JGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;

}

BOOL JEntity::ReactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL        bResult  = false;
    BOOL        bRetCode = false;
    IJG_Buffer* piBuffer = NULL;

    piBuffer = GetInactivateComponent(eComponentType);
    JGLOG_PROCESS_ERROR(piBuffer);

    piBuffer->AddRef();

    bRetCode = RemoveInactivateComponent(eComponentType);
    JGLOG_PROCESS_ERROR(bRetCode);

    bRetCode = AddActiveComponent(piBuffer);
    JGLOG_PROCESS_ERROR(bRetCode);

    bResult = true;
Exit0:
    JG_COM_RELEASE(piBuffer);
    return bResult;
}

BOOL JEntity::ReactivateComponents(JMatch* pMatch)
{
    BOOL            bResult        = false;
    BOOL            bRetCode       = false;
    JCOMPONENT_TYPE eComponentType = ect_end;

    JGLOG_PROCESS_ERROR(pMatch);

    for (MatchTypeManager::iterator itFind = pMatch->m_MatchType.begin(); itFind != pMatch->m_MatchType.end(); ++itFind)
    {
        bRetCode = ReactivateComponent(*itFind);
        JGLOG_PROCESS_ERROR(bRetCode);
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL JEntity::ReplaceComponent(IJG_Buffer* piBuffer)
{
    BOOL         bResult    = false;
    BOOL         bRetCode   = false;
    JIComponent* pComponent = NULL;

    pComponent = GetComponentFromBuffer(piBuffer);
    JGLOG_PROCESS_ERROR(pComponent);

    bRetCode = RemoveActivateComponent(pComponent->m_eType);
    if (bRetCode)
    {
        bRetCode = AddActiveComponent(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    bRetCode = RemoveInactivateComponent(pComponent->m_eType);
    if (bRetCode)
    {
        bRetCode = AddInactiveComponent(piBuffer);
        JGLOG_PROCESS_ERROR(bRetCode);

        goto Exit1;
    }

    goto Exit0;

Exit1:
    bResult = true;
Exit0:
    return bResult;
}

IJG_Buffer* JEntity::GetComponent(JCOMPONENT_TYPE eComponentType)
{
    IJG_Buffer* piBuffer = NULL;

    piBuffer = GetActivateComponent(eComponentType);
    JG_PROCESS_SUCCESS(piBuffer);

    piBuffer = GetInactivateComponent(eComponentType);
    JG_PROCESS_SUCCESS(piBuffer);

Exit1:
    return piBuffer;
}


BOOL JEntity::AddActiveComponent(IJG_Buffer* piBuffer)
{
    BOOL         bResult    = false;
    BOOL         bRetCode   = false;
    JIComponent* pComponent = NULL;

    pComponent = GetComponentFromBuffer(piBuffer);
    JGLOG_PROCESS_ERROR(pComponent);

    bRetCode = HasActivateComponent(pComponent->m_eType);
    JG_PROCESS_ERROR(!bRetCode);

    bRetCode = HasInactivateComponent(pComponent->m_eType);
    JG_PROCESS_ERROR(!bRetCode);

    bRetCode = m_ActivateMatch.AddComponent(pComponent->m_eType);
    JGLOG_PROCESS_ERROR(bRetCode);

    m_ActiveComponentManager[pComponent->m_eType] = piBuffer;
    piBuffer->AddRef();

    bResult = true;
Exit0:
    return bResult;
}

BOOL JEntity::AddInactiveComponent(IJG_Buffer* piBuffer)
{
    BOOL         bResult    = false;
    BOOL         bRetCode   = false;
    JIComponent* pComponent = NULL;

    pComponent = GetComponentFromBuffer(piBuffer);
    JGLOG_PROCESS_ERROR(pComponent);

    bRetCode = HasActivateComponent(pComponent->m_eType);
    JG_PROCESS_ERROR(!bRetCode);

    bRetCode = HasInactivateComponent(pComponent->m_eType);
    JG_PROCESS_ERROR(!bRetCode);

    bRetCode = m_InactivateMatch.AddComponent(pComponent->m_eType);
    JGLOG_PROCESS_ERROR(bRetCode);

    m_InactiveComponentManager[pComponent->m_eType] = piBuffer;
    piBuffer->AddRef();

    bResult = true;
Exit0:
    return bResult;
}

BOOL JEntity::RemoveActivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL                                             bResult  = false;
    BOOL                                             bRetCode = false;
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind;

    itFind = m_ActiveComponentManager.find(eComponentType);
    JG_PROCESS_ERROR(itFind != m_ActiveComponentManager.end());

    JG_COM_RELEASE(itFind->second);

    bRetCode = m_ActivateMatch.RemoveComponent(eComponentType);
    JGLOG_PROCESS_ERROR(bRetCode);

    m_ActiveComponentManager.erase(itFind);

    bResult = true;
Exit0:
    return bResult;
}

BOOL JEntity::RemoveInactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL                                             bResult  = false;
    BOOL                                             bRetCode = false;
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind;

    itFind = m_InactiveComponentManager.find(eComponentType);
    JG_PROCESS_ERROR(itFind != m_InactiveComponentManager.end());

    JG_COM_RELEASE(itFind->second);

    bRetCode = m_InactivateMatch.RemoveComponent(eComponentType);
    JGLOG_PROCESS_ERROR(bRetCode);

    m_InactiveComponentManager.erase(itFind);

    bResult = true;
Exit0:
    return bResult;
}

void JEntity::ClearActivateComponents()
{
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind;

    for (itFind = m_ActiveComponentManager.begin(); itFind != m_ActiveComponentManager.end(); ++itFind)
    {
        JG_COM_RELEASE(itFind->second);
    }

    m_ActivateMatch.Clear();
    m_ActiveComponentManager.clear();
}

void JEntity::ClearInactivateComponents()
{
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind;

    for (itFind = m_InactiveComponentManager.begin(); itFind != m_InactiveComponentManager.end(); ++itFind)
    {
        JG_COM_RELEASE(itFind->second);
    }

    m_InactivateMatch.Clear();
    m_InactiveComponentManager.clear();
}

BOOL JEntity::HasInactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL bResult = false;

    JG_PROCESS_ERROR(m_InactivateMatch.Contain(eComponentType));

    bResult = true;
Exit0:
    return bResult;
}

IJG_Buffer* JEntity::GetActivateComponent(JCOMPONENT_TYPE eComponentType)
{
    IJG_Buffer*                                      piBuffer = NULL;
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind   = m_ActiveComponentManager.find(eComponentType);

    JG_PROCESS_ERROR(HasActivateComponent(eComponentType));
    JGLOG_PROCESS_ERROR(itFind != m_ActiveComponentManager.end());

    piBuffer = itFind->second;

Exit0:
    return piBuffer;
}

IJG_Buffer* JEntity::GetInactivateComponent(JCOMPONENT_TYPE eComponentType)
{
    IJG_Buffer*                                      piBuffer = NULL;
    std::map<JCOMPONENT_TYPE, IJG_Buffer*>::iterator itFind   = m_InactiveComponentManager.find(eComponentType);

    JG_PROCESS_ERROR(HasInactivateComponent(eComponentType));
    JGLOG_PROCESS_ERROR(itFind != m_InactiveComponentManager.end());

    piBuffer = itFind->second;

Exit0:
    return piBuffer;
}
