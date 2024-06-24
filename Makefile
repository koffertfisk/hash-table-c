C_COMPILER       = gcc
C_OPTIONS        = -Wall -pedantic -g -Iinclude -Ilinked_list/include
C_LINK_OPTIONS   = -lm
CUNIT_LINK       = -lcunit
C_COV            = -fprofile-arcs -ftest-coverage
LFLAGS           = -lgcov --coverage
GCOV             = gcov
LCOV             = lcov
COV_HTML         = genhtml
VALGRIND         = valgrind
VALGRIND_FLAGS   = --leak-check=full

OBJ_DIR          = obj
SRC_DIR          = src
INCLUDE_DIR      = include
TESTS_DIR        = tests
LINKED_LIST_DIR  = linked_list

HASH_TABLE_SRCS  = $(SRC_DIR)/hash_table.c
HASH_TABLE_OBJS  = $(HASH_TABLE_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
LINKED_LIST_OBJS = $(LINKED_LIST_DIR)/$(OBJ_DIR)/linked_list.o


all: submodule_init linked_list hash_table

submodule_init:
	@if [ ! -d "$(LINKED_LIST_DIR)" ]; then \
		git submodule update --init --recursive; \
	fi

linked_list: submodule_init
	$(MAKE) -C $(LINKED_LIST_DIR)

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(C_COMPILER) $(C_OPTIONS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TESTS_DIR)/%.c | $(OBJ_DIR)
	$(C_COMPILER) $(C_OPTIONS) -c $< -o $@

hash_table: $(HASH_TABLE_OBJS) $(LINKED_LIST_OBJS)
	$(C_COMPILER) $^ -o $@ $(C_LINK_OPTIONS) -c

hash_table_test: $(OBJ_DIR)/hash_table_test.o $(HASH_TABLE_OBJS) $(LINKED_LIST_OBJS)
	$(C_COMPILER) $^ -o $@ $(CUNIT_LINK) $(C_LINK_OPTIONS)

memtest: all hash_table_test
	$(VALGRIND) $(VALGRIND_FLAGS) ./hash_table_test 

test: all hash_table_test
	./hash_table_test

test_coverage: clean all
	$(C_COMPILER) $(C_OPTIONS) $(LFLAGS) -c $(TESTS_DIR)/hash_table_test.c -o $(OBJ_DIR)/hash_table_test.o
	$(C_COMPILER) $(C_OPTIONS) $(LFLAGS) -c $(SRC_DIR)/hash_table.c -o $(OBJ_DIR)/hash_table.o
	$(C_COMPILER) $(OBJ_DIR)/hash_table_test.o $(OBJ_DIR)/hash_table.o $(LINKED_LIST_DIR)/$(OBJ_DIR)/linked_list.o -o hash_table_test $(CUNIT_LINK) $(LFLAGS)
	./hash_table_test
	$(GCOV) $(TESTS_DIR)/hash_table_test.c $(SRC_DIR)/hash_table.c
	$(GCOV) -abcfu $(SRC_DIR)/hash_table.c
	$(LCOV) -c -d . -o hash_table.info
	$(COV_HTML) hash_table.info -o hash_table-lcov

clean:
	$(MAKE) -C $(LINKED_LIST_DIR) clean
	-$(RMDIR) $(OBJ_DIR) hash_table_test hash_table
	-$(RM) *.gcda *.gcno *.info gmon.out
	-$(RMDIR) hash_table-lcov

RM = rm -f
RMDIR = rm -rf

.PHONY: all clean linked_list submodule_init hash_table memtest test test_coverage