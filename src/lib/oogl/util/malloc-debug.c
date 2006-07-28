/* GNAH. Nothing more to be said ... */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define N_RECORDS 10000

#define REC_FREE 0

struct alloc_record {
  void *ptr;
  size_t size;
  unsigned long seq;
  const char *file;
  const char *func;
  int line;
};

static struct alloc_record records[N_RECORDS];

unsigned long malloc_seq;
int n_alloc;

static void record_alloc(void *ptr, size_t size,
			 const char *file, const char *func, int line)
{
  unsigned long seq_min;
  int i, seq_min_i = 0;

  for (seq_min = ~0, i = 0; i < N_RECORDS; i++) {
    if (records[i].seq == REC_FREE) {
      seq_min_i = i;
      break;
    } else if (records[i].seq < seq_min) {
      seq_min = records[i].seq;
      seq_min_i = i;
    }
  }

  records[seq_min_i].seq  = ++malloc_seq;
  records[seq_min_i].ptr  = ptr;
  records[seq_min_i].size = size;
  records[seq_min_i].file = file;
  records[seq_min_i].func = func;
  records[seq_min_i].line = line;

  ++n_alloc;
}

static void record_free(void *ptr)
{
  int i;

  for (i = 0; i < N_RECORDS; i++) {
    if (ptr == records[i].ptr) {
      memset(&records[i], 0, sizeof(records[i]));
      records[i].seq = REC_FREE;
      --n_alloc;
      break;
    }
  }
}

void *malloc_record(size_t size, const char *file, const char *func, int line)
{
  void *ptr;

  ptr = malloc(size);

  record_alloc(ptr, size, file, func, line);

  return ptr;
}

void free_record(void *ptr)
{
  record_free(ptr);

  /* it is not an error if we do not find the ptr in our list; we only
   * store the last N_RECORDS allocations
   */
  free(ptr);
}

void *realloc_record(void *ptr, size_t size,
		     const char *file, const char *func, int line)
{
  record_free(ptr);
  ptr = realloc(ptr, size);
  record_alloc(ptr, size, file, func, line);

  return ptr;
}

void *calloc_record(size_t nmemb, size_t size,
		     const char *file, const char *func, int line)
{
  void *ptr;
  size *= nmemb;

  ptr = malloc_record(size, file, func, line);
  memset(ptr, 0, size);
  return ptr; 
}

static int seq_cmp(const void *_a, const void *_b)
{
  const struct alloc_record *a = _a, *b = _b;

  if (a->seq < b->seq) {
    return 1;
  }
  if (a->seq > b->seq) {
    return -1;
  }
  return 0;
}

void print_alloc_records(void)
{
  int i;

  qsort(records, N_RECORDS, sizeof(struct alloc_record), seq_cmp);
  
  for (i = 0; i < N_RECORDS; i++) {
    if (records[i].seq == REC_FREE) {
      break;
    }
    fprintf(stderr, "%ld: %d@%p (%s, %s(), %d)\n",
	    records[i].seq,
	    (int)records[i].size,
	    records[i].ptr,
	    records[i].file,
	    records[i].func,
	    records[i].line);
  }
  fprintf(stderr, "#records: %d\n", i);
}
