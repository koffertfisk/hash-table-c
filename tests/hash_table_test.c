#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <CUnit/Basic.h>
#include "linked_list.h"
#include "hash_table.h"

int init_suite(void)
{
  return 0;
}

int clean_suite(void)
{
  return 0;
}

unsigned long extract_int_hash_key(elem_t key)
{
  return (unsigned long) key.i; // following a very terse naming scheme -- improve?
}

unsigned long string_knr_hash(elem_t key)
{
  const char *str = (char*)key.p;
  unsigned long result = 0;
  do
    {
      result = result * 31 + *str;
    }
  while (*++str != '\0');
  return result;
}

static bool str_key_equiv(const elem_t key, const elem_t value_ignored, const void *x)
{
  return (char*)key.p == (char*)((elem_t*)x)->p; 
}

static bool str_value_equiv(const elem_t key_ignored, const elem_t value, const void *x)
{
  return (char*)value.p == (char*)((elem_t*)x)->p; 
}

void test_create_destroy()
{
  hash_table_t *ht = hash_table_create(extract_int_hash_key, NULL, NULL);
  CU_ASSERT_PTR_NOT_NULL(ht);
  hash_table_destroy(ht);
}

void test_lookup_int()
{
  hash_table_t *ht = hash_table_create(extract_int_hash_key, NULL, NULL);
  for (int i = 0; i < 17; ++i)
    {
      elem_t *result = calloc(1, sizeof(elem_t));
      CU_ASSERT_FALSE(hash_table_lookup(ht, int_elem(i), result));
      free(result);
    }
  elem_t *result = calloc(1, sizeof(elem_t));
  CU_ASSERT_FALSE(hash_table_lookup(ht, int_elem(-1), result));
  free(result);
  hash_table_destroy(ht);
}

void test_lookup_str()
{
  hash_table_t *ht = hash_table_create(string_knr_hash, NULL, NULL);
  for (int i = 0; i < 17; ++i)
    {
      elem_t *result = calloc(1, sizeof(elem_t));
      char key[3];
      sprintf(key, "%d", i);     
      CU_ASSERT_FALSE(hash_table_lookup(ht, ptr_elem(key), result));
      free(result);
    }
  elem_t *result = calloc(1, sizeof(elem_t));
  CU_ASSERT_FALSE(hash_table_lookup(ht, ptr_elem("not present"), result));
  free(result);
  hash_table_destroy(ht);
}

void test_lookup()
{
  test_lookup_int();
  test_lookup_str();
}

void test_lookupinsert_int()
{
  hash_table_t *ht = hash_table_create(extract_int_hash_key, NULL, NULL);
  elem_t *result = calloc(1, sizeof(elem_t));
  CU_ASSERT_FALSE(hash_table_lookup(ht, int_elem(1), result));
  hash_table_insert(ht, int_elem(1), ptr_elem("test"));
  bool found = hash_table_lookup(ht, int_elem(1), result);
  CU_ASSERT(found);
  CU_ASSERT(strcmp("test", (char*)result->p) == 0);
  free(result);
  hash_table_destroy(ht);
}

void test_insert_same_bucket()
{
  const size_t bucket_size = 17;
  hash_table_t *ht = hash_table_create_dynamic(bucket_size, 0.75, NULL, NULL, NULL);
  hash_table_insert(ht, int_elem(0), int_elem(0));
  hash_table_insert(ht, int_elem(17), int_elem(17));
  elem_t *result = calloc(1, sizeof(elem_t));
  bool found = hash_table_lookup(ht, int_elem(17), result);
  CU_ASSERT(found);
  free(result);
  hash_table_destroy(ht);
}

