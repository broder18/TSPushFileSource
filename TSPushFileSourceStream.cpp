//************************************************************************
// RVC ES Source Stream
//************************************************************************

#include "LocDefines.h"
#include "LocGuids.h"
//#include <streams.h>
#include <dshow.h>

#include "TSPushFileSource.h"
#include "Macros.h"


//------------------------------------------------------------------------
// Constructor/Destructor
//
CTSPushFileSourceStream::CTSPushFileSourceStream(HRESULT *pHr, CSource *pParent, CPktReader *pAReader) : CSourceStream(NAME("CTSPushFileSourceStream"), pHr, pParent, L"M2T")
{
    pPktReader = pAReader;
}

CTSPushFileSourceStream::~CTSPushFileSourceStream()
{
}

//------------------------------------------------------------------------
// Get pin's media type
//
HRESULT CTSPushFileSourceStream::GetMediaType(CMediaType *Mt)
{
    CAutoLock Lock(m_pFilter->pStateLock());

    return CopyMediaType(Mt, ((CTSPushFileSource *)m_pFilter)->GetMediaType());
}

//------------------------------------------------------------------------
// Agree on buffers with receiving pin
//
HRESULT CTSPushFileSourceStream::DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProps)
{
    CAutoLock Lock(m_pFilter->pStateLock());

    pProps->cBuffers = 1;
    pProps->cbBuffer = TP_SIZE * MAX_PACKETS_PER_SAMPLE;

    ALLOCATOR_PROPERTIES ActualProps;
    EXEC_CHECK_HR(pIMemAlloc->SetProperties(pProps, &ActualProps));

    if(ActualProps.cBuffers < pProps->cBuffers || ActualProps.cbBuffer < pProps->cbBuffer) return E_FAIL;
    return S_OK;
}

//------------------------------------------------------------------------
// We're going to start playing
//
HRESULT CTSPushFileSourceStream::OnThreadStartPlay()
{
    CAutoLock Lock(m_pFilter->pStateLock());

    try
    {
        pPktReader->RewindToBeginning();
    }
    catch(...)
    {
        return E_UNEXPECTED;
    }
    return __super::OnThreadStartPlay();
}

//------------------------------------------------------------------------
// Fill buffer
//
HRESULT CTSPushFileSourceStream::FillBuffer(IMediaSample *pOutSample)
{
    LPBYTE lpOutData;
    EXEC_CHECK_HR(pOutSample->GetPointer(&lpOutData));

    if(pOutSample->GetSize() >= (TP_SIZE * MAX_PACKETS_PER_SAMPLE))
    {
        try
        {
            DWORD BytesRead = pPktReader->GetSample(lpOutData);
            if(BytesRead == 0) return S_FALSE;  // EOS
            return pOutSample->SetActualDataLength(BytesRead);
        }
        catch(...) {}
    }
    return E_UNEXPECTED;
}

//------------------------------------------------------------------------
// Base class barks in debug build if this call is not implemented
//
STDMETHODIMP CTSPushFileSourceStream::Notify(IBaseFilter *pSender, Quality Q)
{
    return E_NOTIMPL;
}

DWORD CTSPushFileSourceStream::GetPacketSeek()
{
    return pPktReader->GetFirstPacketId();
}

