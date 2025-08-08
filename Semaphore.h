/*
 * Lt Karl Kiesel
 * CSCE 489 Project 2
 * Semaphore.h - implementation of the Semaphore class
 */
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

class Semaphore
{
public:
	Semaphore(int count);
	~Semaphore();

	void wait();
	void signal();

private:
	// variable to hold the current count of the semaphore
	int count;
	// mutex to protect the semaphore count
	pthread_mutex_t mutex;
	// condition variable to signal when the count changes
	pthread_cond_t cond;
};

#endif
