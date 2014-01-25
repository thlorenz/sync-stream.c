#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

typedef struct sst_transform_s sst_transform_t;
typedef struct sst_chunk_s sst_chunk_t;
typedef void (*sst_chunk_data_free_cb)(void*);

typedef void ( *sst_emit_cb)  ( sst_transform_t*, sst_chunk_t*);
typedef void ( *sst_read_cb)  ( sst_chunk_t*);
typedef void ( *sst_write_cb) ( sst_transform_t*, sst_chunk_t*);

enum sst_encoding {
    UTF8   = 0x0
  , BASE64 = 0x2
  , HEX    = 0x4
  , CUSTOM = 0x1000
};

/*
 * transform struct
 *
 * @write     call this with a @see chunk in order to feed data into the transform
 * @read      if provided, it is called with every chunk emitted by the transform
 * @pipe      if set all emitted chunk will be written to that transform
 * @emit      call this to emit a chunk
 */
struct sst_transform_s {
  sst_write_cb      write;
  sst_read_cb       read;
  sst_transform_t   *pipe;

  /* readonly */
  sst_emit_cb       emit;
};

/*
 * chunk struct
 *
 * @data          the data transmitted with the chunk
 * @enc           encoding of the data
 * @free_data     (default: free((char*)data)) called to free the data
 *                if set to NULL, data won't be freed, use this if data wasn't allocated
 * @nofree        (default: 0) if set, chunk is not freed after it was emitted/piped
 * @result        set this to a non-zero value to signal an error
 */
struct sst_chunk_s {
  void                    *data;
  enum sst_encoding        enc;
  sst_chunk_data_free_cb   free_data;
  short                   nofree;
  int                     result;
};

/* default free data which assumes data to be a char* */
static void sst__chunk_free_data(void* data) {
  free((char*)data);
}

/*
 * Initializes new chunk and returns the result.
 *
 * @see chunk struct for more initialization options
 * @return chunk
 */
sst_chunk_t *sst_chunk_new() {
  sst_chunk_t *chunk;

  chunk         = malloc(sizeof *chunk);
  chunk->data   = NULL;
  chunk->enc    = UTF8;
  chunk->result = 0;
  chunk->nofree = 0;

  /* by default we assume chunk data to an allocated char* */
  /* in all other cases a custom free needs to be provided */
  chunk->free_data   = sst__chunk_free_data;

  return chunk;
}

/**
 * frees the chunk
 * @chunk the chunk to free
 */
void sst_chunk_free(sst_chunk_t* chunk) {
  if (chunk->free_data) chunk->free_data(chunk->data);
  free(chunk);
}

/**
 * Tries to call read and pipe for the given chunk and then frees it unless it has `nofree` flag set.
 *
 * @private
 * @self    transform stream
 * @chunk   the chunk to emit
 */
void sst__emit_n_pipe(sst_transform_t* self, sst_chunk_t* chunk) {
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
void sst__passthru(sst_transform_t* self, sst_chunk_t* chunk) {
  chunk->nofree = 1;
  self->emit(self, chunk);
}

/**
 * Initializes a transform and returns it
 * @see transform struct for more initializiation options
 *
 * @return transform stream
 */
sst_transform_t* sst_transform_new() {
  sst_transform_t* self;

  self        = malloc(sizeof *self);
  self->write = sst__passthru;
  self->read  = NULL;
  self->pipe  = NULL;
  self->emit  = sst__emit_n_pipe;

  return self;
}

/*
 * Example
 */

char *strtoupper(char *s){
  char *cp, *p;
  p = cp = strdup(s);
  while((*p = toupper(*p))) p++;
  return cp;
}

void onwrite(sst_transform_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;

  s = (char*)chunk->data;

  chunk_out       = sst_chunk_new();
  chunk_out->data = strtoupper(s);
  self->emit(self, chunk_out);

  chunk_out            = sst_chunk_new();
  chunk_out->data      = "world";

  /* data isn't allocated, so we need to prevent attempt to free it */
  chunk_out->free_data = NULL;

  self->emit(self, chunk_out);
}

void onread(sst_chunk_t* chunk) {
  fprintf(stderr, "data: %s\n", chunk->data);
}

int main(void) {
  sst_transform_t *tx_uno, *tx_dos;

  tx_uno = sst_transform_new();
  tx_dos = sst_transform_new();

  tx_uno->write = onwrite;
  tx_uno->pipe  = tx_dos;

  tx_dos->read = onread;

  sst_chunk_t* chunk = sst_chunk_new();
  chunk->data = "hello";
  tx_uno->write(tx_uno, chunk);

  return 0;
}
