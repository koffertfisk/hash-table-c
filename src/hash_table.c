#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

#define PRIMES_SIZE 11

/**
 * @file hash_table.c
 * @author Marcus Enderskog
 * @date 2021-04-15
 * @brief Simple hash table that maps generic keys to values.
 **/


/// @brief Represents a storage key-value pair entry which contains a generic element type.
struct entry
{
  elem_t key;     // Key to map value to.
  elem_t value;   // The actual value to be stored.
  entry_t *next;  // Next entry, possibly NULL.
};

/// @brief Actual hash table that maps generic keys to values.
struct hash_table
{
  size_t no_buckets;            // Number of buckets to store entries in.
  float load_factor;            // Maximum load factor before hash table gets resized.
  hash_function hash_function;  // Hash function to hash keys with.
  predicate_ht key_equiv;       // Function that determines how keys will get compared.
  predicate_ht value_equiv;     // Function that detetmines how values will get compared.
  size_t size;                  // Load/number of entries in the hash table.
  entry_t *buckets;             // Linked structure in which entries are stored.
};

/// Array of primes numbers to be used for setting the number of buckets in a hash table.
size_t primes[PRIMES_SIZE] = {17, 31, 67, 127, 257, 509, 1021, 2053, 4099, 8191, 16381};

/**
 * @brief Default hash function to assign.
 * @param key Key to hash.
 * @return Hash value in the form of an unsigned long integer.
 **/ 
static unsigned long default_hash_function(elem_t key);

/**
 * @brief Compare two integer keys for equality.
 * @param key Entry key to compare.
 * @param value_ignored Entry value (ignored).
 * @param x Key to compare against.
 **/
static bool default_key_equiv(const elem_t key, const elem_t value_ignored, const void *x);

/**
 * @brief Find the previous entry for a certain key.
 * @param func Hash function.
 * @param first_entry First entry of the bucket.
 * @param key Hashed key.
 * @return A pointer to the previous entry.
 **/
static entry_t *find_previous_entry_for_key(hash_function func, entry_t *first_entry, const unsigned long key);

/**
 * @brief Create a new entry.
 * @param key Key to associate with entry.
 * @param value Value to associate with entry.
 * @param next An entry pointer to the next entry.
 * @return A pointer to the newly created entry.
 **/
static entry_t *entry_create(const elem_t key, const elem_t value, entry_t *next);

/** 
 * @brief Return the values for all entries in a hash table (in no particular order, but same as hash_table_keys).
 * @param ht Hash table operated upon.
 * @return An array of values for hash table ht.
 **/
static elem_t **hash_table_values_arr(hash_table_t *ht);

/**
 * @brief Destroy an entry by freeing its allocated memory.
 * @param entry Entry to destroy.
 **/
static void entry_destroy(entry_t *entry);

/**
 * @brief Check whether the prime number library contains a given number.
 * @param num Number to examine.
 * @return True if the number is contained within the library, false otherwise.
 **/
static bool is_number_in_prime_library(const int num);

/**
 * @brief Get the next prime number stored in the library.
 * @param current Current prime number.
 * @return The next prime number or -1 if no one was found.
 **/
static int get_next_prime_number(const int current);

/**
 * @brief Dummy function pointer to be used with linked lists.
 * @param a First element.
 * @param b Second element.
 * @return True
 **/
static bool dummy_func_ptr(const elem_t a, const elem_t b);

/**
 * @brief Default hash function to assign.
 * @param key Key to hash.
 * @return Hash value in the form of an unsigned long integer.
 **/ 
static unsigned long default_hash_function(elem_t key)
{
  return (unsigned long) key.i;
}

/**
 * @brief Compare two integer keys for equality.
 * @param key Entry key to compare.
 * @param value_ignored Entry value (ignored).
 * @param x Key to compare against.
 **/
static bool default_key_equiv(const elem_t key, const elem_t value_ignored, const void *x)
{
  return key.i == ((elem_t*)x)->i; 
}

/**
 * @brief Compare two integer values for equality.
 * @param key_ignoed Entry key (ignored).
 * @param value_ignored Entry value to compare.
 * @param x Value to compare against.
 **/
static bool default_value_equiv(const elem_t key_ignored, const elem_t value, const void *x)
{
  return value.i == ((elem_t*)x)->i;
}

/**
 * @brief Check whether the prime number library contains a given number.
 * @param num Number to examine.
 * @return True if the number is contained within the library, false otherwise.
 **/