void test_lookupinsert_str()
{
  hash_table_t *ht = hash_table_create(string_knr_hash, NULL, NULL);
  elem_t *result = calloc(1, sizeof(elem_t));
  CU_ASSERT_FALSE(hash_table_lookup(ht, ptr_elem("one"), result));
  hash_table_insert(ht, ptr_elem("one"), ptr_elem("test"));
  bool found = hash_table_lookup(ht, ptr_elem("one"), result);
  CU_ASSERT(found);
  CU_ASSERT(strcmp("test", (char*)result->p) == 0);
  free(result);
  hash_table_destroy(ht);
}

void test_lookupinsert_negative_key()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  elem_t *result = calloc(1, sizeof(elem_t));
  CU_ASSERT_FALSE(hash_table_lookup(ht, int_elem(-4), result));
  hash_table_insert(ht, int_elem(-4), ptr_elem("test"));
  bool found = hash_table_lookup(ht, int_elem(-4), result);
  CU_ASSERT(found);
  CU_ASSERT(strcmp("test", (char*)result->p) == 0);
  free(result);
  hash_table_destroy(ht);
  }

void test_lookupinsert_key_in_use()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  hash_table_insert(ht, int_elem(1), int_elem(1));
  hash_table_insert(ht, int_elem(1), int_elem(2));
  elem_t *result = calloc(1, sizeof(elem_t));
  CU_ASSERT(hash_table_lookup(ht, int_elem(1), result));
  CU_ASSERT(result->i == 2);
  free(result);
  hash_table_destroy(ht);
}

void test_remove_invalid_key()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  elem_t *result = calloc(1, sizeof(elem_t));
  bool success = hash_table_remove(ht, int_elem(1), result);
  CU_ASSERT_FALSE(success);
  free(result);
  hash_table_destroy(ht);
}

void test_remove_lookup()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  hash_table_insert(ht, int_elem(1), ptr_elem("test"));
  elem_t *result = calloc(1, sizeof(elem_t));
  bool success = hash_table_remove(ht, int_elem(1), result);
  CU_ASSERT(success);
  CU_ASSERT(strcmp((char*)result->p, "test") == 0);
  CU_ASSERT_FALSE(hash_table_lookup(ht, int_elem(1), result));
  free(result);
  hash_table_destroy(ht);
}

void test_remove_lookup_middle_key()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  hash_table_insert(ht, int_elem(1), ptr_elem("first"));
  hash_table_insert(ht, int_elem(2), ptr_elem("second"));
  hash_table_insert(ht, int_elem(3), ptr_elem("third"));
  elem_t *result = calloc(1, sizeof(elem_t));
  bool success = hash_table_remove(ht, int_elem(2), result);
  CU_ASSERT(success);
  CU_ASSERT(strcmp((char*)result->p, "second") == 0);
  CU_ASSERT(hash_table_lookup(ht, int_elem(1), result));
  CU_ASSERT_FALSE(hash_table_lookup(ht, int_elem(2), result));
  CU_ASSERT(hash_table_lookup(ht, int_elem(3), result));
  free(result);
  hash_table_destroy(ht);
}

void test_remove_all()
{
  const size_t bucket_size = 17;
  hash_table_t *ht = hash_table_create_dynamic(17, 0.75, NULL, NULL, NULL);
  
  for (int i = 0; i < bucket_size; i++)
    {
      hash_table_insert(ht, int_elem(i), int_elem(i));
    }

  for (int i = 0; i < bucket_size; i++)
    {
      elem_t *result = calloc(1, sizeof(elem_t));
      bool success = hash_table_remove(ht, int_elem(i), result);
      CU_ASSERT(success);
      free(result);
    }
  
  CU_ASSERT(hash_table_size(ht) == 0);
  hash_table_destroy(ht);
}

void test_size()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  int number_of_entries = 3;
  for (int i = 0; i < number_of_entries; ++i)
    {
      hash_table_insert(ht, int_elem(i), int_elem(i));
    }
  
  size_t size = hash_table_size(ht);
  CU_ASSERT(size == (size_t)number_of_entries);
  elem_t *result = calloc(1, sizeof(elem_t));
  hash_table_remove(ht, int_elem(2), result);
  size = hash_table_size(ht);
  CU_ASSERT(size == (size_t)number_of_entries - 1);
  free(result);
  hash_table_destroy(ht);
}

