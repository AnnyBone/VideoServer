
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

bool should_stop = false;
HANDLE shutdown_ev;

BOOL ctrl_handler (DWORD ctrl_type)
{
    should_stop = true;
    WaitForSingleObject (shutdown_ev, INFINITE);
    CloseHandle (shutdown_ev);
    return TRUE;
}

int __cdecl main (int argc, char** argv)
{
    // setup Ctrl-C etc. handler
    shutdown_ev = CreateEvent (0, FALSE, FALSE, 0);
    SetConsoleCtrlHandler ((PHANDLER_ROUTINE)ctrl_handler, TRUE);

    int fps = 25;
    int x = 100;
    int y = 100;
    int w = 640;
    int h = 480;

    // setup timer
    TIMECAPS tc;
    timeGetDevCaps (&tc, sizeof (tc));
    fprintf (stdout, "timer resolution=%d ms\n", (int)tc.wPeriodMin);

    timeBeginPeriod (tc.wPeriodMin);
    MMRESULT timer_id = 0;
    HANDLE timer_ev = 0;

    // setup local clock
    LARGE_INTEGER ticks_per_sec, ticks;
    QueryPerformanceFrequency (&ticks_per_sec);
    fprintf (stdout, "ticks per sec=%lld\n", ticks_per_sec.QuadPart);

    // local timestamp, in msec
    #define xxnow() \
        (QueryPerformanceCounter (&ticks), (1e3*ticks.QuadPart/ticks_per_sec.QuadPart))

    // setup grabber
    HWND window = GetDesktopWindow ();
    HDC window_dc = GetDC (window);
    HDC memory_dc = CreateCompatibleDC (window_dc);
    HBITMAP bitmap = CreateCompatibleBitmap (window_dc, w, h);

    BITMAPINFOHEADER bitmap_info;
    ZeroMemory (&bitmap_info, sizeof (bitmap_info));
    bitmap_info.biSize = sizeof (BITMAPINFOHEADER);
    bitmap_info.biPlanes = 1;
    bitmap_info.biBitCount = 32;
    bitmap_info.biWidth = w;
    bitmap_info.biHeight = -h;
    bitmap_info.biCompression = BI_RGB;
    bitmap_info.biSizeImage = 0;

    // allocate grabbed pixels buffer
    void* grabbed_pixels = malloc (w*h*4);

    while (!should_stop) {
        double before = xxnow ();

        // grab screen area
        SelectObject (memory_dc, bitmap);
        BitBlt (memory_dc, 0, 0, w, h, window_dc, x, y, SRCCOPY);
        GetDIBits (memory_dc, bitmap, 0, h, grabbed_pixels,
                (BITMAPINFO*)&bitmap_info, DIB_RGB_COLORS);


        double after = xxnow ();
        double dt = after - before;
        double target_period = 1e3/fps;
        double time_to_sleep = target_period - dt;

        if (time_to_sleep > 0.) {
            timer_id = timeSetEvent ((UINT)time_to_sleep,
                    tc.wPeriodMin, (LPTIMECALLBACK)timer_ev, 0,
                    TIME_CALLBACK_EVENT_SET|TIME_ONESHOT);
            WaitForSingleObject (timer_ev, INFINITE);

            // spin-wait if necessary
            while (xxnow () < before + target_period);
        }

    }

    fprintf (stdout, "shutting down\n");

    // close grabber
    ReleaseDC (window, window_dc);
    DeleteDC (memory_dc);
    DeleteObject (bitmap);
    free (grabbed_pixels);

    // close timer
    timeKillEvent (timer_id);
    timeEndPeriod (tc.wPeriodMin);
    CloseHandle (timer_ev);

    // release ctrl handler
    SetEvent (shutdown_ev);
    return 0;
}
