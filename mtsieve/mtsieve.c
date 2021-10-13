/*******************************************************************************
 * Name        : mtsieve.c
 * Author      : Eric Rudzin and Tudor Rus
 * Date        : April 23, 2021
 * Description : Multithreaded Sieve Implementation
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

int total_count = 0;
pthread_mutex_t lock;

typedef struct arg_struct
{
    int start;
    int end;
} thread_args;

bool has_three(int prime){
    int holder = 0;
    int three_count = 0;
    while (prime > 0){
        holder = prime % 10;
        prime = prime / 10;
        if (holder == 3){
            three_count++;
        }
    }
    if(three_count >=2){
      return true;
    }
    else {
    return false;
    }
}

void *segmented_sieve(void *ptr){
    thread_args* targs = (thread_args *)ptr;
    int sub_count = 0;
    int start = targs->start;
    int end = targs->end;
    int sqrt_limit = (int)sqrtf(end);

    //CREATE LOW PRIMES
    bool *low_primes = (bool *) malloc (sizeof (bool) * (sqrt_limit + 1));
    // CREATE HIGH PRIMES
    int length_high = end - start + 1;
    bool *high_primes = (bool *) malloc (sizeof (bool) * length_high);


    for (int i = 0; i <= sqrt_limit; i++){
        low_primes[i] = true;
    }

    for (int p = 2; p*p < sqrt_limit; p++){
        if (low_primes[p] == true){
            for (int j = p+p; j <= sqrt_limit; j+=p){
                low_primes[j] = false;
            }
        }
    }

   
    for (int i = 0; i < length_high; i++){
        high_primes[i] = true;
    }

    for (int p = 2; p < sqrt_limit; p++){
        if (low_primes[p]){
            int i = (int)ceil((double)start/p) * p - start;
            if (start <= p){
                i += p;
            }
            while (i < length_high){
                high_primes[i] = false;
                i += p;
            }
        }
    }

    for (int i = 2; i <= length_high; i++){
        if (high_primes[i] && has_three(i + start)){
            sub_count++;
        }
    }

    int retval;
    if ((retval = pthread_mutex_lock(&lock)) != 0) {
        fprintf(stderr, "Warning: Cannot lock mutex. %s.\n", strerror(retval));
    }
    total_count += sub_count;
    if ((retval = pthread_mutex_unlock(&lock)) != 0) {
        fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n", strerror(retval));
    }
    free(low_primes);
    free(high_primes);
    pthread_exit(NULL);
}

bool check_int(char *arg){
  if(strlen(arg) < 1){
    return false;
  }
  else{
    if(arg[0] == '-'){
      return false;
    }
    else{
      for (int i = 0; i < strlen(arg); i++){
        if(!isdigit(arg[i])){
          return false;
        }
      }
    }
  }
  return true;
}

bool check_overflow(char *arg){
  long long checker;
	if(sscanf(arg,"%lld", &checker)== 1){
		int value=(int)checker;
		if(checker!=(long long)value){
      return false;
    }
    else{
      return true;
    }
  }
  return false;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        fprintf(stderr, "Usage: %s -s <starting value> -e <ending value> -t <num threads>\n", argv[0]);
        return EXIT_FAILURE;
    }
    bool s_flag, e_flag, t_flag = false;
    int s_val, e_val, t_val = 0;

    int opt;

    while ((opt = getopt(argc, argv, ":s:e:t:")) != -1)
    {
        switch (opt)
        {
    case 's':
      s_flag = true;
      if(!check_int(optarg)){
        fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, opt);
        return EXIT_FAILURE;
      }
      if(!check_overflow(optarg)){
        fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", opt);
        return EXIT_FAILURE;
      }
      s_val = atoi(optarg);
      break;
    case 'e':
      e_flag = true;
      if(!check_int(optarg)){
        fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, opt);
        return EXIT_FAILURE;
      }
      if(!check_overflow(optarg)){
        fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", opt);
        return EXIT_FAILURE;
      }
      e_val = atoi(optarg);
    break;
    case 't':
      t_flag = true;
      if(!check_int(optarg)){
        fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n", optarg, opt);
        return EXIT_FAILURE;      
      }
      if(!check_overflow(optarg)){
        fprintf(stderr, "Error: Integer overflow for parameter '-%c'.\n", opt);
        return EXIT_FAILURE;
      }
      t_val = atoi(optarg);
      break;
        case ':':
        fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
        return EXIT_FAILURE;
        case '?':
            if (optopt == 'e' || optopt == 's' || optopt == 't')
            {
                fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
            }
            else if (isprint(optopt))
            {
                fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
            }
            else
            {
                fprintf(stderr, "Error: Unknown option character '\\x%x'.\n",
                        optopt);
            }
            return EXIT_FAILURE;
        } 
    }
if (optind < argc)
  {
    fprintf(stderr, "Error: Non-option argument '%s' supplied.\n", argv[optind]);
    return EXIT_FAILURE;
  }
  if (!s_flag)
  {
    fprintf(stderr, "Error: Required argument <starting value> is missing.\n");
    return EXIT_FAILURE;
  }

  if (s_val < 2)
  {
    fprintf(stderr, "Error: Starting value must be >= 2.\n");
    return EXIT_FAILURE;
  }
  if (!e_flag)
  {
    fprintf(stderr, "Error: Required argument <ending value> is missing.\n");
    return EXIT_FAILURE;
  }
  if (e_val < 2)
  {
    fprintf(stderr, "Error: Ending value must be >= 2.\n");
    return EXIT_FAILURE;
  }
  if (e_val < s_val)
  {
    fprintf(stderr, "Error: Ending value must be >= starting value.\n");
    return EXIT_FAILURE;
  }
  if (!t_flag)
  {
    fprintf(stderr, "Error: Required argument <num threads> is missing.\n");
    return EXIT_FAILURE;
  }
  if (t_val < 1)
  {
    fprintf(stderr, "Error: Number of threads cannot be less than 1.\n");
    return EXIT_FAILURE;
  }
  int available_procs = get_nprocs();
  int available_threads = 2 * available_procs;
  if (t_val > available_threads)
  {
    fprintf(stderr, "Error: Number of threads cannot exceed twice the number of processors(%d).\n", available_threads);
    return EXIT_FAILURE;
  }


  int num_primes = e_val - s_val +1;
  if (t_val > num_primes){
    t_val = num_primes;
  }
    int primes_count = e_val - s_val + 1;
  if (t_val > primes_count){
    t_val = primes_count;
  }
  int primes_per_thread = primes_count/t_val;
  int primes_per_thread_remainder = primes_count%t_val;

  printf("Finding all prime numbers between %d and %d.\n", s_val, e_val);

  if(t_val == 1){
    printf("%d segment:\n", t_val);
  }
  else{
    printf("%d segments:\n", t_val);
  }
 
  int retval;
  if ((retval = pthread_mutex_init(&lock, NULL)) != 0) {
    fprintf(stderr, "Error: Cannot create mutex. %s.\n", strerror(retval));
    return EXIT_FAILURE;
  }
  pthread_t threads[t_val];
  thread_args thread_args[t_val];

  int start_val = s_val;
  int end_val = s_val;
  
  for (int i = 0; i < t_val; i++){
    
    start_val = end_val;
    if (i != 0){
      start_val++;
    }
    if(i == t_val -1){
      end_val = e_val;
    } else{
      end_val = start_val + primes_per_thread -1;
    }
    
   

    // need to check the segments
    
    if (primes_per_thread_remainder != 0){
      end_val++;
      primes_per_thread_remainder--;
    }

    
    printf("   [%d, %d]\n", start_val, end_val);
    thread_args[i].start = start_val;
    thread_args[i].end = end_val;
    int tres;
    if ((tres = pthread_create(&threads[i], NULL, segmented_sieve, (void *)(&thread_args[i]))) != 0) {
      fprintf(stderr, "Error: Cannot create thread %d. %s.\n", i + 1,strerror(errno));
      return EXIT_FAILURE;
    }
  }
  for (int i = 0; i < t_val; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Warning: Thread %d did not join properly.\n", i + 1);
    }
  }
  int retval1;
  if ((retval1 = pthread_mutex_destroy(&lock)) != 0) {
    fprintf(stderr, "Warning: Cannot destroy mutex. %s.\n", strerror(retval));
  }
  printf("Total primes between %d and %d with two or more '%d' digits: %d\n", s_val, e_val, 3, total_count);
  return EXIT_SUCCESS;


}