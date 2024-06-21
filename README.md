# my-memalloc-c

### About
* A simple memory allocator in C => malloc(), calloc(), realloc() and free().
* It uses sbrk() system call to manage the program break for memory allocation.
* malloc() allocates memory by incrementing the program break and managing a linked list of memory blocks.
* free() marks blocks as free and potentially releases memory back to the OS.
* calloc() allocates zero-initialized memory.
* realloc() resizes memory blocks, copying data to new blocks if necessary.

### Notes
* Why sbrk(0) and sbrk(size) both return the same address?
  * When you use sbrk(0) you get the current "break" address.
  * When you use sbrk(size) you get the previous "break" address, i.e. the one before the change.

### Memory layout of a program.
 * ![image](https://github.com/ashwaniYDV/memory-alloc-c/assets/43786728/d29c27a4-1b7a-47cd-bb5e-cb6b701188f9)
 * A process runs within its own virtual address space distinct from other processes' virtual address spaces.
 * This virtual address space typically comprises of 5 sections:
   * Text section: The part that contains the binary instructions to be executed by the processor.
   * Data section: Contains non-zero initialized static data.
   * BSS (Block Started by Symbol) : Contains zero-initialized static data. Static data uninitialized in program is initialized 0 and goes here.
   * Heap: Contains the dynamically allocated data.
   * Stack: Contains your automatic variables, function arguments, copy of base pointer etc.
 * As you can see in the image, the stack and the heap grow in the opposite directions. Sometimes the data, bss and heap sections are collectively referred to as the “data segment”, the end of which is demarcated by a pointer named program break or **brk**. That is, brk points to the end of the heap.
 * Now if we want to allocate more memory in the heap, we need to request the system to increment brk. Similarly, to release memory we need to request the system to decrement brk.
 * Assuming we run Linux (or a Unix-like system), we can make use of sbrk() system call that lets us manipulate the program break.
   * Calling sbrk(0) gives the current address of program break.
   * Calling sbrk(x) with a positive value increments brk by x bytes, as a result allocating memory.
   * Calling sbrk(-x) with a negative value decrements brk by x bytes, as a result releasing memory.
   * On failure, sbrk() returns (void*) -1.


### References
* https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory
* https://stackoverflow.com/questions/16036595/sbrk0-and-sbrksize-both-return-the-same-address
* https://stackoverflow.com/questions/11130109/c-struct-size-alignment