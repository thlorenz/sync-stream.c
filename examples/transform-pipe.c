#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "sst.h"

char *strtoupper(char *s){
  char *cp, *p;
  p = cp = strdup(s);
  while((*p = toupper(*p))) p++;
  return cp;
}

/* TODO: emit reverse */
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

  sst_transform_free(tx_uno);
  sst_transform_free(tx_dos);
  return 0;
}
