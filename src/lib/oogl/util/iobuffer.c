enum bufferstate {
  IOBNoMark      = 0, /**< Internal buffer list is inactive */
  IOBMarkSet     = 1, /**< Mark is set, read through buffer */
  IOBMarkCleared = 2, /**< Mark is cleared, but still data im buffer */
  IOBVanilla     = 3, /**< Plain stdio, buffereing not necesary */
};

#define BUFFER_SIZE (1 << 16)

typedef struct IOBuffer
{
  struct IOBuffer *next;
  char buffer[BUFFER_SIZE];
} IOBuffer;

typedef struct BufferFILE
{
  FILE     *istream;  /* The underlying stdio FILE */
  int      state;
  IOBuffer *buf_head; /**< Head of linked list of buffers */
  IOBuffer *buf_tail;
  IOBuffer *buf_ptr;  /**< Pointer to tail of buffer list */
  size_t   buf_pos;   /**< Byte pos into buf_ptr */
  size_t   tail_size;  /**< Size used inside *buf_ptr */
  size_t   tot_pos;
  size_t   tot_size;
} IOBFILE;

static size_t
bfread_buffer(void *ptr, size_t size, size_t nmemb, IOBFILE *bstream)
{
  size_t tot_space;
  size_t n_records;
  char *buf = ptr;

  tot_space = bstream->tot_size - bstream->tot_pos;
  n_records = tot_space / size;
  if (n_records > nmemb) {
    n_records = nmemb;
  }
  rq_sz = n_records * size;
  while (rq_sz) {
    rq_sz_pos = min(rq_sz, BUFFER_SIZE - bstream->buf_pos);
    memcpy(buf, bstream->buf_ptr->buffer + bstream->buf_pos, rq_sz_pos);
    bstream->buf_pos += rq_sz_pos;
    bstream->tot_pos += rq_sz_pos;
    buf              += rq_sz_pos;
    rq_sz            -= rq_sz_pos;
    if (rq_sz && bsteam->buf_pos == BUFFER_SIZE) {
      /* advance to next buffer */
      bstream->buf_ptr = bstream->buf_ptr->next;
      bstream->buf_pos = 0;
    }
  }
  return n_records;
}

size_t bfread(void *ptr, size_t size, size_t nmemb, IOBFILE *bstream)
{
  size_t n_records;
  size_t rq_size = size * nmemb;
  size_t tot_space;

  switch (bstream->state) {
  case IOBVanilla:
  case IOBNoMark:
    /** Just forward to stdio */
    return fread(ptr, size, nmemb, bstream->istream);
  case IOBMarkCleared:
    /* Empty internal buffer, then set IOBNoMark and continue with
     * normal stdio stuff.
     */
    n_records = bfread_buffer(ptr, size, nmemb, bstream);
    if (n_records < nmemb) {
      /* handle partial copy, then cleanup */
      size_t space_left = bstream->tot_size - bstream->tot_pos;
      char *buf = ptr;

      buf += n_records * size;
      bfread_buffer(buf, 1, space_left, bstream);
      bstream_release_buffer(bstream);
      bstream->state = IOBNoMark;
      buf   += space_left;
      rq_sz  = size - space_left;
      if (fread(buf, 1, rq_sz, bstream->istream) != rq_size) {
	return n_records;
      } else {
	n_records ++;
	buf += rq_sz;
      }
      return n_records + fread(buf, size, nmemb - n_records, bstream->istream);
    }
  case IOBMarkSet:
    /* Read through buffer, we assume that there is at least one
     * IOBuffer already allocated when we reach here, so buf_tail does
     * not point into Nirwana.
     */
    tot_space   = bstream->tot_size - bstream->tot_pos;
    rq_size     = size * nmemb;
    size_needed = rq_size - tot_space;
    while (size_needed) {
      size_t rq_sz_tail, rd_sz;

      rq_sz_tail = min(size_needed, BUFFER_SIZE - bstream->tail_size);
      if (rq_sz_tail == 0) { /* hit end of buffer */
	bstream->buf_tail->next = malloc(sizeof(IOBuffer));
	bstream->buf_tail       = bstream->buf_tail->next;
	bstream->buf_tail->next = NULL;
	bstream->tail_size      = 0;
	rq_sz_tail = min(size_needed, BUFFER_SIZE - bstream->tail_size);
      }
      rd_sz = fread(bstream->buf_tail->buffer + bstream->tail_size,
		    rq_sz_tail, bstream->file);
      bstream->tail_size += rd_sz;
      bstream->tot_size  += rd_sz;
      size_needed        -= rd_sz;
      if (rd_sz < rq_sz_tail) {
	break;
      }
    }
    return bfread_buffer(ptr, size, nmemb, bstream);
  }
}