void test_hash_table_is_empty_true()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  CU_ASSERT_TRUE(hash_table_is_empty(ht))
  hash_table_destroy(ht);
}

void test_hash_table_is_empty_false()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  hash_table_insert(ht, int_elem(1), int_elem(1));
  CU_ASSERT_FALSE(hash_table_is_empty(ht));
  hash_table_destroy(ht);
}

void test_clear()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);

  int number_of_entries = 3;
  for (int i = 0; i < number_of_entries; ++i)
    {
      hash_table_insert(ht, int_elem(i), int_elem(i));
    }

  hash_table_clear(ht);
  CU_ASSERT_TRUE(hash_table_is_empty(ht))
  hash_table_destroy(ht);
}

void test_get_values()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  int number_of_entries = 3;
  for (int i = 0; i < number_of_entries; ++i)
    {
      char buffer[2];
      sprintf(buffer, "%d", i);
      hash_table_insert(ht, int_elem(i), ptr_elem(buffer));
    }

  list_t *result = hash_table_values(ht);
  
  for (int i = 0; i < number_of_entries; ++i)
    {
      char buffer[2];
      sprintf(buffer, "%d", i);
      CU_ASSERT(strcmp((char*)linked_list_get(result, i).p, buffer) == 0);
    }

  linked_list_destroy(result);
  hash_table_destroy(ht);
}

void test_has_key()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  int key = 1;
  char *value = "test";
  hash_table_insert(ht, int_elem(key), ptr_elem(value));
  CU_ASSERT(hash_table_has_key(ht, int_elem(key)));
  CU_ASSERT_FALSE(hash_table_has_key(ht, int_elem(2)));
  hash_table_destroy(ht);
}

void test_has_value()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  int key = 1;
  int value = 1;
  hash_table_insert(ht, int_elem(key), int_elem(value));
  CU_ASSERT(hash_table_has_value(ht, int_elem(value)));
  CU_ASSERT_FALSE(hash_table_has_value(ht, int_elem(2)));
  hash_table_destroy(ht);
}

static bool int_key_less(const elem_t key, const elem_t value_ignored, const void *x)
{
  return key.i < ((elem_t *)x)->i;
}

static bool int_value_equiv(const elem_t key_ignored, const elem_t value, const void *x)
{
  return value.i == ((elem_t*)x)->i;
}

static bool int_key_equiv(const elem_t key, const elem_t value_ignored, const void *x)
{
  return key.i == ((elem_t*)x)->i;
}

void test_hash_table_all()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  int number_of_entries = 3;
  for (int i = 0; i < number_of_entries; ++i)
    {
      hash_table_insert(ht, int_elem(i), int_elem(i));
    }
  
  int not_greater_than = 4;
  CU_ASSERT(hash_table_all(ht, int_key_less, &not_greater_than));
  not_greater_than = 1;
  CU_ASSERT_FALSE(hash_table_all(ht, int_key_less, &not_greater_than));

  hash_table_destroy(ht);
}

void test_hash_table_any()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  int number_of_entries = 3;
  for (int i = 0; i < number_of_entries; ++i)
    {
      hash_table_insert(ht, int_elem(i), int_elem(i));
    }
  elem_t key_equal_to = int_elem(2);
  CU_ASSERT(hash_table_any(ht, int_key_equiv, &key_equal_to));

  key_equal_to = int_elem(4);
  CU_ASSERT_FALSE(hash_table_any(ht, int_key_equiv, &key_equal_to));
  
  hash_table_destroy(ht);
}

static void set_value(const elem_t key, elem_t *value, const void *extra)
{
  *value = *((elem_t*)extra);
}

