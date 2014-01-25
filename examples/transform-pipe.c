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

  chunk_out       = sst_chunk_new(strtoupper(s));
  self->emit(self, chunk_out);
}

void reverse_onwrite(sst_transform_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;
  s = (char*)chunk->data;

  chunk_out = sst_chunk_new(strreverse(s));
  self->emit(self, chunk_out);

  chunk_out = sst_chunk_new(" <|> ");

  /* data isn't allocated, so we need to prevent attempt to free it */
  chunk_out->free_data = NULL;
  self->emit(self, chunk_out);
}

void write_onchunk(sst_transform_t* stream, sst_chunk_t* chunk) {
  fprintf(stderr, "%s", chunk->data);
}

void write_onend(sst_transform_t* stream) {
  fprintf(stderr, "\nstream ended\n");
}

void write(sst_transform_t* stream, char* data) {
  sst_chunk_t* chunk = sst_chunk_new(data);
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

  writable->emit_cb = write_onchunk;
  writable->end_cb = write_onend;
  write(tx_upper, "hello");
  write(tx_upper, "world");
  write(tx_upper, ",");
  write(tx_upper, "my");
  write(tx_upper, "friends");

  tx_upper->end(tx_upper);


  sst_transform_free(tx_upper);
  sst_transform_free(tx_reverse);
  sst_transform_free(writable);

  return 0;
}
