#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sst.h"

char *strtoupper(char *s) {
  char *cp, *p;
  p = cp = strdup(s);
  while((*p = toupper(*p))) p++;
  return cp;
}

char *strreverse(char *s) {
  char *rs, *sp, *rsp;
  size_t len = strlen(s);
  rs = malloc(sizeof (char*) * len);
  rs[len] = '\0';

  rsp = rs + (len - 1);
  sp = s;
  while(*sp) {
    *rsp = *sp;
    sp++; rsp--;
  }
  return rs;
}

void upper_onwrite(sst_transform_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;

  s = (char*)chunk->data;

  chunk_out       = sst_chunk_new();
  chunk_out->data = strtoupper(s);
  self->emit(self, chunk_out);
}

void reverse_onwrite(sst_transform_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;
  s = (char*)chunk->data;

  chunk_out       = sst_chunk_new();
  chunk_out->data = strreverse(s);
  self->emit(self, chunk_out);

  chunk_out       = sst_chunk_new();
  chunk_out->data = " <|> ";

  /* data isn't allocated, so we need to prevent attempt to free it */
  chunk_out->free_data = NULL;
  self->emit(self, chunk_out);
}

void onread(sst_chunk_t* chunk) {
  fprintf(stderr, "%s", chunk->data);
}

void write(sst_transform_t* stream, char* data) {
  sst_chunk_t* chunk = sst_chunk_new();
  chunk->data = data;
  stream->write(stream, chunk);
}

int main(void) {
  sst_transform_t *tx_upper, *tx_reverse, *writable;

  tx_upper = sst_transform_new();
  tx_reverse = sst_transform_new();
  writable = sst_transform_new();

  tx_upper->write = upper_onwrite;
  tx_upper->pipe  = tx_reverse;

  tx_reverse->write = reverse_onwrite;
  tx_reverse->pipe = writable;

  writable->read = onread;
  write(tx_upper, "hello");
  write(tx_upper, "world");
  write(tx_upper, ",");
  write(tx_upper, "my");
  write(tx_upper, "friends");


  sst_transform_free(tx_upper);
  sst_transform_free(tx_reverse);
  sst_transform_free(writable);

  return 0;
}
