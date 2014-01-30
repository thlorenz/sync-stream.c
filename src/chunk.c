#include <stdlib.h>
#include "sst.h"

sst_chunk_t *sst_chunk_new(void* data, void (*free_data)(void*)) {
  sst_chunk_t* chunk;
  chunk            = malloc(sizeof *chunk);
  chunk->data      = data;
  chunk->enc       = UTF8;
  chunk->error     = 0;
  chunk->free_data = free_data;
  return chunk;
}

void sst_chunk_free(sst_chunk_t* chunk) {
  if (chunk->free_data) chunk->free_data(chunk->data);
  free(chunk);
}
