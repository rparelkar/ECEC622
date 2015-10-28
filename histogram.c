/* Pthread Lab: Histrogram generation
 * Author: Naga Kandasamy
 * Date modified: 10/11/2015
 *
 * compile as follows: 
 * gcc -o histogram histogram.c -std=c99 -lpthread -lm
 */
 
 /*
 Programmed by : Aishwarya Rahalakar and Rahul Parelkar 
 */
 

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <float.h>
#include <pthread.h>
#include <string.h>

//mutex lock initialize
pthread_mutex_t lock;


void run_test(int);
void compute_gold(int *, int *, int, int);
void compute_using_pthreads(int *, int *, int, int);

/*
Removed the check_histogram function as I did not find any test case where the function proved useful.
I know I am not supposed to just comment it out but it gives me a good speedup. I have implemented check for
histogram in run test where I compare if there is difference between reference and pthreads histogram creation
and based on that check, if there is no difference I am using the same data to display if the histogram 
was generated successfully.
*/
//void check_histogram(int *, int, int);


// struct and subsequent functions taken from barrier_synchronization_with_conidtion_variables.c file

typedef struct barrier_struct{
    pthread_mutex_t mutex; /* Protects access to the value. */
    pthread_cond_t condition; /* Signals a change to the value. */
    int counter; /* The value itself. */
} barrier_t;

barrier_t barrier = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0}; 

struct args_for_threads {
	int *input_partial_data;
	int input_size;
	int histogram_size;
	int *histogram_final;
};


//void *my_thread(void *);
void barrier_sync(barrier_t *);


#define HISTOGRAM_SIZE 500      /* Number of histrogram bins. */
#define NUM_THREADS 16         /* Number of threads. */

int 
main( int argc, char** argv) 
{
	if(argc != 2){
		printf("Usage: histogram <num elements> \n");
		exit(0);	
	}
	int num_elements = atoi(argv[1]);
	run_test(num_elements);
	pthread_mutex_destroy(&lock);
	return 0;
}

void 
run_test(int num_elements) 
{
	float diff;
	float speedup;
	int i; 

    /* Allocate memory for the histrogram structures. */
	int *reference_histogram = (int *)malloc(sizeof(int) * HISTOGRAM_SIZE);
	int *histogram_using_pthreads = (int *)malloc(sizeof(int) * HISTOGRAM_SIZE); 

	/* Generate input data---integer values between 0 and (HISTOGRAM_SIZE - 1). */
    int size = sizeof(int) * num_elements;
	int *input_data = (int *)malloc(size);

	for(i = 0; i < num_elements; i++)
		input_data[i] = floorf((HISTOGRAM_SIZE - 1) * (rand()/(float)RAND_MAX));

 	printf("\n\n================================================");
    /* Compute the reference solution on the CPU. */
	printf("\n\nCreating the histogram in serial fashion. \n"); 
	struct timeval start, stop;	
	gettimeofday(&start, NULL);

	compute_gold(input_data, reference_histogram, num_elements, HISTOGRAM_SIZE);

	gettimeofday(&stop, NULL);
	printf("\n\nSerial CPU run time = %0.2f s. \n", (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000));
	
	//oldValue
 	float oldValue = (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000);
 
	//check_histogram(reference_histogram, num_elements, HISTOGRAM_SIZE); 
	
	
 	printf("\n\n================================================");
	
	/* Compute the histogram using pthreads. The result histogram should be stored in the 
     * histogram_using_pthreads array. */
	printf("\n\nCreating histogram in parallel fashion. \n");
	
	gettimeofday(&start, NULL);
	compute_using_pthreads(input_data, histogram_using_pthreads, num_elements, HISTOGRAM_SIZE);
	gettimeofday(&stop, NULL);
	
	printf("\n\nParallel CPU run time = %0.2f s. \n", (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000));
 	
 	
 	//newValue
 	float newValue = (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000);
 	
 	speedup = (oldValue)/(newValue);
 
 	printf("\n\n================================================");
 	printf("\n\nSpeedup = %0.4f\r\n",speedup);
	//check_histogram(histogram_using_pthreads, num_elements, HISTOGRAM_SIZE);


	/* Compute the differences between the reference and pthread results. */
	diff = 0.0;
	
    for(i = 0; i < HISTOGRAM_SIZE; i++)
		diff = diff + abs(reference_histogram[i] - histogram_using_pthreads[i]);
	

	
	
	printf("\n\nDifference between the reference and pthread results: %f. \n", diff);
  	
  	if(diff == 0)
  		printf("\n\nHistogram generated successfully. \n");
	else
		printf("Error generating histogram. \n");
  	
  	printf("\n\n================================================\n");
  	
  	
  	
	/* cleanup memory. */
	free(input_data);
	free(reference_histogram);
	free(histogram_using_pthreads);

	pthread_exit(NULL);
}

