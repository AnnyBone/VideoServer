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
    LARGE_INTEGER ticks_per_sec;
};

typedef struct vtime_s vtime_t;

vtime_t* time_new (void);
void time_destroy (vtime_t** pself);

// return value of performance counter converted to msec
double time_now (vtime_t* self);

// block current thread for the specified amount of time in msec
// return actual time spent waiting
double time_wait (vtime_t* self, double time_to_wait);

// convert msec to SYSTEMTIME
void time_as_systemtime (double msec, SYSTEMTIME* st);

#define TIME_STR_FORMAT "%02d:%02d:%02d.%03d"
#define TIME_STR_SIZE (2+1+2+1+2+1+3+1)

inline int time_str_size (void) { return TIME_STR_SIZE; }
int time_str (double msec, char* buffer, size_t size);

// restartable, pausable clock
struct vclock_s
{
    bool freeze;
    LARGE_INTEGER ticks_per_sec;
    LARGE_INTEGER start_ts, freeze_ts;
    LONGLONG freeze_acc;
    char str [TIME_STR_SIZE];
};

typedef struct vclock_s vclock_t;

vclock_t* clock_new (void);
void clock_destroy (vclock_t** pself);

void clock_reset (vclock_t* self);

void clock_set_freeze (vclock_t* self, bool freeze);
bool clock_is_freeze (vclock_t* self);
void clock_toggle_freeze (vclock_t* self);

double clock_now (vclock_t* self);
const char* clock_now_str (vclock_t* self);

#endif
