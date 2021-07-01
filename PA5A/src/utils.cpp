#include "pa5a.hpp"

EXTERN_C_START

/********************************************************************************************************************************/
VOID CreateConsole()
{
    AllocConsole();
    FILE* file = NULL;

    freopen_s(&file, "CONIN$", "r", stdin);
    freopen_s(&file, "CONOUT$", "w", stdout);
    freopen_s(&file, "CONOUT$", "w", stderr);
}
/********************************************************************************************************************************/
LONGLONG GetTimestamp(BOOL inMlliseconds)
{
    struct timeval temps = { 0 };
    long long timestamp = 0;

    gettimeofday(&temps, NULL);

    if (inMlliseconds == TRUE)
        timestamp = temps.tv_sec * 1000LL + temps.tv_usec / 1000;
    else
        timestamp = temps.tv_sec;

    return timestamp;
}
/********************************************************************************************************************************/
int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    const unsigned __int64 epoch = 116444736000000000;
    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;

    GetLocalTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;
    tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}
/********************************************************************************************************************************/
LONGLONG SystemTimeToUnixTimestamp(SYSTEMTIME system_time)
{
    const unsigned __int64 epoch = 116444736000000000;
    struct timeval tp = { 0 };

    FILETIME       file_time = { 0 };
    ULARGE_INTEGER ularge = { 0 };

    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;
    tp.tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
    tp.tv_usec = (long)(system_time.wMilliseconds * 1000);

    return tp.tv_sec;
}
/********************************************************************************************************************************/
SYSTEMTIME UnixTimestampToSystemTime(LONGLONG timestamp)
{
    const unsigned __int64 epoch = 116444736000000000;

    FILETIME       file_time = { 0 };
    SYSTEMTIME     system_time = { 0 };
    ULARGE_INTEGER ularge = { 0 };

    ularge.QuadPart = timestamp * 10000000L + epoch;
    file_time.dwLowDateTime = ularge.LowPart;
    file_time.dwHighDateTime = ularge.HighPart;

    FileTimeToSystemTime(&file_time, &system_time);

    return system_time;
}
/********************************************************************************************************************************/
PCHAR encode_UTF8(LPCWCHAR messageUTF16)
{
    PCHAR messageUTF8 = NULL;
    INT   size = 0;

    size = WideCharToMultiByte(CP_UTF8, 0, messageUTF16, -1, 0, 0, 0, 0);

    messageUTF8 = (PCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CHAR) * size + 1);

    WideCharToMultiByte(CP_UTF8, 0, messageUTF16, -1, messageUTF8, size, 0, 0);

    return messageUTF8;
}
/********************************************************************************************************************************/

EXTERN_C_END