#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

typedef struct st_sync_transform_s st_sync_transform_t;
typedef struct st_chunk_s st_chunk_t;
typedef void (*st_chunk_data_free_cb)(void*);

typedef void ( *st_sync_emit_cb)  ( st_sync_transform_t*, st_chunk_t*);
typedef void ( *st_sync_read_cb)  ( st_chunk_t*);
typedef void ( *st_sync_write_cb) ( st_sync_transform_t*, st_chunk_t*);

enum st_encoding {
    UTF8   = 0x0
  , BASE64 = 0x2
  , HEX    = 0x4
  , CUSTOM = 0x128
};

struct st_sync_transform_s {
  st_sync_write_cb      write;
  st_sync_read_cb       read;
  st_sync_transform_t   *pipe;

  /* readonly */
  st_sync_emit_cb       emit;

  /* private */
};

struct st_chunk_s {
  void                    *data;
  enum st_encoding        enc;
  int                     result;
  st_chunk_data_free_cb   free;
  short                   pass_thru;
};

static void st__chunk_data_free(void* data) {
  free((char*)data);
}

st_chunk_t *st_chunk_new() {
  st_chunk_t *chunk;

  chunk            = malloc(sizeof *chunk);
  chunk->data      = NULL;
  chunk->enc       = UTF8;
  chunk->result    = 0;
  chunk->pass_thru = 0;

  /* by default we assume chunk data to an allocated char* */
  /* in all other cases a custom free needs to be provided */
  /* in order to prevent freeing the data (i.e. when it wasn't allocated) set chunk->free = NULL */
  chunk->free   = st__chunk_data_free;

  return chunk;
}

void st_chunk_destroy(st_chunk_t* chunk) {
  if (chunk->free) chunk->free(chunk->data);
  free(chunk);
}

void st__sync_emit_n_pipe(st_sync_transform_t* self, st_chunk_t* chunk) {
  if (self->read) self->read(chunk);
  if (self->pipe) self->pipe->write(self->pipe, chunk);
  if (chunk->pass_thru) {
    /* don't destroy now, but unmark */
    chunk->pass_thru = 0;
  } else {
    st_chunk_destroy(chunk);
  }
}

void st__sync_pass_thru(st_sync_transform_t* self, st_chunk_t* chunk) {
  chunk->pass_thru = 1;
  self->emit(self, chunk);
}

st_sync_transform_t* st_sync_transform_new() {
  st_sync_transform_t* self;

  self        = malloc(sizeof *self);
  self->write = st__sync_pass_thru;
  self->read  = NULL;
  self->pipe  = NULL;
  self->emit  = st__sync_emit_n_pipe;

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

void onwrite(st_sync_transform_t* self, st_chunk_t* chunk) {
  char *s;
  s = (char*)chunk->data;

  st_chunk_t *chunk_out;
  chunk_out = st_chunk_new();
  chunk_out->data = strtoupper(s);
  self->emit(self, chunk_out);

  chunk_out = st_chunk_new();
  chunk_out->data = "world";
  chunk_out->free = NULL;
  self->emit(self, chunk_out);
}

void onread(st_chunk_t* chunk) {
  fprintf(stderr, "data: %s\n", chunk->data);
}

int main(void) {
  st_sync_transform_t *tx_uno, *tx_dos;

  tx_uno = st_sync_transform_new();
  tx_dos = st_sync_transform_new();

  tx_uno->write = onwrite;
  tx_uno->pipe  = tx_dos;

  tx_dos->read = onread;

  st_chunk_t* chunk = st_chunk_new();
  chunk->data = "hello";
  tx_uno->write(tx_uno, chunk);

  return 0;
}
