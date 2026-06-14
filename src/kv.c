#include "kv.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define TOMBSTONE 0x1

size_t hash (char *val, int capacity) {
  size_t hash = 0x13371337deadbeef;

  while(*val) {
    hash ^= *val;
    hash = hash << 8;
    hash += *val;

    val++;
  }

  return hash % capacity;
}


void kv_free(kv_t *db) {
  if(db == NULL) return;

  
  // Free all keys and values of entries
  for(size_t i = 0; i < db->capacity; i++) {
    if(db->count == 0) break;
    
    kv_entry_t *entry = &db->entries[i];

    if(
      entry->key != NULL &&
      entry->key != (void*)TOMBSTONE
    ) {
      free(entry->key);
      free(entry->value);

      entry->key = NULL;
      entry->value = NULL;

      db->count--;
    }
  }

  // something bad happend cause the loop finished but there is data left
  // (check BEFORE freeing db, otherwise we read freed memory)
  if(db->count != 0) return;

  // Free entries
  free(db->entries);

  // Free DB itself
  // setting db to NULL is callers DUTY else we need double pointer in function signature
  free(db);

  return;
}

int kv_delete(kv_t *db, char *key) {
  if( db == NULL || key == NULL) return -1;
  
  size_t idx = hash(key, db->capacity);

  for(size_t i = 0; i < db->capacity; i++) {
    size_t real_idx = (idx + i) % db->capacity;

    kv_entry_t *entry = &db->entries[real_idx];

    if(
      entry->key != NULL &&
      entry->key != (void*)TOMBSTONE &&
      !strcmp(entry->key, key)
    ){
      free(entry->key);
      free(entry->value);

      entry->key = (void*)TOMBSTONE;    
      // defensive programming after freeing set it to null to
      // prevent "Use After Free"
      entry->value = NULL;

      db->count--;

      return 0;
    }
  }

  return -1;
}

char *kv_get (kv_t *db, char *key) {
  if( db == NULL || key == NULL) return NULL;

  size_t idx = hash(key, db->capacity);
  

  for(size_t i = 0; i < db->capacity; i++) {
    size_t real_idx = (idx + i) % db->capacity;

    kv_entry_t *entry = &db->entries[real_idx];

    if(entry->key == NULL) {
      return NULL;
    }

    if(
      entry->key != (void*)TOMBSTONE &&
      !strcmp(entry->key, key)
    ) {
      return entry->value;
    }
  }
  return NULL;
}



int kv_put (kv_t *db, char *key, char *value) {
  if (!db || !key || !value) return -1;

  size_t idx = hash(key, db->capacity);

  int foundFirstTombstoneAtThisIDX = -1;
  kv_entry_t *firstTombstoneEntry;
  
  for(size_t i = 0; i < db->capacity; i++) {
    // look for free field in db if hash id is already occupied
    // ensure we dont go out of db memory with % capacity => wrapping around
    size_t real_idx = (idx + i) % db->capacity;

    kv_entry_t *entry = &db->entries[real_idx];

    if(foundFirstTombstoneAtThisIDX == -1 && entry->key == (void*)TOMBSTONE) {
      // Found first tombstone
      foundFirstTombstoneAtThisIDX = real_idx;
      firstTombstoneEntry = entry; 
    }
    
    if(entry->key
       && entry->key != (void*)TOMBSTONE
       && !strcmp(key, entry->key)) {
      char *newVal = strdup(value);
      if(!newVal) return -1;
      // free old value. its not a value in the struct but just a pointer to anywhere in memory so we need to free
      free(entry->value);        
      entry->value = newVal;

      return 0;
    }

    // if entry->key was never set we can safely put the value here cause their would be just tombstone if the search key was farer away
    if(!entry->key) {
      char *newKey = strdup(key);
      char *newVal = strdup(value);

      if(!newKey || !newVal) {
        free(newKey);
        free(newVal);
        return -1;
      }

      entry->key = newKey;
      entry->value = newVal;
      db->count++;
      return 0;
    }
  }

  // searched complete db and didnt find an unused spot.
  // now try to insert into first found tombstone if there is one
  // otherwise db is occupied
  if(foundFirstTombstoneAtThisIDX != -1) {
      char *newKey = strdup(key);
      char *newVal = strdup(value);

      if(!newKey || !newVal) {
        free(newKey);
        free(newVal);
        return -1;
      }

      firstTombstoneEntry->key = newKey;
      firstTombstoneEntry->value = newVal;
      db->count++;
      return 0;
  }

  // the db is occupied
  return -2;
}

kv_t *kv_init(size_t capacity) { 

  if (capacity == 0) return NULL;
  
  kv_t *table = malloc(sizeof(kv_t));

  if (table == NULL) {
    return NULL;
  }
  
  table->capacity = capacity;
  table->count = 0;

  table->entries = calloc(sizeof(kv_entry_t), capacity);

  if (table->entries == NULL) {
    free(table);
    return NULL;
  }


  return table;
}

  
