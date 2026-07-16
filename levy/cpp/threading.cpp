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

#include "threading.h"

// The thread function must return void* and take a void* argument
static void* print_message(void* arg) {
    char* message = (char*)arg;
    printf("%s\n", message);
    return NULL;
}

void Thread::start()
{
    pthread_t thread_id; // Variable to hold the thread identifier
    const char* msg = "Hello from the custom thread!";
    int result;

    // 1. Create the thread
    result = pthread_create(&thread_id, NULL, print_message, (void*)msg);
    
    // Check if the thread was created successfully
    if (result != 0) {
        perror("Thread creation failed");
        return;
    }

    printf("Hello from the main thread!\n");

    // 2. Wait for the custom thread to finish execution
    pthread_join(thread_id, NULL);

    printf("Custom thread has finished.\n");
}
