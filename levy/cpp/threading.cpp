#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// The thread function must return void* and take a void* argument
static void* print_message(void* arg) {
    char* message = (char*)arg;
    printf("%s\n", message);
    return NULL;
}

int start_thread()
{
    pthread_t thread_id; // Variable to hold the thread identifier
    const char* msg = "Hello from the custom thread!";
    int result;

    // 1. Create the thread
    result = pthread_create(&thread_id, NULL, print_message, (void*)msg);
    
    // Check if the thread was created successfully
    if (result != 0) {
        perror("Thread creation failed");
        return 1;
    }

    printf("Hello from the main thread!\n");

    // 2. Wait for the custom thread to finish execution
    pthread_join(thread_id, NULL);

    printf("Custom thread has finished. Exiting main program.\n");
    return 0;
}
