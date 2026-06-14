#include <stdio.h>
#include <kv.h>
#include <assert.h>
#include <string.h>

int main() {
  kv_t *db = kv_init(16);

  kv_put(db, "name", "alice");
  assert(strcmp(kv_get(db, "name"), "alice") == 0);
  assert(kv_get(db, "missing") == NULL);

  printf("All assertions passed!\n");

  // kv_free(db);
}
