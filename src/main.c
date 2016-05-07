
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h>
#include <assert.h>
#include <math.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <vgrabber.h>
#include <vdisplay.h>
#include <vtime.h>

bool should_stop = false;
HANDLE shutdown_ev;

BOOL ctrl_handler (DWORD ctrl_type)
{
    should_stop = true;
    WaitForSingleObject (shutdown_ev, INFINITE);
    CloseHandle (shutdown_ev);
    return TRUE;
}

double random (double min, double max)
{
    return ((double) rand () / (RAND_MAX+1)) * (max-min+1) + min;
}

void random_wait (vtime_t* time, double min, double max)
{
    double time_to_wait = random (min, max);
    time_wait (time, time_to_wait);
}

// return fix-width float
inline double fw(double x, size_t n, size_t k)
{
    assert (n-1>k);

    double mx = pow(10, n-1-k);
    double eps = pow(10, -(double)k);

    #if 0
    fprintf (stdout, "mx=%f, eps=%f\n", mx, eps);
    #endif

    return x<+mx? x>-mx? x:(-mx+eps):(+mx-eps);
}

#define fw41(x) fw(x,4,1)

int __cdecl main (int argc, char** argv)
{
    int rc;

    // setup Ctrl-C etc. handler
    shutdown_ev = CreateEvent (0, FALSE, FALSE, 0);
    SetConsoleCtrlHandler ((PHANDLER_ROUTINE)ctrl_handler, TRUE);

    // setup SDL
    rc = SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    if (rc < 0) {
        fprintf (stderr, "SDL_Init() failed, message: %s\n", SDL_GetError ());
        return 1;
    }

    int fps = 25;
    int x = 0;
    int y = 0;
    int w = 640;
    int h = 480;

    vgrabber_t* grabber = grabber_new (x, y, w, h);
    vdisplay_t* display = display_new (w, h);
    vtime_t* time = time_new ();
    vclock_t* clk = clock_new ();

    SDL_Event event;

    bool grabber_embed_debug = true;
    const int debug_info_len = 50;
    char debug_info[debug_info_len];

    void* pixels = malloc (grabber_buffer_size (grabber));

    double curr_dt = 0.;
    double time_balance = 0.;
    SYSTEMTIME st;

    while (!should_stop) {
        char cmd = _kbhit()? _getch() : 0;
        if (cmd == ' ')
            clock_toggle_freeze (clk);

        double before = time_now (time);

        if (grabber_embed_debug) {
            double now = clock_now (clk);
            time_as_systemtime (now, &st);
            snprintf (debug_info, debug_info_len,
                "%04.1f %04.1f " TIME_STR_FORMAT,
                fw41(curr_dt), fw41(-time_balance),
                st.wHour ,st.wMinute, st.wSecond, st.wMilliseconds);
            grabber_embed_str (grabber, debug_info);
        }

        grabber_capture (grabber, pixels);
        display_update (display, pixels);
        display_draw (display);

        SDL_PollEvent (&event);

        random_wait (time, 10, 60);

        double after = time_now (time);
        curr_dt = after - before;
        double target_period = 1e3/fps;
        double time_to_wait = target_period - curr_dt;

        time_balance += time_to_wait;
        double actual_wait = time_wait (time, time_balance);
        time_balance -= actual_wait;
    }

    fprintf (stdout, "shutting down\n");

    free (pixels);

    clock_destroy (&clk);
    time_destroy (&time);
    display_destroy (&display);
    grabber_destroy (&grabber);

    // release ctrl handler
    SetEvent (shutdown_ev);
    return 0;
}
