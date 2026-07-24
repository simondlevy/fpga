/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>

#include "threading.h"

static  pthread_t thread_id_;

void Thread::Start(void *(*routine) (void *), void * data)
{
    if (pthread_create(&thread_id_, NULL, routine, data) != 0) {
        perror("Thread creation failed");
        return;
    }

}
    
void Thread::Join()
{
    pthread_join(thread_id_, NULL);
}

void Thread::Yield()
{
    sched_yield();
}

void Thread::Sleep(const float sec)
{
    const int tv_sec = sec;
    const int tv_nsec = (tv_sec - sec) * 1e9;

    struct timespec req = {};
    req.tv_sec = tv_sec;
    req.tv_nsec = tv_nsec;

    nanosleep(&req, NULL);
}
