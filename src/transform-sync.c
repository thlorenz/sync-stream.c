#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <ee.h>

typedef void* (*transform_cb)(void*);

struct stream_sync_rw_s {
  ee_t ee;
  transform_cb transform;
};

typedef struct stream_sync_rw_s stream_sync_rw_t;
typedef ee_cb stream_cb;

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
  ee_on((ee_t*)self, name, cb);
}

void stream_sync_rw_write(stream_sync_rw_t* self, void* data) {
  void* res;
  res = self->transform(data);
  ee_emit((ee_t*)self, "data", res);
}

char *strtoupper(char *s){
  char *cp, *p;
  p = cp = strdup(s);
  while((*p = toupper(*p))) p++;
  return cp;
}

void *transform(void* d) {
  char *s = (char*)d;
  return (void*) strtoupper(s);
}

void ondata(void* d) {
  char* s = (char*)d;
  fprintf(stderr, "data: %s\n", s);
}

int main(void) {

  stream_sync_rw_t* stream = stream_sync_rw_new(transform);
  stream_sync_rw_on(stream, "data", ondata);
  stream_sync_rw_write(stream, "hello");

  stream_sync_rw_destroy(stream);

  return 0;
}
