#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define MAX_N 100000000
#define MAX_THREADS 25

int primes[MAX_N+1];
int nextBottom;
pthread_mutex_t nextBottomLock = PTHREAD_MUTEX_INITIALIZER;
pthread_t id[MAX_THREADS];
int n;

void crossout(int k) {
   for (int i = 3; i*k <= n; i += 2)  {
      primes[i*k] = 0;
   }
}

void *worker(int threadNumber) {
  int top = n;
  int bottom = 0;
  int counted = 0;

  while (bottom <= top) {
    pthread_mutex_lock(&nextBottomLock);
    bottom = nextBottom;
    nextBottom += 2;
    pthread_mutex_unlock(&nextBottomLock);

    if (primes[bottom] == 1)  {
      crossout(bottom);
      counted++;
    }
  }

  return counted;
}

main(int argc, char **argv) {
  n = atoi(argv[1]);
  int nthreads = atoi(argv[2]);
  int i = 0;

  for (int i = 3; i <= n; i++)  {
    primes[i] = i%2 != 0;
  }

  nextBottom = 3;
  for (i = 0; i < nthreads; i++)  {
    pthread_create(&id[i], NULL, worker, i);
  }

  int counted;
  for (int j = 0; j < nthreads; j++)  {
    pthread_join(id[j], &counted);
    printf("thread %d processed %d numbers\n", j + 1, counted);
  }

  int primesFound = 1;
  for (i = 3; i <= n; i++) {
    if (primes[i])  {
      primesFound++;
    }
  }

  printf("total number of primes found %d\n", primesFound);
}
