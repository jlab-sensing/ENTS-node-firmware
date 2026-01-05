#include <stdio.h>

#include "include\dict.h"

// Global dictionary variable
static DictEntry *dictionary = NULL;

// Initialize the dictionary (optional, uthash initializes on first use)
void dict_init() { dictionary = NULL; }

// Add an entry to the dictionary
void dict_add(const char *key, int value) {
  DictEntry *entry = malloc(sizeof(DictEntry));
  if (!entry) {
    fprintf(stderr, "Memory allocation failed\n");
    return;
  }
  strcpy(entry->key, key);
  entry->value = value;
  HASH_ADD_STR(dictionary, key, entry);
}

// Search for an entry in the dictionary by key
int dict_search(const char *key) {
  DictEntry *entry;
  HASH_FIND_STR(dictionary, key, entry);
  return entry ? entry->value : -1;  // Return -1 if not found
}

// Free all dictionary entries
void dict_free() {
  DictEntry *entry, *tmp;
  HASH_ITER(hh, dictionary, entry, tmp) {
    HASH_DEL(dictionary, entry);
    free(entry);
  }
}
