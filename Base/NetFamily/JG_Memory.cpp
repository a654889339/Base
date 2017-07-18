#include <new>
#include <stdlib.h>
#include "JG_Memory.h"

#define JG_InterlockedIncrement  InterlockedIncrement
#define JG_InterlockedDecrement  InterlockedDecrement

#define JG_MEMORY_USER_DATA_OFFSET (sizeof(JG_Buffer))

class JG_Buffer : public IJG_Buffer
{
public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG  STDMETHODCALLTYPE  AddRef();
    virtual ULONG  STDMETHODCALLTYPE  Release();
    virtual void                     *GetData()  { return m_pvData; } 
    virtual unsigned                  GetSize()  { return m_uSize;  }

    JG_Buffer(unsigned uSize, void *pvData): m_lRefCount(1), m_uSize(uSize), m_pvData(pvData){}

private:  
    ~JG_Buffer(){}

    volatile long   m_lRefCount;

    unsigned        m_uSize;
    void           *m_pvData;
};

HRESULT STDMETHODCALLTYPE JG_Buffer::QueryInterface(REFIID riid, void **ppvObject) 
{ 
    HRESULT hrResult = E_FAIL;

    ASSERT(ppvObject);

    *ppvObject = NULL;

    if (riid == IID_IUnknown)
    {
        *ppvObject = (void *)((IJG_Buffer *)this);
    }
    else
    {
        JG_COM_PROC_ERR_RET_ERR(E_NOINTERFACE);
    }

    ((IUnknown *)(*ppvObject))->AddRef();

    hrResult = S_OK;
Exit0:
    return hrResult;
}

ULONG JG_Buffer::AddRef()
{
    return JG_InterlockedIncrement((long *)&m_lRefCount);
}

ULONG JG_Buffer::Release()
{
    long lRefNow  = 0;

    lRefNow = JG_InterlockedDecrement((long *)&m_lRefCount);
    if (lRefNow > 0)
        return lRefNow;

    if (lRefNow < 0)
    {
        ASSERT(!"Unexpected memory release!");
        JGLogPrintf(JGLOG_ERR, " %s", "Bad use of memory block : Unexpected memory release!");
    }

    this->~JG_Buffer();
    free((void *)this);

    return lRefNow;
}

IJG_Buffer *JG_MemoryCreateBuffer(unsigned uSize)
{
    int         nRetCode    = false;
    unsigned    uBuffSize   = 0;
    void       *pvBuffer    = NULL;
    void       *pvData      = NULL;
    JG_Buffer  *pBuffer     = NULL;

    ASSERT(uSize);
    uBuffSize = uSize + JG_MEMORY_USER_DATA_OFFSET;
    ASSERT(uBuffSize > JG_MEMORY_USER_DATA_OFFSET && "It seems that uSize is negative!");

    pvBuffer = malloc(uBuffSize);
    JG_PROCESS_ERROR(pvBuffer);

    pvData = (void *)(((unsigned char *)pvBuffer) + JG_MEMORY_USER_DATA_OFFSET);

    pBuffer = new(pvBuffer)JG_Buffer(uSize, pvData);  // placement operator new

Exit0:
    return pBuffer;

}