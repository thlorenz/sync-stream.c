#include <stdlib.h>
#include "sst.h"

sst_chunk_t *sst_chunk_new(void* data, sst_chunk_data_free_cb free_data) {
  sst_chunk_t *chunk;

  chunk            = malloc(sizeof *chunk);
  chunk->data      = data;
  chunk->enc       = UTF8;
  chunk->free_data = free_data;
  chunk->result    = 0;

  return chunk;
}

void sst_chunk_free(sst_chunk_t* chunk) {
  if (chunk->free_data) chunk->free_data(chunk->data);
  free(chunk);
}

void sst_free_string(void* data) {
  free((char*)data);
}
