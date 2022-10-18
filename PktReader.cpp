#include "LocDefines.h"
#include "PktReader.h"


CRITICAL_SECTION critsect;
//------------------------------------------------------------------------
// Constructor/Destructor
//
CPktReader::CPktReader()
{
    hFile = INVALID_HANDLE_VALUE;
    FirstPacketOffset = 0;
    IsM2TS = false;
    InitializeCriticalSection(&critsect);
}

CPktReader::~CPktReader()
{
    Close();
}

//------------------------------------------------------------------------
// Throw an exception
//
void CPktReader::ThrowError(PCHAR pMsg)
{
    Close();
    throw pMsg;
}

//------------------------------------------------------------------------
// Open/Close file
//
void CPktReader::Open(PWCHAR pFileName)
{
    hFile = CreateFileW(pFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if(hFile == INVALID_HANDLE_VALUE) ThrowError("Could not open input file");
}

void CPktReader::Close()
{
    if(hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
}

//------------------------------------------------------------------------
// Get file size
//
UINT64 CPktReader::GetSize()
{
    DWORD Hi;
    DWORD Lo = GetFileSize(hFile, &Hi);
    if(Lo == 0xffffffff && GetLastError() != NO_ERROR) return -1;
    return (((UINT64)Hi) << 32) + Lo;
}

//------------------------------------------------------------------------
// Initialize
//
void CPktReader::Initialize(PWCHAR pFileName)
{
    EnterCriticalSection(&critsect);
    Open(pFileName);
    fileSize = GetSize();
    if(fileSize == -1) ThrowError("Could not determine file size");
    if(fileSize < TP_SIZE * 3) ThrowError("File is too small");
    DetermineFileType();
    LeaveCriticalSection(&critsect);
}

//------------------------------------------------------------------------
// Determine if this is M2T or M2TS stream
//


void CPktReader::DetermineFileType()
{
    DWORD BytesRead = Read(M2TSBuffer, TP_SIZE * 100);
    LONG Counter = 0; 

    while (BytesRead != 0)
    {
        if(ReadByte(&Counter, BytesRead)) return;
        SetBuffer(Counter);
        BytesRead = ReadRefact(M2TSBuffer, (TP_SIZE + 4) * 100);
    }
    startMovePosition = Counter;
    areaMove = fileSize - startMovePosition;
}

bool CPktReader::ReadTest(LONG* Counter, DWORD BytesRead)
{
    for (DWORD Idx = 0; Idx < BytesRead; Idx++)
    {
        if (CheckSyncByte(Idx))
        {
            SetBuffer(Idx);
            BytesRead = ReadRefact(M2TSBuffer, (TP_SIZE + 4) * 100);
            if (IsValidPointer(0)) return true;
        }
        *Counter+=1;
    }
    return false;
}

bool CPktReader::ReadByte(LONG* Counter, DWORD BytesRead)
{
    for (DWORD Idx = 0; Idx < BytesRead; Idx++)
    {
        if (CheckSyncByte(Idx))
        {
            SetBuffer(*Counter);
            BytesRead = ReadRefact(M2TSBuffer, (TP_SIZE + 4)  * 100);       
            if(IsValidPointer(0)) return true;
            Idx = 0;
        }
        *Counter+=1;
    }
    return false;
}

DWORD CPktReader::ReadRefact(LPBYTE pBuffer, DWORD ByteToRead)
{
    DWORD BytesRead;
    if (!ReadFile(hFile, pBuffer, ByteToRead, &BytesRead, 0)) ThrowError("Could not read input file");
    return BytesRead;
}

bool CPktReader::CheckSyncByte(DWORD Idx)
{
    if (M2TSBuffer[Idx] == SYNC_BYTE) return true;
    return false;
}

void CPktReader::SetBuffer(LONG Idx)
{
    EnterCriticalSection(&critsect);
    DWORD dwptr = SetFilePointer(hFile, Idx, 0, FILE_BEGIN);
    if(dwptr == INVALID_SET_FILE_POINTER && GetLastError() != 0) ThrowError("Could not rewind the file");
    LeaveCriticalSection(&critsect);
}

LONG CPktReader::GetFilePointer()
{
    EnterCriticalSection(&critsect);
    return SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
    LeaveCriticalSection(&critsect);
}

bool CPktReader::IsValidPointer(DWORD Idx)
{
    DWORD Idx1 = Idx + TP_SIZE;
    DWORD Idx2 = Idx1 + TP_SIZE;
    return M2TSBuffer[Idx] == SYNC_BYTE && M2TSBuffer[Idx1] == SYNC_BYTE && M2TSBuffer[Idx2] == SYNC_BYTE;
}


//------------------------------------------------------------------------
// See if there are at least three consecutive valid packets
//
bool CPktReader::IsM2TFile(DWORD Idx, DWORD BytesInBuffer)
{
    DWORD Idx1 = Idx + TP_SIZE;
    DWORD Idx2 = Idx1 + TP_SIZE;
    if(Idx < BytesInBuffer && Idx1 < BytesInBuffer && Idx2 < BytesInBuffer)
    {
        return M2TSBuffer[Idx] == SYNC_BYTE && M2TSBuffer[Idx1] == SYNC_BYTE && M2TSBuffer[Idx2] == SYNC_BYTE;
    }
    return false;
}

bool CPktReader::IsM2TSFile(DWORD Idx, DWORD BytesInBuffer)
{
    /* there is an additional DWORD between packets in m2ts files */
    DWORD Idx1 = Idx + TP_SIZE + 4;
    DWORD Idx2 = Idx1 + TP_SIZE + 4;
    if(Idx < BytesInBuffer && Idx1 < BytesInBuffer && Idx2 < BytesInBuffer)
    {
        return M2TSBuffer[Idx] == SYNC_BYTE && M2TSBuffer[Idx1] == SYNC_BYTE && M2TSBuffer[Idx2] == SYNC_BYTE;
    }
    return false;
}

//------------------------------------------------------------------------
// Read file
//
DWORD CPktReader::Read(LPBYTE pBuffer, DWORD BytesToRead)
{
    DWORD BytesRead;
    if (!ReadFile(hFile, pBuffer, BytesToRead, &BytesRead, 0)) ThrowError("Could not read input file");
    return BytesRead;
}

//------------------------------------------------------------------------
// Rewind the file
//
void CPktReader::RewindToBeginning()
{
    if(SetFilePointer(hFile, FirstPacketOffset, 0, FILE_BEGIN) == 0xffffffff && GetLastError() != 0) ThrowError("Could not rewind the file");
}

//------------------------------------------------------------------------
// Read the next sample
//
DWORD CPktReader::GetSample(LPBYTE pBuffer)
{
    DWORD NumPackets, BytesRead;
    if(IsM2TS)
    {
            /* this is a M2TS file so we need to perform the 192 -> 188 conversion */
            BytesRead = Read(M2TSBuffer, (TP_SIZE + 4) * MAX_PACKETS_PER_SAMPLE);
            NumPackets = BytesRead / (TP_SIZE + 4);
        LPBYTE pSrc = M2TSBuffer;
        for(DWORD i = 0; i < NumPackets; i++)
        {
            memcpy(pBuffer, pSrc, TP_SIZE);
            pBuffer += TP_SIZE;
            pSrc += TP_SIZE + 4;
        }
    }
    else
    {
        /* this is a M2T file so we can read the packets directly */
        BytesRead = Read(pBuffer, TP_SIZE * MAX_PACKETS_PER_SAMPLE);
        NumPackets = BytesRead / TP_SIZE;
    }
    return NumPackets * TP_SIZE;
}

DWORD CPktReader::GetFirstPacketId()
{
    return FirstPacketOffset;
}

UINT64 CPktReader::GetMovePosition()
{
    EnterCriticalSection(&critsect);
    currentMovePosition = GetFilePointer();
    LeaveCriticalSection(&critsect);
    deltaMove = fileSize - currentMovePosition;
    return deltaMove / areaMove * 100;
}

