/*************************************************************************************
 * babyyoda - used to test your semaphore implementation and can be a starting point for
 *			     your store front implementation
 *
 * Lt Karl Kiesel
 * CSCE 489 Project 2
 *************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "Semaphore.h"

// Semaphores that each thread will have access to as they are global in shared memory
Semaphore *empty = NULL;
Semaphore *full = NULL;

pthread_mutex_t buf_mutex;

// Circular buffer to hold the items produced. We define the size based on user input
int *buffer = NULL;
// Size of buffer initially set at zero, but will be set to user input in main
int buffer_size = 0;
// Number of consumers that will be created based on user input
int num_consumers = 0;
// Keep track of how many items have been consumed
int consumed = 0;
// Number of items to produce, set by user input
int num_produce = 0;
// Index for the next item to produce
int in = 0;
// Index for the next item to consume
int out = 0;

/*************************************************************************************
 * producer_routine - this function is called when the producer thread is created.
 *
 *			Params: data - a void pointer that should point to an integer that indicates
 *							   the total number to be produced
 *
 *			Returns: always NULL
 *
 *************************************************************************************/

void *producer_routine(void *data)
{

	time_t rand_seed;
	srand((unsigned int)time(&rand_seed));

	// The current serial number (incremented)
	unsigned int serialnum = 1;

	// We know the data pointer is an integer that indicates the number to produce
	int left_to_produce = *((int *)data);

	// Loop through the amount we're going to produce and place into the buffer
	while (left_to_produce > 0)
	{
		printf("Producer wants to put Yoda #%d into buffer...\n", serialnum);

		// Semaphore check to make sure there is an available slot
		full->wait();

		// Place item on the next shelf slot by first setting the mutex to protect our buffer vars
		pthread_mutex_lock(&buf_mutex);

		// Place the Yoda in the buffer at the current index
		buffer[in] = serialnum;
		printf("   Producer put Yoda #%d on shelf in slot #%d.\n", serialnum, in + 1);
		// Increment the index and wrap around if we reach the end of the buffer
		in = (in + 1) % buffer_size;
		// Increment the serial number for the next Yoda
		serialnum++;
		// Decrement the number of Yodas left to produce
		left_to_produce--;

		// Unlock the mutex to allow other threads to access the buffer
		pthread_mutex_unlock(&buf_mutex);

		// Semaphore signal that there are items available, and waiting consumers can now consume
		empty->signal();

		// random sleep but he makes them fast so 1/20 of a second
		usleep((useconds_t)(rand() % 200000));
	}

	printf("Producer has finished producing Yodas for the day. Notifying consumers to stop waiting\n");
	// After all items have been produced, we need to signal waiting consumers to exit
	for (int i = 0; i < num_consumers; i++)
	{
		// Send signal to consumers that we are done producing. Same logic as above, but we place a value of -1 in the buffer
		// to indicate that no more items will be produced and consumers should exit
		full->wait();
		printf("Producer informs consumer #%d to stop waiting.\n", i + 1);
		pthread_mutex_lock(&buf_mutex);
		// Write -1 to the buffer to indicate no more items will be produced and consumers should exit
		buffer[in] = -1;
		in = (in + 1) % buffer_size;
		pthread_mutex_unlock(&buf_mutex);
		empty->signal();
	}

	return NULL;
}

/*************************************************************************************
 * consumer_routine - this function is called when the consumer thread is created.
 *
 *       Params: data - a void pointer that should point to a boolean that indicates
 *                      the thread should exit. Doesn't work so don't worry about it
 *
 *       Returns: always NULL
 *
 *************************************************************************************/

