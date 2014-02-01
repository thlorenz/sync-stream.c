#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sst.h"
#include "test.h"

#define test(fn) \
  fputs("\n\x1b[34m# [chunk] " # fn "\x1b[0m\n", stderr); \
  fn();

sst_chunk_t *create() {
  char* data = strdup("hello");
  return sst_chunk_new(data, free);
}

void destroy(sst_chunk_t* chunk) {
  sst_chunk_free(chunk);
}

void new_free() {
  sst_chunk_t *chunk = create();
  t_ok(chunk != NULL, "create chunk");

  destroy(chunk);
}

int main(void) {
  test(new_free);
  return 0;
}
