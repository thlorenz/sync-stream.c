#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sst.h"
#include "test.h"

#define test(fn) \
  fputs("\n\x1b[34m# [stream-pipe] " # fn "\x1b[0m\n", stderr); \
  fn();

static char *strtoupper(char *s) {
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

static void upper_onwrite(sst_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;

  s = (char*)chunk->data;
  chunk_out = sst_chunk_new(strtoupper(s), free);

  self->emit(self, chunk_out);
  sst_chunk_free(chunk);
}

static void reverse_onwrite(sst_t* self, sst_chunk_t* chunk) {
  char *s;
  sst_chunk_t *chunk_out;
  s = (char*)chunk->data;

  chunk_out = sst_chunk_new(strreverse(s), free);
  self->emit(self, chunk_out);

  sst_chunk_free(chunk);
}

static char* emitted;
static int emit_count = 0;
static int end_src_called = 0;
static int end_uppertx_called = 0;
static int end_reversetx_called = 0;
static int end_dst_called = 0;
void setup() {
  emitted              = NULL;
  emit_count           = 0;
  end_src_called       = 0;
  end_uppertx_called   = 0;
  end_reversetx_called = 0;
  end_dst_called       = 0;
}

static void ondata(sst_t* self, sst_chunk_t* chunk) {
  emitted = strdup(chunk->data);
  emit_count++;
  sst_chunk_free(chunk);
}

static void onsrc_end(sst_t* self) {
  end_src_called++;
  sst_free(self);
}

static void onuppertx_end(sst_t* self) {
  end_uppertx_called++;
  sst_free(self);
}

static void onreversetx_end(sst_t* self) {
  end_reversetx_called++;
  sst_free(self);
}

static void ondst_end(sst_t* self) {
  end_dst_called++;
  sst_free(self);
}

/* ------ Tests ------- */

void pipe_transform_stream() {
  setup();

  sst_t *src = sst_new();
  sst_t *tx = sst_new();
  src->end_cb  = onsrc_end;

  tx->write = upper_onwrite;
  tx->emit_cb = ondata;
  tx->end = onuppertx_end;

  sst_pipe(src, tx);

  src->write(src, sst_chunk_new("d0", NULL));
  t_ok(emit_count == 1, "after one write, one chunk was emitted");
  t_equal_str(emitted, "D0", "first emitted chunk is the one first emitted uppercased");
  free(emitted);

  src->write(src, sst_chunk_new("d1", NULL));
  t_ok(emit_count == 2, "after two writes, two chunks were emitted");
  t_equal_str(emitted, "D1", "second emitted chunk is the one second emitted uppercased");
  free(emitted);

  src->end(src);
  t_ok(end_src_called == 1, "end src is called exactly once after src stream was ended");
  t_ok(end_uppertx_called == 1, "end upper tx is called exactly once after src stream was ended");
}

void pipe_multi_transform_stream() {
  setup();

  sst_t *src       = sst_new();
  sst_t *uppertx   = sst_new();
  sst_t *reversetx = sst_new();
  sst_t *dst       = sst_new();

  src->end_cb  = onsrc_end;

  uppertx->write = upper_onwrite;
  uppertx->end_cb = onuppertx_end;

  reversetx->write = reverse_onwrite;
  reversetx->end_cb = onreversetx_end;

  dst->emit_cb = ondata;
  dst->end_cb = ondst_end;

  sst_pipe(src, uppertx, reversetx, dst);

  src->write(src, sst_chunk_new("d0", NULL));
  t_ok(emit_count == 1, "after one write, one chunk was emitted by dst");
  t_equal_str(emitted, "0D", "first emitted chunk is the one first emitted reversed and uppercased");
  free(emitted);

  src->write(src, sst_chunk_new("d1", NULL));
  t_ok(emit_count == 2, "after two writes, two chunks were emitted by dst");
  t_equal_str(emitted, "1D", "second emitted chunk is the one second emitted reversed and uppercased");
  free(emitted);

  src->end(src);
  t_ok(end_src_called == 1, "end src is called exactly once after src stream was ended");
  t_ok(end_uppertx_called == 1, "end upper tx is called exactly once after src stream was ended");
  t_ok(end_reversetx_called == 1, "end reverse tx is called exactly once after src stream was ended");
  t_ok(end_dst_called == 1, "end dst is called exactly once after src stream was ended");
}

int main(void) {
  test(pipe_transform_stream);
  test(pipe_multi_transform_stream);

  return 0;
}
