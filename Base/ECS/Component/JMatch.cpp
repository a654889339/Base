#include "JMatch.h"


JMatch::JMatch()
{
    Clear();
}

void JMatch::Clear()
{
    m_MatchType.clear();
}

BOOL JMatch::Contain(JMatch* pMatch)
{
    BOOL                       bResult = false;
    MatchTypeManager::iterator itFind;

    JGLOG_PROCESS_ERROR(pMatch);

    JG_PROCESS_ERROR(pMatch->m_MatchType.size() <= m_MatchType.size());

    for (itFind = pMatch->m_MatchType.begin(); itFind != pMatch->m_MatchType.end(); ++itFind)
    {
        JG_PROCESS_ERROR(m_MatchType.count(*itFind));
    }

    bResult = true;
Exit0:
    return bResult;
}

BOOL JMatch::Contain(JCOMPONENT_TYPE eComponentType)
{
    return m_MatchType.count(eComponentType) > 0;
}

BOOL JMatch::HasIntersection(JMatch* pMatch)
{
    BOOL                       bResult = false;
    MatchTypeManager::iterator itFind;

    JGLOG_PROCESS_ERROR(pMatch);

    for (itFind = pMatch->m_MatchType.begin(); itFind != pMatch->m_MatchType.end(); ++itFind)
    {
        if (m_MatchType.count(*itFind))
        {
            bResult = true;
            goto Exit0;
        }
    }

Exit0:
    return bResult;
}

BOOL JMatch::AddComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL                       bResult = false;
    MatchTypeManager::iterator itFind;

    itFind = m_MatchType.find(eComponentType);
    JGLOG_PROCESS_ERROR(itFind == m_MatchType.end());

    m_MatchType.insert(eComponentType);

    bResult = true;
Exit0:
    return bResult;
}

BOOL JMatch::RemoveComponent(JCOMPONENT_TYPE eComponentType)
{
    BOOL                       bResult = false;
    MatchTypeManager::iterator itFind;

    itFind = m_MatchType.find(eComponentType);
    JGLOG_PROCESS_ERROR(itFind != m_MatchType.end());

    m_MatchType.erase(eComponentType);

    bResult = true;
Exit0:
    return bResult;
}