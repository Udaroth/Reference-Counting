# Reference-Counting

You are to develop a reference counting garbage collector library that can be used by other developers. The overall goal of this library is to make memory allocation and deallocation easier for programmers. Assuming they abide by the rules of the library and do not make any allocations outside of this library, allocations should be completely managed by this library.

This library will be used as a replacement for manual memory allocation. One drawback of this library used in a context of C, is that all calls are explicit, so allocations can potentially be lost or you could wind up with an invalid count. Your library will have explicit calls to alloc, upgrade and downgrade.

Your references are broken up into two different types, Strong references and Weak references. A strong reference has direct access to the reference counter entry and the memory allocation, while a weak reference will need to retrieve the strong reference (upgrade to strong reference). A strong reference can be downgraded to a weak reference via downgrade and a weak reference can be upgraded to a strong reference via upgrade. 

WARNING: If your .bss section is found to be larger than 256 bytes, your program will instantly fail this task.

