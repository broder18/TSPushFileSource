#pragma once

#define TEMP_BUFFER_SIZE    1000000

#define SYNC_BYTE                   0x47
#define TP_SIZE                     188
#define MAX_PACKETS_PER_SAMPLE      100                  

class CPktReader
{
private:
    HANDLE hFile;
    LONG startMovePosition;
    LONG currentMovePosition;
    UINT64 fileSize;
    LONG deltaMove;
    LONG areaMove;
    DWORD FirstPacketOffset;
    bool IsM2TS;
    BYTE M2TSBuffer[(TP_SIZE + 4) * MAX_PACKETS_PER_SAMPLE];
    LPBYTE Temp;
    LPBYTE pSampleStart;
    DWORD SampleLength;
    void Open(PWCHAR pFileName);
    void Close();
    UINT64 GetSize();
    void DetermineFileType();
    bool IsM2TFile(DWORD Idx, DWORD BytesInBuffer);
    bool IsM2TSFile(DWORD Idx, DWORD BytesInBuffer);
    void ThrowError(PCHAR pMsg);
    DWORD Read(LPBYTE pBuffer, DWORD BytesToRead);
    bool ReadByte(LONG* Counter, DWORD BytesRead);
    DWORD ReadRefact(LPBYTE pBuffer, DWORD ByteToRead);
    bool CheckSyncByte(DWORD Idx);
    void SetBuffer(LONG FirstPacketOffset);
    bool IsValidPointer(DWORD Idx);
    bool ReadTest(LONG* Counter, DWORD BytesRead);
    LONG GetFilePointer();



public:
    CPktReader();
    ~CPktReader();
    void Initialize(PWCHAR pFileName);
    DWORD GetSample(LPBYTE pBuffer);
    void RewindToBeginning();
    DWORD GetFirstPacketId();
    UINT64 GetMovePosition();
};