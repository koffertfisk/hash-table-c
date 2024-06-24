#pragma once

#include <stdbool.h>
#include "linked_list.h"

/**
 * @file hash_table.h
 * @author Marcus Enderskog
 * @date 2021-04-15
 * @brief Simple hash table that maps generic keys to values.
 **/

/// @brief Represents a storage key-value pair entry which contains a generic element type.
typedef struct entry entry_t;

/// @brief Actual hash table that maps generic keys to values.
typedef struct hash_table hash_table_t;

/**
 * @brief Test if a key-value pair satisfy some condition.
 * @param key Key to operate on.
 * @param value Value to operate on.
 * @param extra Data to check against.
 * @return True if the condition is satisfied, false otherwise.
 **/
typedef bool(*predicate_ht)(const elem_t key, const elem_t value, const void *extra);

/**
 * @brief Update an entry value.
 * @param key Key to operate on.
 * @param value Value to update.
 * @param extra New data to update value with.
 * @return 
 **/ 
typedef void(*apply_function_ht)(const elem_t key, elem_t *value, const void *extra);

/**
 * @brief Hash function to hash keys with.
 * @param key Key to hash.
 **/
typedef unsigned long(*hash_function)(const elem_t key);

/** 
 * @brief Create a new hash table with a starting bucket size of 17 and load factor of 0.75.
 * @param func Hash function to hash keys with.
 * @param key_comp_fun Function that determines how keys will get compared.
 * @param value_comp_fun Function that detetmines how values will get compared.
 * @return A new empty hash table.
 **/
hash_table_t *hash_table_create(hash_function func, predicate_ht key_comp_fun, predicate_ht value_comp_fun);

/**
 * @brief Create a new hash table with a given bucket size and load factor.
 * @param no_buckets Number of buckets to store entries in. Valid numbers include 17, 31, 67, 127, 257, 509, 1021, 2053, 4099, 8191 and 16381.
 * @param load_factor Maximum load factor before hash table gets resized.
 * @param func Hash function to hash keys with.
 * @param key_comp_fun Function that determines how keys will get compared.
 * @param value_comp_fun Function that detetmines how values will get compared.
 * @return A new empty hash table, or NULL if creation failed.
 **/
hash_table_t *hash_table_create_dynamic(const size_t no_buckets, const float load_factor, hash_function func, predicate_ht key_comp_fun, predicate_ht value_comp_fun);

/** 
 * @brief Delete a hash table and frees its memory
 * @param ht Hash table to be deleted
 **/ 
void hash_table_destroy(hash_table_t *ht);

/**
 * @brief Insert a key-value pair entry in a hash table.
 * @param ht Hash table to insert into
 * @param key Key to insert
 * @param value Value to insert
 **/
void hash_table_insert(hash_table_t *ht, const elem_t key, const elem_t value);

/** 
 * @brief Lookup value for key in a hash table.
 * @param ht Hash table operated upon
 * @param key Key to lookup
 * @param result Pointer where a found element will be stored
 * @return True if a key was found, false otherwise
 **/
bool hash_table_lookup(hash_table_t *ht, const elem_t key, elem_t *result);

/** 
 * @brief Remove any mapping from key to a value
 * @param ht Hash table to remove entry from
 * @param key Key to remove
 * @param result Pointer where a removed element will be stored
 * @return True if a key was removed, false otherwise
 **/
bool hash_table_remove(hash_table_t *ht, const elem_t key, elem_t *result);

/** 
 * @brief Returns the number of key-value entries in a hash table.
 * @param ht Hash table operated upon.
 * @return The number of key-value entries in the hash table.
 **/
size_t hash_table_size(hash_table_t *ht);

/** 
 * @brief Checks if a hash table is empty.
 * @param ht Hash table operated upon.
 * @return True if the hash table is empty, false otherwise.
 **/
bool hash_table_is_empty(hash_table_t *ht);

/** 
 * @brief Clear all entries in a hash table.
 * @param ht Hash table operated upon.
 **/
void hash_table_clear(hash_table_t *ht);

/** 
 * @brief Return the keys for all entries in a hash table (in no particular order, but same as hash_table_values).
 * @param ht Hash table operated upon.
 * @return A linked list of keys for hash table ht.
 **/
list_t *hash_table_keys(hash_table_t *ht);

/** 
 * @brief Return the values for all entries in a hash table (in no particular order, but same as hash_table_keys).
 * @param ht Hash table operated upon.
 * @return A linked list of values for hash table ht.
 **/
list_t *hash_table_values(hash_table_t *ht);

/**
 * @brief Check if a hash table has an entry with a given key.
 * @param ht Hash table operated upon.
 * @param key The key sought.
 **/
bool hash_table_has_key(hash_table_t *ht, const elem_t key);

/**
 * @brief Check if a hash table has an entry with a given value.
 * @param ht Hash table operated upon.
 * @param value The value sought.
 **/
bool hash_table_has_value(hash_table_t *ht, const elem_t value);

/**
 * @brief Check if all keys in a hash table satisfy some property.
 * @param ht Hash table operated upon.
 * @param P Function to pass keys and values to.
 * @param x Optional additional data.
 **/
bool hash_table_all(hash_table_t *ht, predicate_ht P, const void *x);

/** 
 * @brief Check if at least one key in a hash table satisfy some property.
 * @param ht Hash table operated upon.
 * @param P Function to pass keys and values to.
 * @param x Optional additional data.
 **/
bool hash_table_any(hash_table_t *ht, predicate_ht P, const void *x);

/** 
 * @brief Apply some property to all entries in a hash table
 * @param ht Hash table operated on
 * @param P Function to pass keys and values to
 * @param x Optional additional data
 **/
void hash_table_apply_to_all(hash_table_t *ht, apply_function_ht f, const void *x);