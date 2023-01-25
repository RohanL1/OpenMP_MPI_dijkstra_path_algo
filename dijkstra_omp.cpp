/* assert */
#include <assert.h>
/* INFINITY */
#include <math.h>
/* FILE, fopen, fclose, fscanf, rewind */
#include <stdio.h>
/* EXIT_SUCCESS, malloc, calloc, free */
#include <stdlib.h>
/* time, CLOCKS_PER_SEC */
#include <time.h>
//OpenMP header
#include <omp.h>

int GLOBAL_THREADS=1;

#define ROWMJR(R,C,NR,NC) (R*NC+C)
#define COLMJR(R,C,NR,NC) (C*NR+R)
/* define access directions for matrices */
#define a(R,C) a[ROWMJR(R,C,ln,n)]
#define b(R,C) b[ROWMJR(R,C,nn,n)]

static void
load(
  const char * const filename,
  int * const np,
  float ** const ap
)
{
  int i, j, n, ret;
  FILE * fp=NULL;
  float * a;

  /* open the file */
  fp = fopen(filename, "r");
  assert(fp);

  /* get the number of nodes in the graph */
  ret = fscanf(fp, "%d", &n);
  assert(1 == ret);

  /* allocate memory for local values */
  a = (float*) malloc(n*n*sizeof(*a));
  assert(a);

  /* read in roots local values */
  for (i=0; i<n; ++i) {
    for (j=0; j<n; ++j) {
      ret = fscanf(fp, "%f", &a(i,j));
      assert(1 == ret);
    }
  }

  /* close file */
  ret = fclose(fp);
  assert(!ret);

  /* record output values */
  *np = n;
  *ap = a;
}

static void
dijkstra(
  const int s,
  const int n,
  const float * const a,
  float ** const lp
)
{
  int i, j;
  struct float_int {
    float l;
    int u;
  } min;
  char * m;
  float * l;
  float_int *thread_mins;

  int THREADS=omp_get_max_threads();
  int thread_id=-1;
  thread_mins = (float_int *) malloc(sizeof(float_int)*(THREADS));


  m = (char*) calloc(n, sizeof(*m));
  assert(m);

  l = (float*) malloc(n*sizeof(*l));
  assert(l);

  for (i=0; i<n; ++i) {
    l[i] = a(i,s);
  }

  m[s] = 1;
  min.u = -1; /* avoid compiler warning */

  for (i=1; i<n; ++i) {
    min.l = INFINITY;


    /* find local minimum */
    #pragma omp parallel private(thread_id)
    {
      thread_id=omp_get_thread_num();
      #pragma omp single
      {
        GLOBAL_THREADS=omp_get_num_threads();
      }

      #pragma omp for
      for (int x = 0; x < THREADS; ++x) {
        thread_mins[x].u = -1;
        thread_mins[x].l = INFINITY;
      }

      #pragma omp barrier

      #pragma omp for
      for (j=0; j<n; ++j) {
        if (!m[j] && l[j] < thread_mins[thread_id].l) {
          thread_mins[thread_id].l = l[j];
          thread_mins[thread_id].u = j;
        }
      }

      #pragma omp barrier

      #pragma omp single
      {
        for (int x = 0; x < THREADS; ++x) {
          if (thread_mins[x].l < min.l){
            min.l=thread_mins[x].l;
            min.u=thread_mins[x].u;
          }
        }

        m[min.u] = 1;

      }

      #pragma omp barrier

      #pragma omp for
      for (j=0; j<n; ++j) {
      if (!m[j] && min.l+a(j,min.u) < l[j])
        l[j] = min.l+a(j,min.u);
      }
    }
  }

  free(m);

  *lp = l;
}

static void
print_time(const double seconds)
{
  printf("Search Time: %0.06fs\n", seconds);
}

static void
print_numbers(
  const char * const filename,
  const int n,
  const float * const numbers)
{
  int i;
  FILE * fout;

  /* open file */
  if(NULL == (fout = fopen(filename, "w"))) {
    fprintf(stderr, "error opening '%s'\n", filename);
    abort();
  }

  /* write numbers to fout */
  for(i=0; i<n; ++i) {
    fprintf(fout, "%10.4f\n", numbers[i]);
  }

  fclose(fout);
}

int
main(int argc, char ** argv)
{
  int n;
  clock_t ts, te;
  float * a, * l;

  if(argc < 4){
     printf("Invalid number of arguments.\nUsage: dijkstra <graph> <num_sources> <nthreads> [<output_file>].\n");
     return EXIT_FAILURE;
  }
  /* initialize random seed: */
  srand (time(NULL));
  unsigned int seed = time(NULL);

  /* figure out number of random sources to search from */
  int nsources = atoi(argv[2]);
  assert(nsources > 0);
  int nthreads = atoi(argv[3]);

  printf("#IN OMP ALGO#\n");
  /* load data */
  printf("Loading graph from %s.\n", argv[1]);
  load(argv[1], &n, &a);

  omp_set_num_threads(nthreads);


  printf("Performing %d searches from random sources.\n", nsources);
  ts = omp_get_wtime();
  for(int i=0; i < nsources; ++i){
    dijkstra(rand_r(&seed) % n, n, a, &l);
  }
  te = omp_get_wtime();
  double time =(double)(te-ts);
  print_time(time);
  if(argc > 4){
    printf("Computing result for source 0.\n");
    dijkstra(0, n, a, &l);
    printf("Writing result to %s.\n", argv[4]);
    print_numbers(argv[4], n, l);
  }
  printf("#STAT#input_file_name,no_searchs,out_file_name,Threads,Time_sec\n%s,%d,%s,%d,%0.06f\n", argv[1],nsources,argv[4],GLOBAL_THREADS,time);

  free(a);
  free(l);

  return EXIT_SUCCESS;
}