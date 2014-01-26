#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sst.h"

static void sst__write_file ( sst_t* stream, sst_chunk_t* chunk);
static void sst__end_file   ( sst_t* stream);

sst_file_t *sst_file_new(FILE *file) {
  sst_file_t *self;

  self = realloc(sst_new(), sizeof *self);
  self->file = file;
  self->bufsize = BUFSIZ;
  self->free_onend = 0;

  /* by default we'll take care of closing the file and freeing the stream */
  self->end = sst__end_file;
  return self;
}

void sst_file_free(sst_file_t* self) {
  free(self->file);
  sst_free((sst_t*)self);
}

void sst_file_read_start(sst_file_t* self) {
  char buf[self->bufsize];
  sst_chunk_t *chunk;

  while(fgets(buf, self->bufsize, self->file)) {
    chunk = sst_chunk_new(buf);
    chunk->free_data = NULL;
    self->write((sst_t*)self, chunk);
  }
  self->end((sst_t*)self);
}

void sst_file_write_init(sst_file_t* self) {
  self->emit_cb = sst__write_file;
  self->free_onend = 1;
}

/* private */

/**
 * The default `emit_cb` for file write streams.
 * Writes the data inside the chunk to the underlying file.
 *
 * @stream    the file stream
 * @chunk     emitted chunk
 */
static void sst__write_file(sst_t* stream, sst_chunk_t* chunk) {
  int r;
  sst_file_t* fstream;
  fstream = (sst_file_t*) stream;

  r = fputs(chunk->data, fstream->file);
  assert(r != EOF && "fputs chunk data");
}

/**
 * The default `end` callback for all file streams.
 * Closes the file handle and frees the file and wrapping stream.
 *
 * @self  the file stream
 */
static void sst__end_file(sst_t* self) {
  sst_file_t* fstream;
  fstream = (sst_file_t*) self;
  fclose(fstream->file);
  if(fstream->free_onend) sst_file_free(fstream);
}
