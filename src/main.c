
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
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
#include <vfile.h>
#include <vformat_yuv.h>
#include <vencoder_x264.h>
#include <vencoder_vpx.h>


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
double fw(double x, size_t n, size_t k)
{
    assert (n-1>k);

    double mx = pow(10, (double)(n-1-k));
    double eps = pow(10, -(double)k);

    #if 0
    fprintf (stdout, "mx=%f, eps=%f\n", mx, eps);
    #endif

    return x<+mx? x>-mx? x:(-mx+eps):(+mx-eps);
}

#define fw41(x) fw(x,4,1)
#define fw51(x) fw(x,5,1)
#define fw61(x) fw(x,6,1)

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

    int w2 = (w+1)/2;
    int h2 = (h+1)/2;

    enum output_type_e { rgba = 0, yuv, h264, vp9 };
    typedef enum output_type_e output_type;

    output_type ot = vp9;

    vgrabber_t* grabber = grabber_new (x, y, w, h);
    vdisplay_t* display = display_new (w, h);
    vtime_t* time = time_new ();
    vclock_t* clk = clock_new ();

    const char* output_filename =
        ot == rgba? "output.raw" :
        ot == yuv? "output.i420" :
        ot == vp9? "output.vp9" :
            "output.h264";

    vfile_t* file = file_new (output_filename, "w+b");
    vencoder_x264_t* encoder_x264 = encoder_x264_new (w, h, fps);
    vencoder_vpx_t* encoder_vpx = encoder_vpx_new (w, h, fps);

    SDL_Event event;

    bool grabber_embed_debug = true;
    #define debug_info_len 50
    char debug_info[debug_info_len];

    int raw_size = grabber_buffer_size (grabber);
    void* pixels = malloc (raw_size);

    int i420_size = w*h + 2*w2*h2;
    void* i420 = malloc (i420_size);

    double curr_dt = 0.;
    double time_balance = 0.;
    SYSTEMTIME st;

    int frame_number = 0;
    const int max_frame_number = 250;

    while (!should_stop) {
        char cmd = _kbhit()? _getch() : 0;
        if (cmd == ' ')
            clock_toggle_freeze (clk);

        double before = time_now (time);

        if (grabber_embed_debug) {
            double now = clock_now (clk);
            time_as_systemtime (now, &st);
            snprintf (debug_info, debug_info_len,
                "%05.1f %05.1f " TIME_STR_FORMAT " %05d",
                fw51(curr_dt), fw51(-time_balance),
                st.wHour ,st.wMinute, st.wSecond, st.wMilliseconds,
                frame_number);
            grabber_embed_str (grabber, debug_info);
        }

        grabber_capture (grabber, pixels);

        display_update (display, pixels);
        display_draw (display);

        int encoded_size;
        switch (ot) {
            case rgba:
                file_write (file, pixels, raw_size);
                break;
            case yuv:
                format_rgba_to_i420 (pixels, w, h, i420);
                file_write (file, i420, i420_size);
                break;

            case h264:
                encoded_size = encoder_x264_encode (encoder_x264, pixels, frame_number);
                if (encoded_size > 0)
                    file_write (file, encoder_x264_frame (encoder_x264), encoded_size);
                break;

            case vp9:
                encoded_size = encoder_vpx_encode (encoder_vpx, pixels, frame_number);
                break;
        }

        SDL_PollEvent (&event);

        #if 0
        random_wait (time, 10, 60);
        #endif

        ++frame_number;
        if (frame_number >= max_frame_number)
            should_stop = true;

        double after = time_now (time);
        curr_dt = after - before;
        double target_period = 1e3/fps;
        double time_to_wait = target_period - curr_dt;

        time_balance += time_to_wait;
        double actual_wait = time_wait (time, time_balance);
        time_balance -= actual_wait;
    }

    if (ot == h264) {
        while (encoder_x264_has_delayed_frames (encoder_x264))
        {
            int encoded_size = encoder_x264_encode (encoder_x264, 0, 0);
            if (encoded_size > 0)
                file_write (file, encoder_x264_frame (encoder_x264), encoded_size);
        }
    }

    fprintf (stdout, "shutting down\n");
    free (pixels);
    free (i420);

    file_destroy (&file);
    encoder_x264_destroy (&encoder_x264);
    encoder_vpx_destroy (&encoder_vpx);
    clock_destroy (&clk);
    time_destroy (&time);
    display_destroy (&display);
    grabber_destroy (&grabber);

    // release ctrl handler
    SetEvent (shutdown_ev);
    return 0;
}
