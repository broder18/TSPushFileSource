#pragma once

#include "TSPushFileSourceStream.h"
#include "PktReader.h"
//#include <streams.h>

class CTSPushFileSource : public CSource, public IFileSourceFilter
{
public:
    CTSPushFileSource(LPUNKNOWN pUnk, HRESULT *pHr);
    ~CTSPushFileSource();

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr);     

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

    DECLARE_IUNKNOWN;

    /*  IFileSourceFilter methods */
    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt);
    DWORD GetByteSeek();
    CMediaType *GetMediaType() { return &FileMt; }; //empty function

private:
    CPktReader PktReader;
    CMediaType FileMt;
    LPWSTR m_pFileName;
    HANDLE hFile;
    CTSPushFileSourceStream *pOutPin;

    void DeleteFileName();
    WCHAR *AllocFileName(LPCOLESTR pStr);
};
