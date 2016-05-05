#ifndef VTIME_H
#define VTIME_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <stdbool.h>

struct vtime_s
{
    TIMECAPS tc;
    MMRESULT timer_id;
    HANDLE timer_ev;

    LARGE_INTEGER ticks_per_sec, start_ticks;

    bool frozen;
    LARGE_INTEGER freeze_ticks_ts;
    LONGLONG freeze_ticks_acc;
};

typedef struct vtime_s vtime_t;

vtime_t* time_new (void);
void time_destroy (vtime_t** pself);

// restart time count as per time_now ()
void time_reset (vtime_t* self);

// return current time since last call to time_new () or time_reset (), in msec
// accounts for time_freeze ()
double time_now (vtime_t* self);

// freeze/unfreeze time count
void time_freeze (vtime_t* self, bool should_freeze);

// return time spent in a frozen state
double time_frozen (vtime_t* self);

// block current thread for the specified amount of time in msec
// return actual time spent waiting
double time_wait (vtime_t* self, double time_to_wait);

// convert msec to SYSTEMTIME
void time_as_systemtime (double msec, SYSTEMTIME* st);

#endif
