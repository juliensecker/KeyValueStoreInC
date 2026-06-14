#include <stdio.h>
#include <kv.h>
#include <assert.h>
#include <string.h>

int main() {
  kv_t *db = kv_init(16);
  
  kv_put(db, "name", "alice");
  kv_put(db, "city", "berlin");
  
  kv_put(db, "name", "lice");
  kv_put(db, "name", "ce");


  
  assert(kv_delete(db, "name") == 0);
  assert(kv_get(db, "name") == NULL);
  assert(db->count == 1);
  
  assert(kv_delete(db, "missing") == -1);


  printf("all Tests passed!");
  // kv_free(db);
}
