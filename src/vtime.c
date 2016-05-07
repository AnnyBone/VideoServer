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
        self->timer_ev = CreateEvent (0, FALSE, FALSE, 0);

        QueryPerformanceFrequency (&self->ticks_per_sec);
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

    return 1e3 * curr_ticks.QuadPart / self->ticks_per_sec.QuadPart;
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

int time_str (double msec, char* buffer, size_t size)
{
    SYSTEMTIME st;
    time_as_systemtime (msec, &st);
    return snprintf (buffer, size, TIME_STR_FORMAT,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

vclock_t* clock_new (void)
{
    vclock_t* self = (vclock_t*) malloc (sizeof (vclock_t));
    if (self) {
        memset (self, 0, sizeof (vclock_t));
        QueryPerformanceCounter (&self->start_ts);
        QueryPerformanceFrequency (&self->ticks_per_sec);
    }
    return self;
}

void clock_destroy (vclock_t** pself)
{
    assert (pself);
    vclock_t* self = *pself;
    if (self) {
        free (self);
        self = 0;
    }
}

void clock_reset (vclock_t* self)
{
    assert (self);
    QueryPerformanceCounter (&self->start_ts);
}

void clock_set_freeze (vclock_t* self, bool freeze)
{
    assert (self);

    if (self->freeze != freeze) {
        self->freeze = freeze;

        if (freeze) {
            QueryPerformanceCounter (&self->freeze_ts);
        }
        else {
            LARGE_INTEGER curr_ticks;
            QueryPerformanceCounter (&curr_ticks);
            LONGLONG leap = curr_ticks.QuadPart - self->freeze_ts.QuadPart;
            self->freeze_acc += leap;
        }
    }
}

bool clock_is_freeze (vclock_t* self)
{
    assert (self);
    return self->freeze;
}

void clock_toggle_freeze (vclock_t* self)
{
    assert (self);
    clock_set_freeze (self, !clock_is_freeze (self));
}

double clock_now (vclock_t* self)
{
    assert (self);

    LARGE_INTEGER curr_ticks;
    QueryPerformanceCounter (&curr_ticks);

    LONGLONG dticks = curr_ticks.QuadPart - self->start_ts.QuadPart;
    LONGLONG freeze_ticks = self->freeze_acc;
    if (self->freeze)
        freeze_ticks = curr_ticks.QuadPart - self->freeze_ts.QuadPart;

    return 1e3 * (dticks - freeze_ticks) / self->ticks_per_sec.QuadPart;
}

