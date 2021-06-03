#include "pa5a.hpp"

/**********************************************************************************************************/
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
/**********************************************************************************************************/
BOOL WINAPI SaveScreenshot(LPCWSTR filename)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    {
        HDC dc = ::GetDC(0);
        int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
        int Height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        int Width = GetSystemMetrics(SM_CXVIRTUALSCREEN);

        printf("SM_XVIRTUALSCREEN(%d) - SM_YVIRTUALSCREEN(%d)\n", x, y);
        printf("SM_CYVIRTUALSCREEN(%d) - SM_CXVIRTUALSCREEN(%d)\n", Height, Width);
        printf("%d\n", GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, 125));

        Height = 1440;
        Width = 2560;
        HDC memdc = CreateCompatibleDC(dc);
        HBITMAP membit = CreateCompatibleBitmap(dc, Width, Height);
        HBITMAP bmpContainer = (HBITMAP)SelectObject(memdc, membit);
        BitBlt(memdc, 0, 0, Width, Height, dc, x, y, SRCCOPY);

        Gdiplus::Bitmap* bmpPtr = new Gdiplus::Bitmap(membit, NULL);

        CLSID clsid;
        if (GetEncoderClsid(L"image/jpeg", &clsid) == -1)
            return FALSE;

        bmpPtr->Save(filename, &clsid);

        //cleanup
        delete bmpPtr;
        SelectObject(memdc, bmpContainer);
        DeleteObject(membit);
        DeleteDC(memdc);
        ReleaseDC(0, dc);
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);

    return TRUE;
}
/**********************************************************************************************************/