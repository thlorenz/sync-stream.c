#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sst.h"

#define t_ok(pred, msg) do {                       \
  fprintf(stderr, "  - \x1b[90m" msg "\x1b[0m\n"); \
  assert(pred && msg);                             \
} while(0)

sst_chunk_t *create() {
  char* data = strdup("hello");
  return sst_chunk_new(data, free);
}

void destroy(sst_chunk_t* chunk) {
  sst_chunk_free(chunk);
}

int main(void) {
  sst_chunk_t *chunk = create();
  t_ok(chunk != NULL, "create chunk");

  destroy(chunk);

  return 0;
}
