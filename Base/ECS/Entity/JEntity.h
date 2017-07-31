#ifndef _JENTITY_H_
#define _JENTITY_H_

#include <map>
#include "JIEntity.h"
#include "JIComponent.h"

class JEntity : JIEntity
{
public:
    JEntity();
    virtual ~JEntity();

    BOOL Init(DWORD dwID, std::list<IJG_Buffer *>* pActivateList, std::list<IJG_Buffer *>* pInactivateList);
    void UnInit();

public: // ReadOnly, only check active components.
    BOOL HasActivateComponent(JCOMPONENT_TYPE eComponentType);
    BOOL HasActivateComponents(JMatch* pMatch);
    BOOL HasAnyActivateComponent(JMatch* pMatch);

    const IJG_Buffer* GetComponentReadOnly(JCOMPONENT_TYPE eComponentType);

public:
    BOOL AddComponent(IJG_Buffer* piComponent, BOOL bActiveStatus);
    BOOL RemoveComponent(JCOMPONENT_TYPE eComponentType);
    void RemoveAllComponent();

    BOOL InactivateComponent(JCOMPONENT_TYPE eComponentType);
    BOOL InactivateComponents(JMatch* pMatch);
    BOOL ReactivateComponent(JCOMPONENT_TYPE eComponentType);
    BOOL ReactivateComponents(JMatch* pMatch);

    BOOL ReplaceComponent(IJG_Buffer* piBuffer);

    IJG_Buffer* GetComponent(JCOMPONENT_TYPE eComponentType);

private:
    BOOL AddActiveComponent(IJG_Buffer* piBuffer);
    BOOL AddInactiveComponent(IJG_Buffer* piBuffer);

    BOOL RemoveActivateComponent(JCOMPONENT_TYPE eComponentType);
    BOOL RemoveInactivateComponent(JCOMPONENT_TYPE eComponentType);

    void ClearActivateComponents();
    void ClearInactivateComponents();

    BOOL HasInactivateComponent(JCOMPONENT_TYPE eComponentType);

    IJG_Buffer* GetActivateComponent(JCOMPONENT_TYPE eComponentType);
    IJG_Buffer* GetInactivateComponent(JCOMPONENT_TYPE eComponentType);

private:
    DWORD             m_dwID;
    JMatch            m_ActivateMatch;
    JMatch            m_InactivateMatch;

    typedef std::map<JCOMPONENT_TYPE, IJG_Buffer*> COMPONENT_MANAGER;
    COMPONENT_MANAGER                              m_ActiveComponentManager;
    COMPONENT_MANAGER                              m_InactiveComponentManager;
};

#endif // _JENTITY_H_