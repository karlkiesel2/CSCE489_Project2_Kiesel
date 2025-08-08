/*
 * Lt Karl Kiesel
 * CSCE 489 Project 2
 * Semaphore.cpp - implementation of the Semaphore class
 */
#include <pthread.h>
#include "Semaphore.h"

/*************************************************************************************
 * Semaphore (constructor) - this should take count and place it into a local variable.
 *						Here you can do any other initialization you may need.
 *
 *    Params:  count - initialization count for the semaphore
 *
 *************************************************************************************/

Semaphore::Semaphore(int count)
{
    // Save the starting count to a local variable
    this->count = count;
    // Initialize the mutex and condition variable
    // Resources are allocated for the mutex and condition variable, so we need to destroy them in the destructor
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

/*************************************************************************************
 * ~Semaphore (destructor) - called when the class is destroyed. Clean up any dynamic
 *						memory.
 *
 *************************************************************************************/

Semaphore::~Semaphore()
{
    // Destroy the mutex and condition variable
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

/*************************************************************************************
 * wait - implement a standard wait Semaphore method here
 *
 *************************************************************************************/

void Semaphore::wait()
{
    // Lock the mutex to ensure multiple threads do not modify the count simultaneously
    pthread_mutex_lock(&mutex);

    // If the count is zero or less, wait for a signal
    // Otherwise, allow the thread to proceed and decrement the count
    while (count <= 0)
    {
        // Wait on the condition variable, releasing the mutex while waiting
        pthread_cond_wait(&cond, &mutex);
    }

    // Decrement the semaphore count
    count--;

    // Unlock the mutex after editing the count
    pthread_mutex_unlock(&mutex);
}

/*************************************************************************************
 * signal - implement a standard signal Semaphore method here
 *
 *************************************************************************************/

void Semaphore::signal()
{
    // Lock the mutex to ensure multiple threads do not modify the count simultaneously
    pthread_mutex_lock(&mutex);

    // Resource is being returned, so increment the semaphore count
    count++;

    // Signal waiting threads that the count has increased
    pthread_cond_signal(&cond);

    // Unlock the mutex after editing the count and signaling waiting threads
    pthread_mutex_unlock(&mutex);
}