int bfgetc(BufferFILE *bstream)
{
  int c;

  switch (bstream->state) {
  case NoMark:
    return fgetc(bstream->istream);
  case MarkCleared:
    if (bstream->buf_space >= 1) {
      c = bstream->buffer[bstream->buf_pos];
      --bstream->buf_space;
      ++bstream->buf_pos;
    }
    if (bstream->buf_space <= 0) {
      free(bstream->buffer);
      bstream->buffer    = NULL;
      bstream->buf_alloc = 0;
      bstream->buf_pos   = 0;
      bstream->buf_size  = 0;
      bstream->buf_space = 0;
      bstream->state     = NoMark;
    }
    return c;
  case MarkSet:
    if (bstream->buf_space < 1) {
      if (bstream->buf_alloc <= bstream->buf_size) {
	bstream->buffer = realloc(bstream->buffer,
				  bstream->alloc,
				  (bstream->buf_size + ALLOC_CHUNK)
				  / ALLOC_CHUNK * ALLOC_CHUNK);
	bstream->buf_alloc =
	  (bstream->buf_pos + ALLOC_CHUNK) / ALLOC_CHUNK * ALLOC_CHUNK;
      }
      c = getc(bstream->istream);
      if (c != EOF) {
	bstream->buffer[bstream->buf_size] = c;
	++bstream->buf_pos;
	++bstream->buf_size;
      }
    } else {
      c = bstream->buffer[bstream->buf_pos];
      ++bstream->buf_pos;
      --bstream->buf_space;
    }
    return c;
  }
}

void bstream_setmark(BufferFILE *bstream)
{
  switch (bstream->state) {
  case MarkSet:
    abort();
  case MarkCleared:
    memcpy(bstream->buffer,
	   bstream->buffer + bstream->buf_pos, bstream->buf_space);
    bstream->buf_pos = 0;
    bstream->buf_size = bstream->buf_space;
    break;
  case NoMark:
    if (bstream->buffer) {
      abort();
    }
    bstream->buf_alloc = 0;
    bstream->buf_pos   = 0;
    bstream->buf_size  = 0;
    bstream->buf_space = 0;
    break;
  }
}

void bstream_seekmark(BufferFILE *bstream)
{
  if (!bstream->buffer || !bstream->MarkSet) {
    abort();
  }
  bstream->buf_pos   = 0;
  bstream->buf_space = bstream->buf_size;
}

void bstream_clearmark(BufferFILE *bstream)
{
  bstream->state = MarkCleared;
}

int bungetc(int c, BufferFILE *bstream)
{
  if (c == EOF) {
    return c;
  }
  switch (bstream->state) {
  case NoMark:
    return ungetc(c, bstream->istream);
  case MarkSet:
    if (

    if (bstream->buf_space < 1) {
      if (bstream->buf_alloc <= bstream->buf_size) {
	bstream->buffer = realloc(bstream->buffer,
				  bstream->alloc,
				  (bstream->buf_size + ALLOC_CHUNK)
				  / ALLOC_CHUNK * ALLOC_CHUNK);
	bstream->buf_alloc =
	  (bstream->buf_pos + ALLOC_CHUNK) / ALLOC_CHUNK * ALLOC_CHUNK;
      }
    }
    bstream->buffer[
  }
}
