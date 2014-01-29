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
  rs = malloc(sizeof (char) * len);
  rs[len] = '\0';

  rsp = rs + (len - 1);
  sp = s;
  while(*sp) {
    *rsp = *sp;
    sp++; rsp--;
  }
  return rs;
}

void upper_onwrite(sst_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;

  s = (char*)chunk->data;

  chunk_out = sst_chunk_new(strtoupper(s), sst_free_string);
  self->emit(self, chunk_out);
}

void reverse_onwrite(sst_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;
  s = (char*)chunk->data;

  chunk_out = sst_chunk_new(strreverse(s), sst_free_string);
  self->emit(self, chunk_out);

  chunk_out = sst_chunk_new(" <|> ", NULL); /* data isn't allocated */
  self->emit(self, chunk_out);

  sst_chunk_free(chunk);
}

void writable_onchunk(sst_t* stream, sst_chunk_t* chunk) {
  fprintf(stderr, "%s", chunk->data);
  sst_chunk_free(chunk);
}

void writable_onend(sst_t* stream) {
  fprintf(stderr, "\nstream ended\n");
  /* this also frees all streams that are upstream from this stream */
  sst_free(stream);
}

void write(sst_t* stream, char* data) {
  sst_chunk_t* chunk = sst_chunk_new(data, sst_free_string);
  stream->write(stream, chunk);
}

int main(void) {
  sst_t *tx_upper, *tx_reverse, *writable;

  tx_upper = sst_new();
  tx_reverse = sst_new();
  writable = sst_new();

  tx_upper->write = upper_onwrite;
  tx_reverse->write = reverse_onwrite;

  sst_pipe(tx_upper, tx_reverse, writable);

  writable->emit_cb = writable_onchunk;
  writable->end_cb = writable_onend;

  write(tx_upper, "hello");
  write(tx_upper, "world");
  write(tx_upper, ",");
  write(tx_upper, "my");
  write(tx_upper, "friends");

  tx_upper->end(tx_upper);

  return 0;
}
