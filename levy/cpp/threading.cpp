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

#include "threading.h"

static  pthread_t thread_id_;

void Thread::start(void *(*routine) (void *), void * data)
{
    if (pthread_create(&thread_id_, NULL, routine, data) != 0) {
        perror("Thread creation failed");
        return;
    }

}
    
void Thread::join()
{
    pthread_join(thread_id_, NULL);
}

void Thread::yield()
{
    sched_yield();
}
