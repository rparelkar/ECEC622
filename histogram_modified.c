/* Pthread Lab: Histrogram generation
 * Author: Naga Kandasamy
 * Date modified: 10/11/2015
 *
 * compile as follows: 
 * gcc -o histogram histogram.c -std=c99 -lpthread -lm
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <float.h>
#include <pthread.h>
#include <unistd.h>
void run_test(int);
void compute_gold(int *, int *, int, int);
void compute_using_pthreads(int *, int *, int, int);
void check_histogram(int *, int, int);

void *create_local_hist(void*);
pthread_barrier_t barr;
 
int **histogram_local;
int *input_data;
typedef struct thread_args_t{
  int thread_id;
  int start;
  int end;
}args_threads;

args_threads * thread_args;

#define HISTOGRAM_SIZE 500      /* Number of histrogram bins. */
#define NUM_THREADS 16           /* Number of threads. */
pthread_t mythread[NUM_THREADS];
int 
main( int argc, char** argv) 
{
	if(argc != 2){
		printf("Usage: histogram <num elements> \n");
		exit(0);	
	}
	int num_elements = atoi(argv[1]);
	run_test(num_elements);
	return 0;
}

void 
run_test(int num_elements) 
{
	float diff;
	int i; 

    /* Allocate memory for the histrogram structures. */
	int *reference_histogram = (int *)malloc(sizeof(int) * HISTOGRAM_SIZE);
	int *histogram_using_pthreads = (int *)malloc(sizeof(int) * HISTOGRAM_SIZE); 

	/* Generate input data---integer values between 0 and (HISTOGRAM_SIZE - 1). */
    int size = sizeof(int) * num_elements;
	input_data = (int *)malloc(size);

	for(i = 0; i < num_elements; i++)
		input_data[i] = floorf((HISTOGRAM_SIZE - 1) * (rand()/(float)RAND_MAX));

    /* Compute the reference solution on the CPU. */
	printf("Creating the reference histogram. \n"); 
	struct timeval start, stop;	
	gettimeofday(&start, NULL);

	compute_gold(input_data, reference_histogram, num_elements, HISTOGRAM_SIZE);

	gettimeofday(&stop, NULL);
	printf("CPU run time = %0.4f s. \n", (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000));
	check_histogram(reference_histogram, num_elements, HISTOGRAM_SIZE); 
	float old_value=(float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000);
	/* Compute the histogram using pthreads. The result histogram should be stored in the 
     * histogram_using_pthreads array. */
	printf("Creating histogram using pthreads. \n");
	gettimeofday(&start, NULL);
	
	compute_using_pthreads(input_data, histogram_using_pthreads, num_elements, HISTOGRAM_SIZE);
	
	gettimeofday(&stop, NULL);
        printf("\nCPU run time for pthreads = %0.4f s. \n", (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000));
        float new_value=(float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000);
	 check_histogram(histogram_using_pthreads, num_elements, HISTOGRAM_SIZE); 
         
	//sleep(1);
	/* Compute the differences between the reference and pthread results. */
	diff = 0.0;
    for(i = 0; i < HISTOGRAM_SIZE; i++)
		diff = diff + abs(reference_histogram[i] - histogram_local[0][i]);

	printf("Difference between the reference and pthread results: %f. \n", diff);
        float speedup = old_value/new_value;
        printf("Speedup = %0.4f \n",speedup);
	/* cleanup memory. */
	free(input_data);
	free(reference_histogram);
	free(histogram_using_pthreads);

	
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

void *create_local_hist(void *args)
{
  args_threads *thread_args = (args_threads*) args;
// printf("\nEntering create local hist thread id %d",thread_args->thread_id);
  int i,j;
  int sum;
  int temp = thread_args->thread_id;
  sum = 0;

//printf("For thread_id:%d , Start:%d , End:%d /n",temp,thread_args->start,thread_args->end);
  for(i=thread_args->start;i<=(thread_args->end);i++){
//    	printf("\n Changing local hist of thread %d",thread_args->thread_id);
	histogram_local[temp][input_data[i]]++;
	}
    pthread_barrier_wait(&barr);      

for(i=0;i<NUM_THREADS;i++)
 {
    for(j=0;j<HISTOGRAM_SIZE;j++)

    {
       sum += 1;
    }
        //printf("\nsum for thread_id %d = %d",i,sum);
        //sum=0;
}

/*    for(int i=0;i<HISTOGRAM_SIZE;i++ )
     {
       sum+= histogram_local[temp][i];
     }
     printf("\nsum of thread_id %d = %d",temp,sum);
*/
//sleep(15);
if(temp==0)
{
pthread_exit(NULL);
}     
for(int j=0;j<HISTOGRAM_SIZE;j++)
     {
        histogram_local[0][j] += histogram_local[temp][j];
	  }
/*for(int i=0;i<HISTOGRAM_SIZE;i++ )
     {
       sum+= histogram_local[0][i];
     }
printf("Sum of all bins = %d \n ",sum);
*/
sleep(2);
pthread_barrier_wait(&barr);  
pthread_exit(NULL);
}
      

/* Write the function to compute the histogram using pthreads. */
void 
compute_using_pthreads(int *input_data, int *histogram, int num_elements, int histogram_size)
{
//no of threads to run for loop                                                                                                                  
  int i;
  int j;
//  args_threads * thread_args;
  int data_size = num_elements/NUM_THREADS;;
   //printf("\n Creating histogram table!");
   histogram_local = (int**)malloc(sizeof(int*)*NUM_THREADS);
     for(int i=0;i<NUM_THREADS;i++){
        histogram_local[i] = (int*)malloc(sizeof(int)*HISTOGRAM_SIZE);
   }
   //printf("\n Histogram table created!");

   for(int i=0;i<NUM_THREADS;i++){
        for(int j=0;j<HISTOGRAM_SIZE;j++){
           histogram_local[i][j]=0;
           }
         }
    pthread_barrier_init(&barr,NULL,NUM_THREADS);

    for(i=0;i<NUM_THREADS;i++)
{
   //printf("\n Mallocing for struct!");
    thread_args = (args_threads*)malloc(sizeof(args_threads));
  //printf("\n Malloc done for struct!");
    thread_args->thread_id=i;
    thread_args->start=i*data_size;
    if(i==NUM_THREADS-1)
        thread_args->end=num_elements-1;
    else
        thread_args->end=((i+1)*data_size)-1;
   //printf("\n Creating thread %d",thread_args->thread_id);
   if((pthread_create(&mythread[i], NULL, create_local_hist, (void*)thread_args))!=0)
 {
    printf ("thread not created \n");
    exit(0);
  }
}
/*for(i=0;i<NUM_THREADS;i++)
	{
		pthread_join(mythread[i],NULL);
	}
*/
pthread_join(mythread[0],NULL);

}

/* Helper function to check for correctness of the resulting histogram. */
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




