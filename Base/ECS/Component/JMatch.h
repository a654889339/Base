#ifndef _JCONTEXT_H_
#define _JCONTEXT_H_

#include <set>
#include "JBaseDef.h"
#include "JComponentDef.h"

typedef std::set<JCOMPONENT_TYPE> MatchTypeManager;

class JMatch
{
public:
    JMatch();

    void Clear();

    BOOL Contain(JMatch* pMatch);
    BOOL Contain(JCOMPONENT_TYPE eComponentType);
    BOOL HasIntersection(JMatch* pMatch);

    BOOL AddComponent(JCOMPONENT_TYPE eComponentType);
    BOOL RemoveComponent(JCOMPONENT_TYPE eComponentType);

    MatchTypeManager     m_MatchType;
};

#endif // _JCONTEXT_H_