#include <stdio.h>
#include <stdlib.h>

#include "sst.h"

static void sst__emit_n_pipe ( sst_t* self, sst_chunk_t* chunk);
static void sst__passthru    ( sst_t* self, sst_chunk_t* chunk);
static void sst__end         ( sst_t* self);

sst_t* sst_new() {
  sst_t* self;

  self          = malloc(sizeof *self);
  self->write   = sst__passthru;
  self->emit_cb = NULL;
  self->end_cb  = NULL;

  /* readonly */
  self->emit        = sst__emit_n_pipe;
  self->end         = sst__end;

  /* private */
  self->_destination = NULL;
  self->_source      = NULL;

  return self;
}

void sst_free(sst_t* self) {

  if (self->_source) sst_free(self->_source);
  free(self);
}

void sst_pipe(sst_t* source, sst_t* destination) {
  source->_destination = destination;
  destination->_source = source;
}

/* private */

/**
 * Tries to call read and pipe for the given chunk and then frees it unless it has `nofree` flag set.
 *
 * @self    stream
 * @chunk   the chunk to emit
 */
static void sst__emit_n_pipe(sst_t* self, sst_chunk_t* chunk) {
  if (self->emit_cb) self->emit_cb(self, chunk);
  if (self->_destination) self->_destination->write(self->_destination, chunk);

  if (chunk->nofree) {
    /* don't destroy now, but unmark */
    chunk->nofree = 0;
  } else {
    sst_chunk_free(chunk);
  }
}

/**
 * pass through function which is used as the default write for the stream essentially creating a through stream
 *
 * @self    stream
 * @chunk   the chunk to emit
 */
static void sst__passthru(sst_t* self, sst_chunk_t* chunk) {
  chunk->nofree = 1;
  self->emit(self, chunk);
}

/**
 * called when user calls stream->end
 * invokes stream->end_cb and if piped pipe->end in order to propagate the end event
 *
 * @self   stream
 */
static void sst__end(sst_t* self) {
  if (self->end_cb) self->end_cb(self);
  if (self->_destination) self->_destination->end(self->_destination);
}