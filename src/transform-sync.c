#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <ee.h>


typedef struct stream_sync_rw_s stream_sync_rw_t;
typedef struct stream_chunk_s stream_chunk_t;
typedef void (*transform_cb)(const stream_chunk_t*, stream_chunk_t*);
typedef void (*stream_cb) (stream_chunk_t*);

enum stream_encoding {
    UTF8
  , BASE64
  , HEX
};

struct stream_sync_rw_s {
  ee_t ee;
  transform_cb transform;
};

struct stream_chunk_s {
  void* data;
  enum stream_encoding enc;
  int result;
};

// async would include the loop?

stream_sync_rw_t* stream_sync_rw_new(transform_cb transform) {
  stream_sync_rw_t* self;
  ee_t* ee;
  ee = ee_new();
  self = (stream_sync_rw_t*) realloc(ee, sizeof self);
  self->transform = transform;

  return self;
}

void stream_sync_rw_destroy(stream_sync_rw_t* self) {
  ee_destroy((ee_t*)self);
}

void stream_sync_rw_on(stream_sync_rw_t* self, const char* name, stream_cb cb) {
  ee_on((ee_t*)self, name, (ee_cb)cb);
}

void stream_sync_rw_write(stream_sync_rw_t* self, const stream_chunk_t* chunk_in) {
  stream_chunk_t* chunk_out;
  chunk_out = malloc(sizeof chunk_out);

  self->transform(chunk_in, chunk_out);

  /* destroy chunk_in */
  return chunk_out->result
    ? ee_emit((ee_t*)self, "error", chunk_out)
    : ee_emit((ee_t*)self, "data", chunk_out);
}

/* TODO: stream_chunk_destroy */

/*
 * Example
 */

char *strtoupper(char *s){
  char *cp, *p;
  p = cp = strdup(s);
  while((*p = toupper(*p))) p++;
  return cp;
}

void transform(const stream_chunk_t* chunk_in, stream_chunk_t* chunk_out) {
  char *s = (char*)chunk_in->data;
  chunk_out->data = strtoupper(s);
}

void ondata(stream_chunk_t* chunk) {
  char* s = (char*)chunk->data;;
  fprintf(stderr, "data: %s\n", s);
}

int main(void) {

  stream_sync_rw_t* stream = stream_sync_rw_new(transform);
  stream_sync_rw_on(stream, "data", ondata);

  stream_chunk_t hello_chunk = { .data = "hello" };
  stream_sync_rw_write(stream, &hello_chunk);

  stream_sync_rw_destroy(stream);

  return 0;
}
