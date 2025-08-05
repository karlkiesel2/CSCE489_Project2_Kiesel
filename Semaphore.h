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
	int count; // The current count of the semaphore
	pthread_mutex_t mutex; // Mutex to protect the semaphore
	pthread_cond_t cond; // Condition variable for signaling
};

#endif
