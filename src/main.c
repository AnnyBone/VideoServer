
#include <stdio.h>
#include <stdbool.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

int main (int argc, char** argv)
{
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
    MMRESULT timer_id;
    HANDLE sleep_ev = 0;

    // setup local clock
    LARGE_INTEGER ticks_per_sec, ticks;
    QueryPerformanceFrequency (&ticks_per_sec);
    fprintf (stdout, "ticks per sec=%lld\n", ticks_per_sec.QuadPart);

    // local timestamp, in msec
    #define xxnow() \
        (QueryPerformanceCounter (&ticks), (1e3*ticks.QuadPart/ticks_per_sec.QuadPart))

    double target_period = 1e3/fps;
    bool should_stop = false;

    while (!should_stop) {
        double before = xxnow ();

        // do stuff

        double after = xxnow ();
        double dt = after - before;

        double time_to_sleep = target_period - dt;
        if (time_to_sleep > 0.) {
            timer_id = timeSetEvent ((UINT)time_to_sleep,
                    tc.wPeriodMin, (LPTIMECALLBACK)sleep_ev, 0,
                    TIME_CALLBACK_EVENT_SET|TIME_ONESHOT);
            WaitForSingleObject (sleep_ev, INFINITE);

            // spin-wait if necessary
            while (xxnow () < before + target_period);
        }

    }

    timeKillEvent (timer_id);
    timeEndPeriod (tc.wPeriodMin);
    CloseHandle (sleep_ev);

    return 0;
}
