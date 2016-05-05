#include <vtime.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

vtime_t* time_new (void)
{
    vtime_t* self = (vtime_t*) malloc (sizeof (vtime_t));
    if (self) {
        memset (self, 0, sizeof (vtime_t));

        timeGetDevCaps (&self->tc, sizeof (TIMECAPS));
        timeBeginPeriod (self->tc.wPeriodMin);

        QueryPerformanceFrequency (&self->ticks_per_sec);
        QueryPerformanceCounter (&self->start_ticks);

        self->frozen = false;
        self->freeze_ticks_acc = 0;
        self->freeze_ticks_ts.QuadPart = 0;
    }

    return self;
}

void time_destroy (vtime_t** pself)
{
    assert (pself);

    vtime_t* self = *pself;
    if (self) {

        timeKillEvent (self->timer_id);
        timeEndPeriod (self->tc.wPeriodMin);
        CloseHandle (self->timer_ev);

        free (self);
        self = 0;
    }
}

void time_reset (vtime_t* self)
{
    assert (self);
    QueryPerformanceCounter (&self->start_ticks);
}



void time_freeze (vtime_t* self, bool should_freeze)
{
    assert (self);

    if (self->frozen == should_freeze)
        return;

    self->frozen = should_freeze;

    if (should_freeze) {
        QueryPerformanceCounter (&self->freeze_ticks_ts);
    }
    else {
        LARGE_INTEGER curr_ticks;
        QueryPerformanceCounter (&curr_ticks);
        LONGLONG leap = curr_ticks.QuadPart - self->freeze_ticks_ts.QuadPart;
        self->freeze_ticks_acc += leap;
    }
}

LONGLONG s_frozen_ticks (vtime_t* self)
{
    if (!self->frozen)
        return self->freeze_ticks_acc;

    LARGE_INTEGER curr_ticks;
    QueryPerformanceCounter (&curr_ticks);
    LONGLONG curr_leap = curr_ticks.QuadPart - self->freeze_ticks_ts.QuadPart;
    return self->freeze_ticks_acc + curr_leap;
}

double time_now (vtime_t* self)
{
    assert (self);

    LARGE_INTEGER curr_ticks;
    QueryPerformanceCounter (&curr_ticks);

    LONGLONG dticks = curr_ticks.QuadPart - self->start_ticks.QuadPart;
    return 1e3 * dticks / self->ticks_per_sec.QuadPart;
}

double time_frozen (vtime_t* self)
{
    assert (self);

    return 1e3 * s_frozen_ticks (self) / self->ticks_per_sec.QuadPart;
}

double time_wait (vtime_t* self, double time_to_wait)
{
    assert (self);

    double eps = 1e-6;
    if (time_to_wait < eps)
        return 0.;

    double before = time_now (self);

    // wait integral msec part on the timer
    UINT itime_to_wait = (UINT) time_to_wait;
    if (itime_to_wait > 0) {
        self->timer_id = timeSetEvent (itime_to_wait, self->tc.wPeriodMin,
                (LPTIMECALLBACK)self->timer_ev, 0,
                TIME_CALLBACK_EVENT_SET|TIME_ONESHOT);
        WaitForSingleObject (self->timer_ev, INFINITE);
    }

    // spin-wait
    double after;
    while ((after = time_now (self)) < before + time_to_wait);
    return after - before;
}


void time_as_systemtime (double msec, SYSTEMTIME* st)
{
    FILETIME ft;
    ULARGE_INTEGER t;
    t.QuadPart = (ULONGLONG)(msec * 1e4 + .5);
    ft.dwLowDateTime = t.LowPart;
    ft.dwHighDateTime = t.HighPart;
    FileTimeToSystemTime (&ft, st);
}
