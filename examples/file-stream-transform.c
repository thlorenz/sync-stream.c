#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "sst.h"

void tx_write(sst_t* tx, sst_chunk_t* chunk) {
  /* emit line length first followed by the line */
  int len;
  char slen[10];
  sst_chunk_t *chunk_len;
  sst_chunk_t *chunk_out;

  len = strlen(chunk->data) - 1;    /* don't count new line */
  sprintf(slen, "%3d | ", len);

  chunk_len = sst_chunk_new(slen, NULL); /* slen is not allocated */
  chunk_out = sst_chunk_new(strdup(chunk->data), sst_free_string);

  tx->emit(tx, chunk_len);
  tx->emit(tx, chunk_out);
}

int main(void) {
  FILE *infile, *outfile;
  infile = fopen("src/stream.c", "r");
  assert(infile > 0 && "opening infile");

  // outfile = fopen("tmp/stream.txt", "w");
  outfile = stdout;
  assert(outfile > 0 && "opening outfile");

  sst_file_t *infs = sst_file_new(infile);
  sst_file_t *outfs = sst_file_new(outfile);

  sst_t *tx = sst_new();
  tx->write = tx_write;

  sst_pipe((sst_t*)infs, tx, (sst_t*)outfs);

  sst_file_write_init(outfs);
  sst_file_read_start(infs);

  return 0;
}
