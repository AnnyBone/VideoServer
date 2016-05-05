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

double time_now (vtime_t* self)
{
    assert (self);

    LARGE_INTEGER curr_ticks;
    QueryPerformanceCounter (&curr_ticks);

    LONGLONG dticks = curr_ticks.QuadPart - self->start_ticks.QuadPart;
    return 1e3 * dticks / self->ticks_per_sec.QuadPart;
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