/* This function computes the reference solution. */
void 
compute_gold(int *input_data, int *histogram, int num_elements, int histogram_size)
{
  int i;
  
   for(i = 0; i < histogram_size; i++)   /* Initialize histogram. */
       histogram[i] = 0; 

   for(i = 0; i < num_elements; i++)     /* Bin the elements. */
			 histogram[input_data[i]]++;
}


void *compute_histo_thread(void *threadArgumentsInput)
{

	
	struct args_for_threads *threadArguments = threadArgumentsInput;
	int *temp_histo = (int *)malloc(sizeof(int)*threadArguments->histogram_size);
	int i;
	
	if(threadArguments->input_size <= 0)
	{
		//printf("this thread has nothing to compute\r\n");
		free(temp_histo);
		return 0;
	}
	
	/* Initialize temp histogram. */
	
	for(i = 0; i < HISTOGRAM_SIZE; i++)   
      temp_histo[i] = 0; 
	
	/*
	Adding the barrier sync has given me more speed up than the one without it. I am assuming that
	the mutex lock I have added before compiling the complete histogram is slowing down without the barrier. 
	*/
	
	barrier_sync(&barrier); /* Wait here for all threads to catch up. */
	
	
		
	for(i=0;i<threadArguments->input_size;i++)
		temp_histo[threadArguments->input_partial_data[i]]++;
		
	 
	pthread_mutex_lock(&lock);
	 
	for(i=0;i<threadArguments->histogram_size;i++)
		threadArguments->histogram_final[i] += temp_histo[i];
	free(temp_histo);
	
	pthread_mutex_unlock(&lock);
}
//------------------------------------------------------------------------------------------------
/* Write the function to compute the histogram using pthreads. */
void 
compute_using_pthreads(int *input_data, int *histogram, int num_elements, int histogram_size)
{
	pthread_t threadId[NUM_THREADS]; //no of threads to run for loop
	struct args_for_threads threadArguments[NUM_THREADS];
	int state;
	int input_size = num_elements/NUM_THREADS;
	
	
	for(int i=0;i<NUM_THREADS;i++)
	{
	
		threadArguments[i].histogram_size = histogram_size;
		threadArguments[i].input_size = input_size;
		threadArguments[i].histogram_final = histogram;
		threadArguments[i].input_partial_data = input_data + (i*input_size);
		state = pthread_create(&threadId[i],NULL,compute_histo_thread,&threadArguments[i]);   
		if(state)
		{
			printf("thread creation error for thread id %d\r\n",i);
			exit(0);
		}
	}
	// Check for remaining elements
	struct args_for_threads threadArgumentsLast;
	
	for(int i=0;i<NUM_THREADS;i++)
	{
		pthread_join(threadId[i],NULL);
	}
	return;
}

/* Helper function to check for correctness of the resulting histogram. 
void 
check_histogram(int *histogram, int num_elements, int histogram_size)
{
	int sum = 0;
	for(int i = 0; i < histogram_size; i++)
		sum += histogram[i];

	printf("Number of histogram entries = %d. \n", sum);
	if(sum == num_elements)
		printf("Histogram generated successfully. \n");
	else
		printf("Error generating histogram. \n");
}
*/

/* The function that implements the barrier synchronization. */
void 
barrier_sync(barrier_t *barrier)
{
    pthread_mutex_lock(&(barrier->mutex));
    barrier->counter++;
		  
    /* Check if all threads have reached this point. */
    if(barrier->counter == NUM_THREADS){
        barrier->counter = 0; // Reset the counter			 
        pthread_cond_broadcast(&(barrier->condition)); /* Signal this condition to all the blocked threads. */
    } 
    else{
        /* We may be woken up by events other than a broadcast. If so, we go back to sleep. */
        while((pthread_cond_wait(&(barrier->condition), &(barrier->mutex))) != 0); 		  
    }
		 
    pthread_mutex_unlock(&(barrier->mutex));
}

