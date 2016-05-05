
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <conio.h>

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
    vtime_t* clk = time_new ();

    SDL_Event event;
    char debug_info[50];

    double curr_dt = 0.;
    SYSTEMTIME st;

    while (!should_stop) {
        char cmd = _kbhit()? _getch() : 0;
        if (cmd == ' ')
            time_freeze (clk, !clk->frozen);

        double before = time_now (clk);

        double xt = before - time_frozen (clk);
        time_as_systemtime (xt, &st);
        snprintf (debug_info, 50, "%03.0f %02d:%02d:%02d.%03d",
            curr_dt, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        grabber_debug_info (grabber, debug_info);

        void* pixels = grabber_capture (grabber);
        display_update (display, pixels);
        display_draw (display);

        SDL_PollEvent (&event);

        double after = time_now (clk);
        curr_dt = after - before;
        double target_period = 1e3/fps;
        double time_to_wait = target_period - curr_dt;

        time_wait (clk, time_to_wait);
    }

    fprintf (stdout, "shutting down\n");

    time_destroy (&clk);
    display_destroy (&display);
    grabber_destroy (&grabber);

    // release ctrl handler
    SetEvent (shutdown_ev);
    return 0;
}