static bool is_number_in_prime_library(const int num)
{
  bool result = false;
  for (int i = 0; i < PRIMES_SIZE; i++)
  {
    if (primes[i] == num)
    {
      result = true;
      break;
    }
  }
  return result;
}

/**
 * @brief Get the next prime number stored in the library.
 * @param current Current prime number.
 * @return The next prime number or -1 if no one was found.
 **/
static int get_next_prime_number(const int current)
{
  int result = -1;
  for (int i = 0; i < PRIMES_SIZE; i++)
  {
    if (primes[i] == current)
    {
      if (i < (PRIMES_SIZE - 1))
        result = primes[i + 1];
      break;
    }
  }

  return result;
}

/**
 * @brief Resize and rehash a hash table if necessary.
 * @param ht Hash table to operate on.
 * @return A resized hash table if all operations were successful, otherwise the old hash table is returned.
 * 
 * Resizing is done by examining whether the maximum load (determined by the load factor) has been reached;
 * if this is the case, a new bucket size is calculated by attempting to get the next prime number from the 
 * internal prime number library. If the hash table is currently using the last (highest) available for its 
 * bucket size, resizing is not possible and the present hash table will be returned. If, however, resizing 
 * is possible, memory for the bucket entries will be reallocated and rehashed.
 **/
static hash_table_t *hash_table_resize(hash_table_t *ht)
{  
  float current_load = (float) ht->size / (float) ht->no_buckets;
  if (current_load >= ht->load_factor)
  {
    printf("Maximum load factor reached (%.2f), triggering resize..\n", current_load);
    int no_buckets_new = get_next_prime_number(ht->no_buckets);
    if (no_buckets_new != -1)
    {
      int no_buckets_old = ht->no_buckets;
      printf("New size is: %d\n", no_buckets_new);
      entry_t *buckets_new = calloc(no_buckets_new, sizeof(entry_t));
      if (buckets_new == NULL)
      {
        puts("Failed to reallocate memory!");
        return ht;
      }

      for (int i = 0; i < no_buckets_new; i++)
      {
        entry_t new_entry = {.key = int_elem(0), .value = int_elem(0), .next = NULL};
        buckets_new[i] = new_entry;
      }
      
      puts("Rehashing");
      for (int i = 0; i < no_buckets_old; i++) 
      {
        entry_t *first_entry = &ht->buckets[i];
        entry_t *cursor = first_entry->next;
        while (cursor != NULL) {
            entry_t *next = cursor->next;
            const unsigned long hash_key = ht->hash_function(cursor->key);
            const int bucket = hash_key % no_buckets_new;
            
            entry_t *entry = find_previous_entry_for_key(ht->hash_function, &buckets_new[bucket], hash_key);
            cursor->next = entry->next;
            entry->next = cursor;
            cursor = next;
        }
      }
  
      free(ht->buckets);
      ht->buckets = buckets_new;
      ht->no_buckets = no_buckets_new;

      return ht;

    }
    else
    {
      puts("hash table resizing not possible - no more prime numbers in library!");
      return ht;
    }
  }
  return ht;
}

/** 
 * @brief Returns the number of key-value entries in a hash table.
 * @param ht Hash table operated upon.
 * @return The number of key-value entries in the hash table.
 * 
 * This operation is performed in O(1) time by returning the size field contained within the hash table itself.
 **/
size_t hash_table_size(hash_table_t *ht)
{
  return ht->size;
}

/** 
 * @brief Checks if a hash table is empty.
 * @param ht Hash table operated upon.
 * @return True if the hash table is empty, false otherwise.
 * 
 * This operated is performed by examining whether the size of the hash table is 0.
 **/
bool hash_table_is_empty(hash_table_t *ht)
{
  return hash_table_size(ht) == 0;
}

/** 
 * @brief Lookup value for key in a hash table.
 * @param ht Hash table operated upon.
 * @param key Key to lookup.
 * @param result Pointer where a found element will be stored.
 * @return True if a key was found, false otherwise.
 **/
bool hash_table_lookup(hash_table_t *ht, const elem_t key, elem_t *result)
{
  const unsigned long hash_key = ht->hash_function(key); 
  const int bucket = hash_key % ht->no_buckets;
  entry_t *first_entry = &ht->buckets[bucket];
  entry_t *cursor = first_entry->next;

  while (cursor != NULL)
    {
      const unsigned long current_hash_key = ht->hash_function(cursor->key);
      if (current_hash_key == hash_key)
        {
          *result = cursor->value;
          return true;
        }
      cursor = cursor->next;
    }

  return false;
}

