#include <unistd.h>
#include <string.h>
#include <pthread.h>
/* Only for the debug printf */
#include <stdio.h>

/*
We can use __attribute__((aligned(16))) syntax to ensure that header_t structure is aligned to 16 bytes

Header's memory address is aligned to 16 bytes. It is efficient and also helps us in pointer arithmetic.
The end of the header is where the actual memory block begins.
Therefore memory provided to the caller by the allocator will be aligned to 16 bytes.
*/
struct header_t
{
	size_t size;
	unsigned is_free;
	struct header_t *next;
} __attribute__((aligned(16)));

header_t *head = NULL, *tail = NULL;
pthread_mutex_t global_malloc_lock;

/* function to get first free block using first-fit approach */
header_t *get_free_block(size_t size)
{
	header_t *curr = head;
	while (curr)
	{
		/* see if there's a free block that can accomodate requested size */
		if (curr->is_free && curr->size >= size)
			return curr;
		curr = curr->next;
	}
	return NULL;
}

void *malloc(size_t size)
{
	if (!size)
		return NULL;

	pthread_mutex_lock(&global_malloc_lock);
	header_t *header = get_free_block(size);
	if (header)
	{
		/* Woah, found a free block to accomodate requested memory. */
		header->is_free = 0;
		pthread_mutex_unlock(&global_malloc_lock);
		return (void *)(header + 1);
	}

	/* We need to get memory to fit in the requested block and header from OS. */
	size_t total_size = sizeof(header_t) + size;
	void *block = sbrk(total_size);
	/* On failure, sbrk() system call returns (void*) -1. */
	if (block == (void *)-1)
	{
		pthread_mutex_unlock(&global_malloc_lock);
		return NULL;
	}

	header = (header_t *)block;
	header->size = size;
	header->is_free = 0;
	header->next = NULL;
	if (!head)
		head = header;
	if (tail)
		tail->next = header;
	tail = header;
	pthread_mutex_unlock(&global_malloc_lock);

	return (void *)(header + 1);
}

void free(void *block)
{
	if (!block)
		return;

	pthread_mutex_lock(&global_malloc_lock);
	header_t *header = (header_t *)block - 1;

	/* program break is the end of the process's data segment */
	/* sbrk(0) gives the current program break address */
	void *programbreak = sbrk(0);

	/*
	   Check if the block to be freed is the last one in the linked list.
	   If it is, then we could shrink the size of the heap and release memory to OS.
	   Else, we will keep the block but mark it as free.
	 */
	if ((char *)block + header->size == programbreak)
	{
		if (head == tail)
		{
			head = tail = NULL;
		}
		else
		{
			header_t *tmp = head;
			while (tmp)
			{
				if (tmp->next == tail)
				{
					tmp->next = NULL;
					tail = tmp;
				}
				tmp = tmp->next;
			}
		}
		/*
		   sbrk() with a negative argument decrements the program break.
		   So memory is released by the program to OS.
		*/
		sbrk(0 - header->size - sizeof(header_t));

		/* Note: This lock does not really assure thread safety,
		because sbrk() itself is not really thread safe.
		Suppose there occurs a foregin sbrk(N) after we find the program break and before we decrement it,
		then we end up realeasing the memory obtained by the foreign sbrk().
		*/
		pthread_mutex_unlock(&global_malloc_lock);
		return;
	}

	/* we will keep the block but mark it as free */
	header->is_free = 1;
	pthread_mutex_unlock(&global_malloc_lock);
}

void *calloc(size_t num, size_t nsize)
{
	if (!num || !nsize)
		return NULL;

	size_t size = num * nsize;

	/* check mul overflow */
	if (nsize != size / num)
		return NULL;

	void *block = malloc(size);

	if (!block)
		return NULL;

	memset(block, 0, size);
	return block;
}

void *realloc(void *block, size_t size)
{
	if (!block || !size)
		return malloc(size);

	header_t *header = (header_t *)block - 1;
	if (header->size >= size)
		return block;

	void *ret = malloc(size);
	if (ret)
	{
		/* Relocate contents to the new bigger block */
		memcpy(ret, block, header->size);
		/* Free the old memory block */
		free(block);
	}
	return ret;
}

/* A debug function to print the entire link list */
void print_mem_list()
{
	header_t *curr = head;
	printf("head = %p, tail = %p \n", (void *)head, (void *)tail);
	while (curr)
	{
		printf("addr = %p, size = %zu, is_free=%u, next=%p\n",
			   (void *)curr, curr->size, curr->is_free, (void *)curr->next);
		curr = curr->next;
	}
}