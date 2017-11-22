/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "badassteam",
    /* First member's full name */
    "Anouk Paradis",
    /* First member's email address */
    "anouk.paradis@polytechnique.edu",
    /* Second member's full name (leave blank if none) */
    "Lorraine Roulier",
    /* Second member's email address (leave blank if none) */
    "lorraine.roulier@polytechnique.edu"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 4
#define ALIGNMENT_BIS 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x3)
#define ALIGN_BIS(size) (((size) + (ALIGNMENT_BIS-1)) & ~0x7)
#define MARKED_BITS 3
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

void * heapLo;
/*
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{

    mem_sbrk((int)mem_pagesize());
    heapLo = mem_heap_lo() + 4;
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN_BIS(size + 2*SIZE_T_SIZE);
    //printf("SIZE_T %u\n ",SIZE_T_SIZE);
    //printf("pagesize %u, ",mem_pagesize());
    //printf("void * %u ", sizeof(char));
    char * heapCur = heapLo;
    char busy;
    int blockSize;
    int flag = 0;
    while((heapCur < (char *)mem_heap_hi())){
    	busy = (char)((*(int *)heapCur) & -2);
    	blockSize = ((*(int *)heapCur) >> MARKED_BITS)<< MARKED_BITS;
      	if (busy){
      	    heapCur += blockSize;
      	}
    	else{
    	    if (blockSize >= newsize){
            * heapCur = newsize | 1;
            * (heapCur + newsize - 4) = newsize | 1;
            flag = 1;
          }
	}
    }
    if (flag == 0){
      mem_sbrk((int)mem_pagesize());
      heapCur = mem_heap_hi() - mem_pagesize();
      * heapCur = newsize | 1;
      * (heapCur + newsize - 4) = newsize | 1;
      *(heapCur + newsize) = mem_pagesize() - newsize;
      *(heapCur + mem_pagesize() - 4) = mem_pagesize() - newsize;
    }
    return (void *)((char *)heapCur + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
	//printf("freeing ptr : %d\n", ptr);
	ptr = (char *) (ptr - SIZE_T_SIZE);
	
	//let's mark the block as free
	//check that it is indeed allocated :
	//TODO
	*ptr = (*ptr) & -2;
	size_t size = ((*ptr) >> MARKED_BITS) << MARKED_BITS;
	*(ptr + size) = *(ptr + size) & -;
	printf("size of block ; %u\n", size);

	//let's try to coalesce with previous block
	size_t sizePrevBlock = ((*(ptr - SIZE_T_SIZE)) >> MARKED_BITS) << MARKED_BITS;
	char* prevBlock = ptr - sizePrevBlock;
	if(! (*prevBlock) & 1){//prev block is not busy
		*prevBlock = size + sizePrevBlock;
		*prevBlock = (*prevBlock) | 1;
		*(prevBlock + size + sizePrevBlock) = *prevBlock;
		ptr = prevBlock;
		size += sizePrevBlock;
	}
	
	//let's try to coalesce with next block
	size_t sizeNextBlock = ((*(ptr + size)) >> MARKED_BITS) << MARKED_BITS;
	char* nextBlock = ptr + sizeNextBlock;
	if(! (*nextBlock) & 1){//next block is not busy
		*nextBlock = size + sizeNextBlock;
		*nextBlock = (*nextBlock) | 1;
		*(nextBlock + size + sizeNextBlock) = *nextBlock;
	}	
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














