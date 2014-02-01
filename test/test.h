#define t_ok(pred, msg) do {                       \
  fprintf(stderr, "  - \x1b[90m" msg "\x1b[0m\n"); \
  assert(pred && msg);                             \
} while(0)

#define t_equal_str(s1, s2, msg) t_ok(strcmp((s1), (s2)) == 0, msg)

#ifndef strdup
extern char *strdup(const char *str);
#endif

