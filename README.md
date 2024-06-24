# Hash Table C
A hash table implementation in C. 

## Dependencies
- `gcc` compiler
- [Linked List C](https://github.com/koffertfisk/linked-list-c) (submodule)
- `CUnit` (in order to run unit tests)
- `valgrind` (in order to run memory tests)


## Building
To compile and run:
-  `make` to build a simple word frequency count application
-  `make submodules` to initialize linked list submodule
-  `make test` to build and run unit test suite
-  `make hash_table` to build hash table
-  `make memtest` to memory test hash table
-  `make test_coverage` to produce code coverage reports for the hash table test package
-  `make clean` to remove compiled output files and directory

### Code Coverage Reports
To generate and view test coverage reports, call `make test_coverage` then navigate to `hash_table-lcov` and open `index.html` in your web browser of choice. 