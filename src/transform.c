#include <stdio.h>
#include <stdlib.h>

#include "sst.h"

static void sst__emit_n_pipe ( sst_transform_t* self, sst_chunk_t* chunk);
static void sst__passthru    ( sst_transform_t* self, sst_chunk_t* chunk);
static void sst__end         ( sst_transform_t* self);

sst_transform_t* sst_transform_new() {
  sst_transform_t* self;

  self          = malloc(sizeof *self);
  self->write   = sst__passthru;
  self->emit_cb = NULL;
  self->end_cb  = NULL;
  self->pipe    = NULL;

  /* readonly */
  self->emit    = sst__emit_n_pipe;
  self->end     = sst__end;

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
  if (self->emit_cb) self->emit_cb(self, chunk);
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

/**
 * called when user calls stream->end
 * invokes stream->end_cb and if piped pipe->end in order to propagate the end event
 *
 * @self   transform stream
 */
static void sst__end(sst_transform_t* self) {
  if (self->end_cb) self->end_cb(self);
  if (self->pipe) self->pipe->end(self->pipe);
}
