#ifndef VTIME_H
#define VTIME_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

struct vtime_s
{
    TIMECAPS tc;
    MMRESULT timer_id;
    HANDLE timer_ev;

    LARGE_INTEGER ticks_per_sec, start_ticks;
};

typedef struct vtime_s vtime_t;

vtime_t* time_new (void);
void time_destroy (vtime_t** pself);

// return current time since last call to time_new () in msec
double time_now (vtime_t* self);

// block current thread for the specified amount of time in msec
// return actual time spent waiting
double time_wait (vtime_t* self, double time_to_wait);

#endif