/**
 * @brief Create a new hash table with a given bucket size and load factor.
 * @param no_buckets Number of buckets to store entries in. Valid numbers include 17, 31, 67, 127, 257, 509, 1021, 2053, 4099, 8191 and 16381.
 * @param load_factor Maximum load factor before hash table gets resized.
 * @param func Hash function to hash keys with.
 * @param key_comp_fun Function that determines how keys will get compared.
 * @param value_comp_fun Function that detetmines how values will get compared.
 * @return A new empty hash table, or NULL if creation failed.
 * 
 * Creation of a new hash table is done by checking wheter the given number of buckets is a valid prime number,
 * i.e. is contained within the internal library. The given load factor is also sanity checked to ensure it is 
 * larger than 0. If any of these checks fail, NULL is returned; otherwise initial memory gets allocated and 
 * starting values are set. If no hash, key or value comparison function is provided, default functions are set
 * and the hash table is assumed to operate on integer keys and values. If any memory allocation fails, a message
 * is printed and NULL is returned.
 **/
hash_table_t *hash_table_create_dynamic(const size_t no_buckets, const float load_factor, hash_function func, predicate_ht key_comp_fun, predicate_ht value_comp_fun)
{
  if (!is_number_in_prime_library(no_buckets)) 
  {
    printf("Bucket size %ld not in prime number library!\n", no_buckets);
    return NULL;
  }
  if (load_factor <= 0)
  {
    printf("Load factor must be greater than 0! Got %.2f\n", load_factor);
    return NULL;
  }

  hash_table_t *ht = calloc(1, sizeof(hash_table_t));
  if (ht == NULL)
  {
    puts("Failed to allocate memory for hash table!");
    return NULL;
  }
  
  ht->buckets = calloc(no_buckets, sizeof(entry_t));
  if (ht->buckets == NULL)
  {
    puts("Failed to allocate memory for hash table entries!");
    free(ht);
    return NULL;
  }

  ht->no_buckets = no_buckets;
  ht->load_factor = load_factor;
  ht->size = 0;
  if (func == NULL)
    {
      ht->hash_function = default_hash_function;
    }
  else
    {
      ht->hash_function = func;
    }
  if (key_comp_fun == NULL)
  {
    ht->key_equiv = default_key_equiv;
  }
  else 
  {
    ht->key_equiv = key_comp_fun;
  }
  if (value_comp_fun == NULL)
  {
    ht->value_equiv = default_value_equiv;
  }
  else {
    ht->value_equiv = value_comp_fun;
  }
  for (int i = 0; i < ht->no_buckets; ++i)
    {
      entry_t new_entry = {.key = int_elem(0), .value = int_elem(0), .next = NULL};
      ht->buckets[i] = new_entry;
    }
  
  return ht;
}

/** 
 * @brief Create a new hash table with a starting bucket size of 17 and load factor of 0.75.
 * @param func Hash function to hash keys with.
 * @param key_comp_fun Function that determines how keys will get compared.
 * @param value_comp_fun Function that detetmines how values will get compared.
 * @return A new empty hash table.
 * 
 * If no hash, key or value comparison function is provided, default functions are set
 * and the hash table is assumed to operate on integer keys and values.
 **/
hash_table_t *hash_table_create(hash_function func, predicate_ht key_comp_fun, predicate_ht value_comp_fun)
{
  return hash_table_create_dynamic(17, 0.75, func, key_comp_fun, value_comp_fun);
}

/**
 * @brief Find the previous entry for a certain key.
 * @param func Hash function.
 * @param first_entry First entry of the bucket.
 * @param key Hashed key.
 * @return A pointer to the previous entry.
 **/
static entry_t *find_previous_entry_for_key(hash_function func, entry_t *first_entry, const unsigned long key)
{
  entry_t *prev = first_entry;
  entry_t *cursor = first_entry->next;
  while (cursor != NULL)
    {
      const unsigned long hash_key = func(cursor->key);
      if (hash_key >= key)
        {
          break;
        }
      prev = cursor;
      cursor = cursor->next; /// Step forward to the next entry, and repeat loop
    }
  
  return prev;
}

/**
 * @brief Create a new entry.
 * @param key Key to associate with entry.
 * @param value Value to associate with entry.
 * @param next An entry pointer to the next entry.
 * @return A pointer to the newly created entry; possibly NULL if memory allocation failed.
 * 
 * Creation is done by attempting to allocate memory for another entry and assigning the given parameters.
 **/