void test_hash_table_apply_to_all()
{
  hash_table_t *ht = hash_table_create(NULL, NULL, NULL);
  int number_of_entries = 3;
  for (int i = 0; i < number_of_entries; ++i)
    {
      hash_table_insert(ht, int_elem(i), int_elem(i));
    }
  
  int new = 4;
  elem_t new_elem = int_elem(new);
  hash_table_apply_to_all(ht, set_value, &new_elem);
  CU_ASSERT(hash_table_all(ht, int_value_equiv, &new));
  
  hash_table_destroy(ht);
}

void test_lookupinsert_multiple_str()
{
  hash_table_t *ht = hash_table_create(string_knr_hash, NULL, NULL);
  
  elem_t *result = calloc(1, sizeof(elem_t));

  CU_ASSERT_FALSE(hash_table_lookup(ht, ptr_elem("A"), result));
  CU_ASSERT_FALSE(hash_table_lookup(ht, ptr_elem("B"), result));
  CU_ASSERT_FALSE(hash_table_lookup(ht, ptr_elem("C"), result));

  hash_table_insert(ht, ptr_elem("A"), ptr_elem("0"));
  hash_table_insert(ht, ptr_elem("B"), ptr_elem("1"));
  hash_table_insert(ht, ptr_elem("C"), ptr_elem("2"));

  bool found = hash_table_lookup(ht, ptr_elem("A"), result);
  CU_ASSERT(found);
  CU_ASSERT(strcmp("0", (char*)result->p) == 0);

  found = hash_table_lookup(ht, ptr_elem("B"), result);
  CU_ASSERT(found);
  CU_ASSERT(strcmp("1", (char*)result->p) == 0);
  
  found = hash_table_lookup(ht, ptr_elem("C"), result);
  CU_ASSERT(found);
  CU_ASSERT(strcmp("2", (char*)result->p) == 0);
  
  free(result);
  hash_table_destroy(ht);
}

void test_hash_table_create_dynamic()
{
  hash_table_t *ht = hash_table_create_dynamic(0, 0.5, NULL, NULL, NULL);
  CU_ASSERT_PTR_NULL(ht);
  
  ht = hash_table_create_dynamic(17, 0.75, NULL, NULL, NULL);
  hash_table_insert(ht, int_elem(1), ptr_elem("test"));
  hash_table_insert(ht, int_elem(2), ptr_elem("test 2"));
  elem_t *result = calloc(1, sizeof(elem_t));
  bool found = hash_table_lookup(ht, int_elem(1), result);
  CU_ASSERT(found);
  free(result);

  hash_table_destroy(ht);
}

void test_hash_table_resize()
{
  const size_t bucket_size = 17;
  hash_table_t *ht = hash_table_create_dynamic(bucket_size, 0.75, NULL, NULL, NULL);

  int num_of_entries = 100;
  for (int i = 0; i < num_of_entries; i++)
  {
    hash_table_insert(ht, int_elem(i), int_elem(i));
  }

  elem_t *result = calloc(1, sizeof(elem_t));
  for (int i = 0; i < num_of_entries; i++)
  {
    CU_ASSERT(hash_table_lookup(ht, int_elem(i), result));
  }
  free(result);
  
  hash_table_destroy(ht);
}

void test_hash_table_resize_not_possible()
{  
  const size_t bucket_size = 16381;
  hash_table_t *ht = hash_table_create_dynamic(bucket_size, 0.01, NULL, NULL, NULL);

  int num_of_entries = 165;
  for (int i = 0; i < num_of_entries; i++)
  {
    hash_table_insert(ht, int_elem(i), int_elem(i));
  }
  
  CU_ASSERT(hash_table_size(ht) == 165);

  hash_table_destroy(ht);
}

