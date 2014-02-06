#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sst.h"

static void sst__write_file ( sst_t* stream, sst_chunk_t* chunk);
static void sst__end_file   ( sst_t* stream);

sst_file_t *sst_file_new(FILE *file, void (*free_file)(void*)) {
  sst_file_t *self;

  self = realloc(sst_new(), sizeof *self);
  self->file = file;
  self->free_file = free_file;
  self->bufsize = BUFSIZ;
  self->free_onend = 0;

  return self;
}

void sst_file_free(sst_file_t* self) {
  if (self->free_file) self->free_file(self->file);
  sst_free((sst_t*)self);
}

void sst_file_read_start(sst_file_t* self) {
  char buf[self->bufsize];
  sst_chunk_t *chunk;

  while(fgets(buf, self->bufsize, self->file)) {
    chunk = sst_chunk_new(buf, NULL);
    self->write((sst_t*)self, chunk);
  }
  self->end((sst_t*)self);
}

void sst_file_write_init(sst_file_t* self) {
  self->emit_cb = sst__write_file;
}

/* private */

/**
 * The default `emit_cb` for file write streams.
 * Writes the data inside the chunk to the underlying file.
 *
 * Frees the chunk after it was written.
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

  sst_chunk_free(chunk);
}