static entry_t *entry_create(const elem_t key, const elem_t value, entry_t *next)
{
  entry_t *new_entry = calloc(1, sizeof(entry_t));
  if (new_entry == NULL)
  {
    puts("Failed to allocate memory for entry!");
    return NULL;
  }
  new_entry->key = key;
  new_entry->value = value;
  new_entry->next = next;

  return new_entry;
}

/**
 * @brief Insert a key-value pair entry in a hash table.
 * @param ht Hash table to insert into.
 * @param key Key to insert.
 * @param value Value to insert.
 * 
 * Before inserting a new entry, if necessary, the hash table gets resized and rehashed.
 **/
void hash_table_insert(hash_table_t *ht, const elem_t key, const elem_t value)
{
  ht = hash_table_resize(ht);
  const unsigned long hash_key = ht->hash_function(key);
  const int bucket = hash_key % ht->no_buckets;
  
  entry_t *entry = find_previous_entry_for_key(ht->hash_function, &ht->buckets[bucket], hash_key);
  entry_t *next = entry->next;

  if (next != NULL && ht->hash_function(next->key) == hash_key)
    {
      next->value = value;
    }
  else
    {
      entry_t *new_entry = entry_create(key, value, next);
      if (new_entry == NULL)
      {
        puts("Insertion failed due to memory corruption!");
        return;
      }
      entry->next = new_entry;
      ht->size += 1;
    }
}

/**
 * @brief Destroy an entry by freeing its allocated memory.
 * @param entry Entry to destroy.
 **/
static void entry_destroy(entry_t *entry)
{
  free(entry);
}

/** 
 * @brief Remove any mapping from key to a value.
 * @param ht Hash table to remove entry from.
 * @param key Key to remove.
 * @param result Pointer where a removed element will be stored.
 * @return True if a key was removed, false otherwise.
 * 
 * If an entry for the given key exists, the entry gets detatched from the linked structure then destroyed.
 **/
bool hash_table_remove(hash_table_t *ht, const elem_t key, elem_t *result)
{
  const unsigned long hash_key = ht->hash_function(key); 
  const int bucket = hash_key % ht->no_buckets;
  bool entry_found = hash_table_lookup(ht, key, result);
  
  if (!entry_found)
    {
      return false;
    }
  else
    {
      entry_t *entry = find_previous_entry_for_key(ht->hash_function, &ht->buckets[bucket], hash_key);
      entry_t *entry_to_remove = entry->next;
      entry_t *next = entry_to_remove->next;
      entry->next = next;
      entry_destroy(entry_to_remove);
      ht->size -= 1;
      return true;
    }
}

/** 
 * @brief Clear all entries in a hash table.
 * @param ht Hash table operated upon.
 * 
 * This operation is performed by detatching entries from the linked structure and destroying them.
 **/
void hash_table_clear(hash_table_t *ht)
{
  for (int i = 0; i < ht->no_buckets; ++i)
    {
      entry_t *first_entry = &ht->buckets[i];
      entry_t *cursor = first_entry->next;

      while (cursor != NULL)
        {
          entry_t *entry_to_remove = cursor;
          cursor = cursor->next;
          first_entry->next = cursor;
          entry_destroy(entry_to_remove);
          ht->size -= 1;
        }
    }
}

/** 
 * @brief Delete a hash table and frees its memory.
 * @param ht Hash table to be deleted.
 * 
 * This operation is performed by clearing all entries, then deallocting memory for the entries
 * and hash table itself.
 **/ 
void hash_table_destroy(hash_table_t *ht)
{
  hash_table_clear(ht);
  free(ht->buckets);
  free(ht);
}

/**
 * @brief Dummy function pointer to be used with linked lists.
 * @param a First element.
 * @param b Second element.
 * @return True
 **/
static bool dummy_func_ptr(const elem_t a, const elem_t b)
{
  return true;
}

/** 
 * @brief Return the keys for all entries in a hash table (in no particular order, but same as hash_table_values).
 * @param ht Hash table operated upon.
 * @return A linked list of keys for hash table ht.
 **/
list_t *hash_table_keys(hash_table_t *ht)
{
  list_t *keys = linked_list_create(dummy_func_ptr);
  
  for (int i = 0; i < ht->no_buckets; ++i)
    {
      entry_t *first_entry = &ht->buckets[i];
      entry_t *cursor = first_entry->next;
      
      while (cursor != NULL)
        {
          linked_list_append(keys, cursor->key);
          cursor = cursor->next;
        }
    }

  return keys;
}

