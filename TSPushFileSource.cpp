#include <streams.h>
#include "LocGuids.h"
#include "TSPushFileSource.h"

//------------------------------------------------------------------------
// Constructor/Destructor
//
CTSPushFileSource::CTSPushFileSource(LPUNKNOWN pUnk, HRESULT *pHr) : CSource(NAME(TSPUSHFILESOURCE_NAME), pUnk, CLSID_TSPUSHFILESOURCE, pHr)
{
    m_pFileName = 0;
    hFile = INVALID_HANDLE_VALUE;

    /* create all output streams. Don't forget that the pins add themselves to the m_paStreams array automatically.
       Also note that this array gets created automatically when we create the first pin so we don't have to create it manually. */
    pOutPin = new CTSPushFileSourceStream(pHr, this, &PktReader);
    if(pOutPin == 0)
    {
        *pHr = E_OUTOFMEMORY;
        return;
    }

    if(m_iPins != 1 || FAILED(*pHr))
    {
        /* something went wrong. let's delete everything */
        while(m_iPins) delete m_paStreams[0];

        /* the last deleted pin also deletes the array. However, if we couldn't create any pins, the array won't be deleted so we delete it explicitly */
        if(m_paStreams) delete [] m_paStreams;
        m_paStreams = 0;

        pOutPin = 0;
        *pHr = CO_E_CLASS_CREATE_FAILED;
        return;
    }
}

CTSPushFileSource::~CTSPushFileSource()
{
    if(pOutPin) delete pOutPin;
    pOutPin = 0;
    DeleteFileName();
}

//------------------------------------------------------------------------
// Create instance
//
CUnknown * WINAPI CTSPushFileSource::CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr)
{
    CTSPushFileSource *RVCESSource = new CTSPushFileSource(pUnk, pHr);
    if(RVCESSource == 0)
    {
        *pHr = E_OUTOFMEMORY;
        return 0;
    }
    if(FAILED(*pHr))
    {
        delete RVCESSource;
        RVCESSource = 0;
    }
    return RVCESSource;
}

//------------------------------------------------------------------------
// Allocate/Free memory allocated for the name of the file
//
void CTSPushFileSource::DeleteFileName()
{
    if(m_pFileName) CoTaskMemFree(m_pFileName);
    m_pFileName = 0;
}

WCHAR *CTSPushFileSource::AllocFileName(LPCOLESTR pStr)
{
    int Len = wcslen(pStr) + 1;
    WCHAR *pMem = (WCHAR *)CoTaskMemAlloc(Len * sizeof(WCHAR));
    if(pMem) CopyMemory(pMem, pStr, Len * sizeof(WCHAR));
    return pMem;
}

//------------------------------------------------------------------------
// Non-delegating QI
//
STDMETHODIMP CTSPushFileSource::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if(riid == IID_IFileSourceFilter) return GetInterface((IFileSourceFilter *)this, ppv);
    return __super::NonDelegatingQueryInterface(riid, ppv);
}

//------------------------------------------------------------------------
// IFileSourceFilter methods
//
STDMETHODIMP CTSPushFileSource::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pMt)
{
    /* delete previously allocated buffer (if any) */
    DeleteFileName();

    /* remember file name */
    m_pFileName = AllocFileName(pszFileName);
    if(m_pFileName == 0) return E_OUTOFMEMORY;
    
    CAutoLock lck(&m_cStateLock);

    try
    {
        PktReader.Initialize(m_pFileName);
    }
    catch(...)
    {
        DeleteFileName();
        return VFW_E_NOT_FOUND;
    }

    try
    {
//        if(!Demux.GetStreamParametersFromFile(&StreamWidth, &StreamHeight)) throw "Invalid file";
    }
    catch(...)
    {
        DeleteFileName();
        return VFW_E_UNKNOWN_FILE_TYPE;
    }

    /*  Check the file type. For now, we simply ignore passed media type */
    FileMt.SetType(&MEDIATYPE_Stream);
    FileMt.SetSubtype(&MEDIASUBTYPE_MPEG2_TRANSPORT);

    return S_OK;
}

STDMETHODIMP CTSPushFileSource::GetCurFile(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pMt)
{
    *ppszFileName = 0;
    if(m_pFileName) *ppszFileName = AllocFileName(m_pFileName);
    if(pMt) CopyMediaType(pMt, &FileMt);
    return S_OK;
}

DWORD CTSPushFileSource::GetByteSeek()
{
    return pOutPin->GetPacketSeek();
}
