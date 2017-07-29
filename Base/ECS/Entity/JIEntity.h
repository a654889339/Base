#ifndef _JIENTITY_H_
#define _JIENTITY_H_

#include <list>
#include <map>
#include "JBaseDef.h"
#include "JG_Memory.h"
#include "JIComponent.h"
#include "JContext.h"

class JIEntity
{
public:
    JIEntity();
    virtual ~JIEntity();

    BOOL Init(DWORD dwID, std::list<IJG_Buffer *>* pActivateList, std::list<IJG_Buffer *>* pInactivateList, JContext* pContext);
    void UnInit();

public: // ReadOnly, Only Check Activate Component.
    BOOL HasComponent(JCOMPONENT_TYPE eComponentType);
    BOOL HasComponents(JContext* pContext);
    BOOL HasAnyComponent(JContext* pContext);

    const IJG_Buffer* GetComponentReadOnly(JCOMPONENT_TYPE eComponentType);

public:
    BOOL AddComponent(IJG_Buffer* piComponent, BOOL bActiveStatus);
    BOOL RemoveComponent(JCOMPONENT_TYPE eComponentType);
    void RemoveAllComponent();

    BOOL InactivateComponent(JCOMPONENT_TYPE eComponentType);
    BOOL InactivateComponents(JContext* pContext);
    BOOL ReactivateComponent(JCOMPONENT_TYPE eComponentType);
    BOOL ReactivateComponents(JContext* pContext);

    BOOL ReplaceComponent(IJG_Buffer* piBuffer);

    IJG_Buffer* GetComponent(JCOMPONENT_TYPE eComponentType);

private:
    BOOL InsertActiveComponent(IJG_Buffer* piBuffer);
    BOOL InsertInactiveComponent(IJG_Buffer* piBuffer);

    BOOL EraseComponent(JCOMPONENT_TYPE eComponentType);
    BOOL EraseActivateComponent(JCOMPONENT_TYPE eComponentType);
    BOOL EraseInactivateComponent(JCOMPONENT_TYPE eComponentType);

    void ClearActivateComponents();
    void ClearInactivateComponents();

    BOOL HasActivateComponent(JCOMPONENT_TYPE eComponentType);
    BOOL HasInactivateComponent(JCOMPONENT_TYPE eComponentType);
    BOOL HasActivateComponents(JContext* pContext);
    BOOL HasAnyActivateComponent(JContext* pContext);

    IJG_Buffer* GetActivateComponent(JCOMPONENT_TYPE eComponentType);
    IJG_Buffer* GetInactivateComponent(JCOMPONENT_TYPE eComponentType);

private:
    DWORD               m_dwID;
    JContext            m_Context;

    typedef std::map<JCOMPONENT_TYPE, IJG_Buffer*> COMPONENT_MANAGER;
    COMPONENT_MANAGER                              m_ActiveComponentManager;
    COMPONENT_MANAGER                              m_InactiveComponentManager;
};

#endif // _JIENTITY_H_