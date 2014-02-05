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

char* strreverse(char *s) {
  char* str = strdup(s);

  char temp;
  size_t len = strlen(str) - 1;
  size_t stop = len / 2;
  size_t i,k;

  for(i = 0, k = len; i <= stop; i++, k--) {
    temp = str[k];
    str[k] = str[i];
    str[i] = temp;
  }
  return str;
}

void upper_onwrite(sst_t* self, sst_chunk_t* chunk) {
  sst_chunk_t *chunk_out;

  chunk_out = sst_chunk_new(strtoupper(chunk->data), free);
  sst_chunk_free(chunk);

  self->emit(self, chunk_out);
}

void reverse_onwrite(sst_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;
  s = (char*)chunk->data;

  chunk_out = sst_chunk_new(strreverse(s), free);
  self->emit(self, chunk_out);

  chunk_out = sst_chunk_new(" <|> ", NULL); /* data isn't allocated */
  sst_chunk_free(chunk);

  self->emit(self, chunk_out);
}

void writable_onchunk(sst_t* stream, sst_chunk_t* chunk) {
  fprintf(stderr, "%s", chunk->data);
  sst_chunk_free(chunk);
}

void upper_onend(sst_t* stream) {
  fprintf(stderr, "\nupper stream ended\n");
  sst_free(stream);
}

void reverse_onend(sst_t* stream) {
  fprintf(stderr, "\nreverse stream ended\n");
  sst_free(stream);
}

void writable_onend(sst_t* stream) {
  fprintf(stderr, "\nwritable stream ended\n");
  sst_free(stream);
}

void write(sst_t* stream, char* data) {
  sst_chunk_t* chunk = sst_chunk_new(data, NULL);
  stream->write(stream, chunk);
}

int main(void) {
  sst_t *tx_upper, *tx_reverse, *writable;

  tx_upper = sst_new();
  tx_reverse = sst_new();
  writable = sst_new();

  tx_upper->write = upper_onwrite;
  tx_upper->end_cb = upper_onend;

  tx_reverse->write = reverse_onwrite;
  tx_reverse->end_cb = reverse_onend;

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
