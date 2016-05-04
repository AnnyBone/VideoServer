
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <grabber.h>

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
    grabber_t grabber;
    grabber_init (&grabber, x, y, w, h);

    while (!should_stop) {
        double before = xxnow ();

        void* grabbed_pixels = grabber_capture (&grabber);

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

    grabber_close (&grabber);

    // close timer
    timeKillEvent (timer_id);
    timeEndPeriod (tc.wPeriodMin);
    CloseHandle (timer_ev);

    // release ctrl handler
    SetEvent (shutdown_ev);
    return 0;
}