/** 
 * @brief Return the values for all entries in a hash table (in no particular order, but same as hash_table_keys).
 * @param ht Hash table operated upon.
 * @return A linked list of values for hash table ht.
 **/
list_t *hash_table_values(hash_table_t *ht)
{
  list_t *values = linked_list_create(dummy_func_ptr);
  
  for (int i = 0; i < ht->no_buckets; ++i)
    {
      entry_t *first_entry = &ht->buckets[i];
      entry_t *cursor = first_entry->next;
      
      while (cursor != NULL)
        {
          linked_list_append(values, cursor->value);
          cursor = cursor->next;
        }
    }
 
  return values;
}

/**
 * @brief Check if a hash table has an entry with a given key.
 * @param ht Hash table operated upon.
 * @param key The key sought.
 * 
 * This operation is performed by examining whether any entry in the hash table satisfy the provided
 * key equivalence function.
 **/
bool hash_table_has_key(hash_table_t *ht, const elem_t key)
{
  return hash_table_any(ht, ht->key_equiv, &key);
}

/**
 * @brief Check if a hash table has an entry with a given value.
 * @param ht Hash table operated upon.
 * @param value The value sought.
 * 
 * This operation is performed by examining whether any entry in the hash table satisfy the provided
 * value equivalence function.
 **/
bool hash_table_has_value(hash_table_t *ht, const elem_t value)
{
  return hash_table_any(ht, ht->value_equiv, &value);
}

/**
 * @brief Check if all keys in a hash table satisfy some property.
 * @param ht Hash table operated upon.
 * @param P Function to pass keys and values to.
 * @param x Optional additional data.
 **/
bool hash_table_all(hash_table_t *ht, predicate_ht P, const void *x)
{
  const size_t size = hash_table_size(ht);
  list_t *keys = hash_table_keys(ht);
  list_t *values = hash_table_values(ht);
  bool result = true;
  for (size_t i = 0; i < size && result; ++i)
    {
      result = result && P(linked_list_get(keys, i), linked_list_get(values, i), x);
    }

  linked_list_destroy(keys);
  linked_list_destroy(values);
  
  return result;
}

/** 
 * @brief Check if at least one key in a hash table satisfy some property.
 * @param ht Hash table operated upon.
 * @param P Function to pass keys and values to.
 * @param x Optional additional data.
 **/
bool hash_table_any(hash_table_t *ht, predicate_ht P, const void *x)
{
  const size_t size = hash_table_size(ht);
  list_t *keys = hash_table_keys(ht);
  list_t *values = hash_table_values(ht);
  bool result = false;
  for (size_t i = 0; i < size && !result; ++i)
    {
      result = !result && P(linked_list_get(keys, i), linked_list_get(values, i), x);
    }
  linked_list_destroy(keys);
  linked_list_destroy(values);

  return result;
}

/** 
 * @brief Apply some property to all entries in a hash table.
 * @param ht Hash table operated on.
 * @param P Function to pass keys and values to.
 * @param x Optional additional data.
 * 
 * NOTE: This implementation does not utilize hash_table_values as the instructions are a bit unclear.
 **/
void hash_table_apply_to_all(hash_table_t *ht, apply_function_ht f, const void *x)
{
  const size_t size = hash_table_size(ht);
  list_t *keys = hash_table_keys(ht);
  elem_t **values = hash_table_values_arr(ht);

  for (size_t i = 0; i < size; ++i)
    {
      elem_t key = linked_list_get(keys, i);
      elem_t *value = values[i];
      f(key, value, x);
    }

  linked_list_destroy(keys);
  free(values);
}

/** 
 * @brief Return the values for all entries in a hash table (in no particular order, but same as hash_table_keys).
 * @param ht Hash table operated upon.
 * @return An array of values for hash table ht.
 * 
 * NOTE: This internal function exists because the instructions are a bit unclear for how things are supposed 
 * to work with hash_table_apply_to_all.
 **/
static elem_t **hash_table_values_arr(hash_table_t *ht)
{
  const size_t size = hash_table_size(ht);
  elem_t **values = calloc(1, size * sizeof(elem_t));
  for (int i = 0; i < ht->no_buckets; ++i)
    {
      entry_t *first_entry = &ht->buckets[i];
      entry_t *cursor = first_entry->next;
      size_t j = 0;
      while (cursor != NULL)
        {
          values[i + j] = &cursor->value;
          cursor = cursor->next;
          j += 1;
        }
    }
 
  return values;
}