void *consumer_routine(void *data)
{
	// We know the data pointer is an integer that indicates the number of consumers
	(void) data;

	// Consumers will continue until all items are consumed
	while (consumed < num_produce)
	{

		printf("Consumer wants to buy a Yoda...\n");

		// Semaphore to see if there are any items to take
		empty->wait();

		// Lock the mutex to safely access the buffer
		pthread_mutex_lock(&buf_mutex);

		// Grab the item from the buffer at the current index
		int item = buffer[out];

		// If the item is -1, it means the producer has signaled that no more items will be produced, so consumer should exit
		if (item == -1)
		{
			// Signal for exit
			pthread_mutex_unlock(&buf_mutex);
			full->signal();
			// Break out of loop
			printf("Consumer informed that all Yodas have been purchased.\n");
			break;
		}
		// If we reach here, we have a valid item to consume
		// Make sure the buffer slot is not empty
		else if (item != 0)
		{
			printf("   Consumer bought Yoda #%d from shelf slot #%d.\n", item, out + 1);
			// Mark slot as empty
			buffer[out] = 0;
			// Increment the out index and wrap around if we reach the end of the buffer
			out = (out + 1) % buffer_size;
			// Increment the count of consumed items
			consumed++;
		}

		// Unlock the mutex to allow other threads to access the buffer
		pthread_mutex_unlock(&buf_mutex);

		// Consumers wait up to one second BEFORE signaling that they have taken an item
		usleep((useconds_t)(rand() % 1000000));

		// Signal that an item has been consumed, and waiting producers can now produce again
		full->signal();

		// Yoda is taken off shelf, now shelf space is available. Sleep for a second to allow producers to restock and other consumers to buy
		sleep(1);
	}
	// After consuming all items, consumer(s) goes home
	printf("Consumer goes home.\n");

	return NULL;
}

/*************************************************************************************
 * main - Standard C main function for our storefront.
 *
 *		Expected params: pctest <buffer_size> <num_consumers> <max_items>
 *				max_items - how many items will be produced before the shopkeeper closes
 *
 *************************************************************************************/

int main(int argv, const char *argc[])
{

	// Get our argument parameters
	if (argv != 4)
	{
		printf("Invalid parameters. Format: %s <buffer_size> <num_consumers> <max_items>\n", argc[0]);
		exit(0);
	}

	// User input on the size of the buffer, number of consumers, and number of items to produce
	num_produce = (unsigned int)strtol(argc[3], NULL, 10);
	buffer_size = (unsigned int)strtol(argc[1], NULL, 10);
	num_consumers = (unsigned int)strtol(argc[2], NULL, 10);

	// Allocate buffer (shelf) size based on user input
	buffer = new int[buffer_size];

	printf("Producing %d Yodas today with a shelf size of %d for %d consumers.\n", num_produce, buffer_size, num_consumers);

	// Initialize our semaphores
	empty = new Semaphore(0);
	full = new Semaphore(buffer_size);

	// Initialize the buffer mutex, used to protect access to the buffer
	pthread_mutex_init(&buf_mutex, NULL);

	// single producer thread and multiple consumer threads
	pthread_t producer;
	pthread_t *consumers = new pthread_t[num_consumers];

	// Launch our producer thread
	pthread_create(&producer, NULL, producer_routine, (void *)&num_produce);

	// Launch our consumer threads, based on user input
	for (int i = 0; i < num_consumers; i++)
	{
		pthread_create(&consumers[i], NULL, consumer_routine, NULL);
	}
	// Wait for our producer thread to finish up
	pthread_join(producer, NULL);

	printf("The manufacturer has completed his work for the day.\n");
	printf("Waiting for consumers to buy up the rest.\n");

	// Give the consumers a second to finish snatching up items
	while (consumed < num_produce)
		sleep(1);

	// Now join all consumer threads
	for (int i = 0; i < num_consumers; i++)
	{
		pthread_join(consumers[i], NULL);
	}

	// We are exiting, clean up all dynamic memory
	delete empty;
	delete full;
	pthread_mutex_destroy(&buf_mutex);
	delete[] buffer;
	delete[] consumers;

	printf("Producer/Consumer simulation complete!\n");
}
