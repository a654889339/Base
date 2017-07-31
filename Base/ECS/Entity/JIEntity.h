#ifndef _JIENTITY_H_
#define _JIENTITY_H_

#include <list>
#include "JBaseDef.h"
#include "JG_Memory.h"
#include "JComponentDef.h"
#include "JMatch.h"

class JIEntity
{
public:
    virtual BOOL Init(DWORD dwID, std::list<IJG_Buffer *>* pActivateList, std::list<IJG_Buffer *>* pInactivateList) = 0;
    virtual void UnInit() = 0;

public: // ReadOnly, only check active components.
    virtual BOOL HasActivateComponent(JCOMPONENT_TYPE eComponentType) = 0;
    virtual BOOL HasActivateComponents(JMatch* pContext) = 0;
    virtual BOOL HasAnyActivateComponent(JMatch* pContext) = 0;

    virtual const IJG_Buffer* GetComponentReadOnly(JCOMPONENT_TYPE eComponentType) = 0;

public:
    virtual BOOL AddComponent(IJG_Buffer* piComponent, BOOL bActiveStatus) = 0;
    virtual BOOL RemoveComponent(JCOMPONENT_TYPE eComponentType) = 0;
    virtual void RemoveAllComponent() = 0;

    virtual BOOL InactivateComponent(JCOMPONENT_TYPE eComponentType) = 0;
    virtual BOOL InactivateComponents(JMatch* pContext) = 0;
    virtual BOOL ReactivateComponent(JCOMPONENT_TYPE eComponentType) = 0;
    virtual BOOL ReactivateComponents(JMatch* pContext) = 0;

    virtual BOOL ReplaceComponent(IJG_Buffer* piBuffer) = 0;

    virtual IJG_Buffer* GetComponent(JCOMPONENT_TYPE eComponentType) = 0;

protected:
    virtual BOOL AddActiveComponent(IJG_Buffer* piBuffer) = 0;
    virtual BOOL AddInactiveComponent(IJG_Buffer* piBuffer) = 0;

    virtual BOOL RemoveActivateComponent(JCOMPONENT_TYPE eComponentType) = 0;
    virtual BOOL RemoveInactivateComponent(JCOMPONENT_TYPE eComponentType) = 0;

    virtual void ClearActivateComponents() = 0;
    virtual void ClearInactivateComponents() = 0;

    virtual BOOL HasInactivateComponent(JCOMPONENT_TYPE eComponentType) = 0;

    virtual IJG_Buffer* GetActivateComponent(JCOMPONENT_TYPE eComponentType) = 0;
    virtual IJG_Buffer* GetInactivateComponent(JCOMPONENT_TYPE eComponentType) = 0;
};

#endif // _JIENTITY_H_