#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rc.h"

typedef struct command command_t;

struct command {
	char* str;
	int (*exe)();
};


int rc_example_1() {
    int pass = 0;
    int expected = 2;

    struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL);

    *((int*)m->ptr) = 100; // Stoes at Int of 100 at ptr

    int v = *((int*)m->ptr); // Retrieves it back from the pointer

    if(v == 100) {
      pass++;
    }

    struct weak_ref w = rc_downgrade(m); 
    struct strong_ref* p = rc_upgrade(w); // Strong reference no longer exists for w, hence returning NULL
    if(p == NULL) {
        pass++;
    }

    return pass == expected;
}

int rc_example_2() {
    int pass = 0;
    int expected = 3;

    struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL);
    struct strong_ref* a = rc_alloc(m->ptr, 0, NULL);


    *((int*)m->ptr) = 100;

    struct weak_ref w = rc_downgrade(m);
    struct strong_ref* p = rc_upgrade(w);


    if(p != NULL) {
        pass++;
    }

    int v = *((int*)p->ptr);
    if(v == 100) {
      pass++;
    }
    rc_downgrade(p);
    w = rc_downgrade(a);

    p = rc_upgrade(w);
    if(p == NULL) {
        pass++;
    }

    return pass == expected;
}


void test_2(){
  struct strong_ref* m = rc_alloc(NULL, sizeof(int), NULL);
  struct strong_ref* a = rc_alloc(m->ptr, 0, NULL);
  *((int*)m->ptr) = 2;
  rc_downgrade(m);
  *((int*)a->ptr) = 0;
  rc_downgrade(a);
}

struct obj{
  void* p;
};

void test_3(){
  struct strong_ref* m = rc_alloc(NULL, sizeof(struct obj), NULL);

  ((struct obj*)m->ptr)->p = rc_alloc(NULL, sizeof(int), m);

  rc_downgrade(m);
  // We should see two deallocations

  // Now at this point they should both be deallocated
  // Attempt to reallocate to smae location?
  // rc_alloc(m->ptr, sizeof(int), NULL);
  // ((struct obj*)m->ptr)->p = rc_alloc(m->ptr, sizeof(int), m);
  // struct strong_ref* k = rc_alloc(m->ptr, 0, m);


}

command_t tests[] = {
  { "rc_example_1", rc_example_1 },
  { "rc_example_2", rc_example_2 },
};


int main(int argc, char** argv) {
  int test_n = sizeof(tests) / sizeof(command_t);

  // custom_1();

  // rc_example_2();

  test_3();


  // if(argc >= 2) {
	// 	for(int i = 0; i < test_n; i++) {
	// 		if(strcmp(argv[1], tests[i].str) == 0) {
	// 			if(tests[i].exe()) {
	// 			  fprintf(stdout, "%s Passed\n", tests[i].str);
	// 			} else {
	// 			  fprintf(stdout, "%s Failed\n", tests[i].str);
	// 			}
	// 		}
	// 	}
	// }


}