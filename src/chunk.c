#include <stdlib.h>
#include "sst.h"

static void sst__chunk_free_data(void* data);

sst_chunk_t *sst_chunk_new(void* data) {
  sst_chunk_t *chunk;

  chunk         = malloc(sizeof *chunk);
  chunk->data   = data;
  chunk->enc    = UTF8;
  chunk->result = 0;
  chunk->nofree = 0;

  /* by default we assume chunk data to an allocated char* */
  /* in all other cases a custom free needs to be provided */
  chunk->free_data = sst__chunk_free_data;

  return chunk;
}

void sst_chunk_free(sst_chunk_t* chunk) {
  if (chunk->free_data) chunk->free_data(chunk->data);
  free(chunk);
}

/*
 * private
 */

/* default free data which assumes data to be a char* */
static void sst__chunk_free_data(void* data) {
  free((char*)data);
}
