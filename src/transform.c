#include <stdio.h>
#include <stdlib.h>

#include "sst.h"

static void sst__emit_n_pipe ( sst_transform_t* self, sst_chunk_t* chunk);
static void sst__passthru    ( sst_transform_t* self, sst_chunk_t* chunk);

sst_transform_t* sst_transform_new() {
  sst_transform_t* self;

  self        = malloc(sizeof *self);
  self->write = sst__passthru;
  self->read  = NULL;
  self->pipe  = NULL;
  self->emit  = sst__emit_n_pipe;

  return self;
}

void sst_transform_free(sst_transform_t* self) {
  free(self);
}

/* private */

/**
 * Tries to call read and pipe for the given chunk and then frees it unless it has `nofree` flag set.
 *
 * @self    transform stream
 * @chunk   the chunk to emit
 */
static void sst__emit_n_pipe(sst_transform_t* self, sst_chunk_t* chunk) {
  if (self->read) self->read(chunk);
  if (self->pipe) self->pipe->write(self->pipe, chunk);

  if (chunk->nofree) {
    /* don't destroy now, but unmark */
    chunk->nofree = 0;
  } else {
    sst_chunk_free(chunk);
  }
}

/**
 * pass through transform function which is used as the default write for transforms
 *
 * @self    transform stream
 * @chunk   the chunk to emit
 */
static void sst__passthru(sst_transform_t* self, sst_chunk_t* chunk) {
  chunk->nofree = 1;
  self->emit(self, chunk);
}
