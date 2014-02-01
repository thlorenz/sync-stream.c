#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sst.h"

#ifndef strdup
extern char *strdup(const char *str);
#endif

#define test(fn) \
  fputs("\n\x1b[34m# " # fn "\x1b[0m\n", stderr); \
  fn();

#define t_ok(pred, msg) do {                       \
  fprintf(stderr, "  - \x1b[90m" msg "\x1b[0m\n"); \
  assert(pred && msg);                             \
} while(0)

#define t_equal_str(s1, s2, msg) t_ok(strcmp((s1), (s2)) == 0, msg)

char *strtoupper(char *s) {
  char *cp, *p;
  p = cp = (char*)strdup(s);
  while((*p = toupper(*p))) p++;
  return cp;
}

void upper_onwrite(sst_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;

  s = (char*)chunk->data;
  free(chunk);

  chunk_out = sst_chunk_new(strtoupper(s), free);
  self->emit(self, chunk_out);
}

static char* emitted;
static int emit_count = 0;
static int end_called = 0;
void setup() {
  emitted = NULL;
  emit_count = 0;
  end_called = 0;
}

static void ondata(sst_t* self, sst_chunk_t* chunk) {
  emitted = strdup(chunk->data);
  emit_count++;
  sst_chunk_free(chunk);
}

static void onend(sst_t* self) {
  end_called++;
  sst_free(self);
}

/* ------ Tests ------- */

void new_free() {
  sst_t *tx = sst_new();
  t_ok(tx != NULL, "new stream");
  sst_free(tx);
}

void writable_stream() {
  setup();

  sst_t *writable = sst_new();
  writable->emit_cb = ondata;
  writable->end_cb  = onend;

  writable->write(writable, sst_chunk_new("d0", NULL));
  t_ok(emit_count == 1, "after one write, one chunk was emitted");
  t_equal_str(emitted, "d0", "first emitted chunk is the one first emitted");
  free(emitted);

  writable->write(writable, sst_chunk_new("d1", NULL));
  t_ok(emit_count == 2, "after two writes, two chunks were emitted");
  t_equal_str(emitted, "d1", "second emitted chunk is the one second emitted");
  free(emitted);

  t_ok(end_called == 0, "end is not called before stream was ended");

  writable->end(writable);
  t_ok(end_called == 1, "end is called exactly once after stream was ended");
}

void transform_stream() {
  setup();

  sst_t *tx = sst_new();
  tx->write = upper_onwrite;
  tx->emit_cb = ondata;
  tx->end_cb  = onend;

  tx->write(tx, sst_chunk_new("d0", NULL));
  t_ok(emit_count == 1, "after one write, one chunk was emitted");
  t_equal_str(emitted, "D0", "first emitted chunk is the one first emitted");
  free(emitted);

  tx->write(tx, sst_chunk_new("d1", NULL));
  t_ok(emit_count == 2, "after two writes, two chunks were emitted");
  t_equal_str(emitted, "D1", "second emitted chunk is the one second emitted");
  free(emitted);

  tx->end(tx);
  t_ok(end_called == 1, "end is called exactly once after stream was ended");
}

int main(void) {
  test(new_free);
  test(writable_stream);
  test(transform_stream);
  return 0;
}
