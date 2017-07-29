#include "JContext.h"


JContext::JContext()
{
    Clear();
}

void JContext::Clear()
{
    memset(szText, 0, sizeof(szText));
    m_dwFind = 0;
}

void JContext::StartTraverse()
{
    m_dwFind = ect_begin;
}

JCOMPONENT_TYPE JContext::GetNext()
{
    JCOMPONENT_TYPE eComponentType = ect_end;

    while (m_dwFind < ect_end)
    {
        if (szText[m_dwFind / 8] & (1 << (m_dwFind & 7)))
        {
            eComponentType = (JCOMPONENT_TYPE)m_dwFind;
            goto Exit0;
        }
        m_dwFind++;
    }

Exit0:
    return eComponentType;
}


BOOL JContext::operator == (const JContext& Context) const
{
    return memcmp(Context.szText, szText, JCOMPONENT_BINARY_SIZE) == 0;
}

JContext JContext::operator & (const JContext& Context) const
{
    JContext ResultText;

    for (int i = 0; i < JCOMPONENT_BINARY_SIZE; i++)
    {
        ResultText.szText[i] = Context.szText[i] & szText[i];
    }

    return ResultText;
}

JContext JContext::operator | (const JContext& Context) const
{
    JContext ResultText;

    for (int i = 0; i < JCOMPONENT_BINARY_SIZE; i++)
    {
        ResultText.szText[i] = Context.szText[i] | szText[i];
    }

    return ResultText;
}
