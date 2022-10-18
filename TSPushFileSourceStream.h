#pragma once

#include "PktReader.h"
//#include <Windows.h>
#include <dshow.h>
#include <streams.h>


class CTSPushFileSourceStream : public CSourceStream
{
private:
    CPktReader *pPktReader;
    DWORD FrameNumber;

public:
    CTSPushFileSourceStream(HRESULT *pHr, CSource *pParent, CPktReader *pAReader);
    virtual ~CTSPushFileSourceStream();

    HRESULT FillBuffer(IMediaSample *pOutSample);
    STDMETHODIMP Notify(IBaseFilter *pSender, Quality Q);
    HRESULT GetMediaType(CMediaType *Mt);
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProps);
    HRESULT OnThreadStartPlay();
    DWORD GetPacketSeek();
};
