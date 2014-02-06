#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sst.h"
#include "test.h"

#define test(fn) \
  fputs("\n\x1b[34m# [file-stream] " # fn "\x1b[0m\n", stderr); \
  fn();

static char *read_file(const char* name) {
  char *file_contents;
  long file_size;
  FILE *file = fopen(name, "rb");

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  rewind(file);

  file_contents = malloc(file_size * (sizeof(char)));
  fread(file_contents, sizeof(char), file_size, file);
  fclose(file);

  file_contents[file_size - 1] = '\0';
  return file_contents;
}
static char *strtoupper(char *s) {
  char *cp, *p;
  p = cp = strdup(s);
  while((*p = toupper(*p))) p++;
  return cp;
}

static void onupper_write(sst_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;

  s = (char*)chunk->data;
  chunk_out = sst_chunk_new(strtoupper(s), free);

  sst_chunk_free(chunk);
  self->emit(self, chunk_out);
}

static void onoutfile_write(sst_t* self, sst_chunk_t* chunk) {
  sst_chunk_free(chunk);
}

static void oninfile_end(sst_t* stream) {
  sst_file_t* self = (sst_file_t*) stream;
  fclose(self->file);
  sst_file_free(self);
}

static void ontx_end(sst_t* self) {
  sst_free(self);
}

static void onoutfile_end(sst_t* stream) {
  sst_file_t* self = (sst_file_t*) stream;
  fclose(self->file);
  sst_file_free(self);
}

/* ------ Tests ------- */

void transform_file_content() {
  remove("test/fixtures/sample.out.txt");

  FILE *infile, *outfile;

  infile = fopen("test/fixtures/sample.txt", "r");
  assert(infile > 0 && "opening infile");

  outfile = fopen("test/fixtures/sample.out.txt", "w");
  assert(outfile > 0 && "opening outfile");

  sst_file_t *infs = sst_file_new(infile, NULL);
  infs->end_cb = oninfile_end;

  sst_t *tx = sst_new();
  tx->write = onupper_write;
  tx->end_cb = ontx_end;

  sst_file_t *outfs = sst_file_new(outfile, NULL);
  outfs->end_cb = onoutfile_end;

  sst_pipe((sst_t*)infs, tx, (sst_t*)outfs);

  sst_file_write_init(outfs);
  sst_file_read_start(infs);

  char *out = read_file("test/fixtures/sample.out.txt");
  char *expected = read_file("test/fixtures/sample.upper.txt");

  t_equal_str(expected, out, "transforms file content and writes it to out file");
  free(out);
  free(expected);
}

int main(void) {
  test(transform_file_content);
  return 0;
}
