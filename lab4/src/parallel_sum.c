#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>

#include <pthread.h>
#include "utils.h"

struct SumArgs {
  int *array;
  int begin;
  int end;
};

int Sum(const struct SumArgs *args) {
  int sum = 0;
  for(int i = args->begin;i<args->end;i++){
    sum += args->array[i];
  } 
  return sum;
}

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {
           
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;
  pthread_t threads[threads_num];
  while(true){
    int current_optind = optind ? optind : 1;
    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"threads_num", required_argument,0 , 0},
                                      {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "f",options, &option_index);
    if (c ==- 1) break;
    switch (c){
      case 0:
        switch(option_index){
          case 0:
            seed = atoi(optarg);
            break;
          case 1:
            array_size = atoi(optarg);
            break;
          case 2:
            threads_num = atoi(optarg);
            break;
        }
      case 'f':
        break;
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }
  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  } 

  if (seed == 0 || array_size == 0 || threads_num == 0) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --threads_num \"num\" \n",
           argv[0]);
    return 1;
  }
  
  
  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  /*for(int i = 0; i < array_size; i++){
    printf("Arr[%d]: %d\n", i, array[i]);
  }*/
  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  
  struct SumArgs args[threads_num];
  int chunk_size = array_size/threads_num;
  
  for (uint32_t i = 0; i < threads_num; i++){
    int start_index = i*chunk_size;
    int end_index = (i == threads_num - 1) ? array_size : (i+1) * chunk_size;
    args[i].array = array;
    args[i].begin = start_index;
    args[i].end = end_index;
    //printf("the start_index: %d the end_index: %d\n", args[i].begin, args[i].end);
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])){
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int sum = 0;
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);
  
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
  
  free(array);
  
  printf("Elapsed time: %fms\n", elapsed_time);
  printf("Total: %d\n", total_sum);
  
  return 0;
}