void test_keys_and_values()
{
  const size_t bucket_size = 17;
  hash_table_t *ht = hash_table_create_dynamic(bucket_size, 0.75, NULL, NULL, NULL);
  
  for (int i = 0; i < bucket_size; i++)
    {
      hash_table_insert(ht, int_elem(i), int_elem(i));
    }

  list_t *keys = hash_table_keys(ht);
  list_t *values = hash_table_values(ht);
  puts("*** clearing hash table ***");
  hash_table_clear(ht);

  for (int i = 0; i < bucket_size; i++)
    {
      int key = (int)linked_list_get(keys, i).i;
      int value = (int)linked_list_get(values, i).i;
      CU_ASSERT(key == i);
      CU_ASSERT(value == i);
    }  

  linked_list_destroy(keys);
  linked_list_destroy(values);
  hash_table_destroy(ht);
}

void test_key_and_value_equiv()
{
  const size_t bucket_size = 17;
  hash_table_t *ht = hash_table_create_dynamic(bucket_size, 0.75, string_knr_hash, str_key_equiv, str_value_equiv);
  hash_table_insert(ht, ptr_elem("one"), ptr_elem("test"));
  CU_ASSERT_TRUE(hash_table_has_key(ht, ptr_elem("one")));
  CU_ASSERT_TRUE(hash_table_has_value(ht, ptr_elem("test")));
  hash_table_destroy(ht);
}

int main()
{
  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  CU_pSuite creation = CU_add_suite("Creation", NULL, NULL);
  CU_pSuite size = CU_add_suite("Size", NULL, NULL);
  CU_pSuite retrieval = CU_add_suite("Retrieval", NULL, NULL);
  CU_pSuite insertion = CU_add_suite("Insertion", NULL, NULL);
  CU_pSuite removal = CU_add_suite("Removal", NULL, NULL);
  CU_pSuite keys_and_values = CU_add_suite("Keys And Values", NULL, NULL);
  CU_pSuite function_application = CU_add_suite("Function Application", NULL, NULL);
  CU_pSuite resize_and_rehash = CU_add_suite("Resize And Rehash", NULL, NULL);
  
  CU_add_test(creation, "Creation", test_create_destroy);
  CU_add_test(creation, "Creation Dynamic", test_hash_table_create_dynamic);
  CU_add_test(creation, "Clear", test_clear);

  CU_add_test(size, "Size", test_size);
  CU_add_test(size, "Is Empty", test_hash_table_is_empty_true);
  CU_add_test(size, "Is Empty False", test_hash_table_is_empty_false);

  CU_add_test(retrieval, "Lookup", test_lookup);
  CU_add_test(retrieval, "Has Key", test_has_key);
  CU_add_test(retrieval, "Has Value", test_has_value);

  CU_add_test(insertion, "Insert Integer", test_lookupinsert_int);
  CU_add_test(insertion, "Insert String", test_lookupinsert_str);
  CU_add_test(insertion, "Insert Key Already In Use", test_lookupinsert_key_in_use );
  CU_add_test(insertion, "Insert Negative Key", test_lookupinsert_negative_key);
  CU_add_test(insertion, "Insert Multiple", test_lookupinsert_multiple_str);
  CU_add_test(insertion, "Insert Same Bucket", test_insert_same_bucket);

  CU_add_test(removal, "Remove Invalid Key", test_remove_invalid_key);
  CU_add_test(removal, "Remove", test_remove_lookup);
  CU_add_test(removal, "Remove Middle Key", test_remove_lookup_middle_key);
  CU_add_test(removal, "Remove All", test_remove_all);

  CU_add_test(keys_and_values, "Keys And Values", test_keys_and_values);

  CU_add_test(function_application, "Key And Value Equiv", test_key_and_value_equiv);
  CU_add_test(function_application, "All", test_hash_table_all);
  CU_add_test(function_application, "Any", test_hash_table_any);
  CU_add_test(function_application, "Apply To All", test_hash_table_apply_to_all);

  CU_add_test(resize_and_rehash, "Resize", test_hash_table_resize);
  CU_add_test(resize_and_rehash, "Resizing Not Possible", test_hash_table_resize_not_possible);

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  return CU_get_error();
}
