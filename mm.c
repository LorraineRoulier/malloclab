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
#define MARKED_BITS 2
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

void * heapLo;
/*
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{
		mem_sbrk((int)mem_pagesize());
		heapLo = mem_heap_lo() + 4;
		int* hLAsInt = (int*) heapLo;
		*hLAsInt = (mem_pagesize() - SIZE_T_SIZE*2);
		int* endPtr = (int*) (heapLo + mem_pagesize() - SIZE_T_SIZE*3 );
		*endPtr = mem_pagesize() - SIZE_T_SIZE*2;
		//printf("init Hl %u and endPtr %u and blocksize %u \n", heapLo,endPtr,*endPtr);
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
		printf("allocation : %u et new %u \n ", size, newsize);
		char * heapCur = heapLo;
		//printf("hL : %u, val : %d\n", heapLo, *((int *)heapLo));
		int busy = 0;
		int blockSize = 0;
		int flag = 0;
		while(heapCur < ((char*) mem_heap_hi()- SIZE_T_SIZE)){
				busy = (*((int *)heapCur)) & (~(-2));
				printf("heapCur %u memheapHi %u ",heapCur, mem_heap_hi());
				blockSize = ((*(( int *)heapCur)) >> MARKED_BITS)<< MARKED_BITS;
				printf("size : %u busy : %d\n", blockSize, busy);
				if (blockSize == 0){
					printf("blocksize 0");
					break;
				}
				if (busy){
						heapCur += blockSize;
				}
				else{
						if (blockSize >= newsize){
								flag = 1;
								break;
						}
						else
								heapCur += blockSize;
				}
		}
		if (flag == 0){
				int nb_sbrk = (newsize/mem_pagesize())+1;
				mem_sbrk((int)mem_pagesize()*nb_sbrk);
				if (busy == 0){
					heapCur -= blockSize;
					blockSize = mem_pagesize()*nb_sbrk + blockSize;
				}
				else {
					blockSize = mem_pagesize()*nb_sbrk;
					heapCur = mem_heap_hi() +1 - SIZE_T_SIZE - mem_pagesize()*nb_sbrk;
				}
		printf("create nb sbrk %u, new heapCur %u, new blocksize %u\n",nb_sbrk,heapCur,blockSize);
		}
		
		
		int* heapCAsInt = (int*) heapCur;
		* heapCAsInt = newsize | 1;
		int* endPtrAsInt = (int*) (heapCur + newsize - SIZE_T_SIZE);
		* endPtrAsInt = newsize | 1;
		
		int* emptyBegin = (int*) (heapCur + newsize);
		int* emptyEnd = (int*) (heapCur + blockSize - SIZE_T_SIZE);
		*emptyBegin = blockSize - newsize;
		*emptyEnd = blockSize - newsize;

		//printf("heapCur value : %d\n",* ((int*) heapCur));
		return (void *)((char *)heapCur + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
		printf("freeing ptr : %u\n", ptr);
		ptr = (char* ) ptr - SIZE_T_SIZE;

		//let's mark the block as free
		//check that it is indeed allocated :
		//TODO
		int* ptrAsInt = (int*) ptr;
		*ptrAsInt = *ptrAsInt & (-2);
		size_t size = (*ptrAsInt >> MARKED_BITS) << MARKED_BITS;
		int* endPtrAsInt = (int*) (ptr + size - SIZE_T_SIZE);
		*endPtrAsInt = *ptrAsInt;
		printf("size of block ; %u\n", size);

		//let's try to coalesce with previous block
		int* ptrToPrevBl = (int*) (ptr - SIZE_T_SIZE);
		size_t sizePrevBlock = ((*ptrToPrevBl) >> MARKED_BITS) << MARKED_BITS;
		int* prevBlock = (int*) (ptr - sizePrevBlock);
		printf("prevblock %u, sizeprevblock %u, busy ? %u \n",prevBlock, sizePrevBlock, ((*prevBlock) & 1));
		if(!((*prevBlock) & 1) && (ptr > mem_heap_lo()+5)){//prev block is not busy
				printf("coalese prev : \n");
				size += sizePrevBlock;
				*prevBlock = size & (-2);
				*endPtrAsInt = *prevBlock;
				ptrAsInt = prevBlock;
				ptr = ptr - sizePrevBlock;
				/**prevBlock = size + sizePrevBlock;
				 *prevBlock = (*prevBlock) | 1;
				 *(ptr + size - SIZE_T_SIZE) = *prevBlock;
				 ptr = (char*) prevBlock;
				 size += sizePrevBlock;*/
				printf("coalese prev : value coalesed %u \n", *ptrAsInt);
		}

		//let's try to coalesce with next block
		int* ptrNextBl = (int*) (ptr + size);
		size_t sizeNextBlock = ((*ptrNextBl) >> MARKED_BITS) << MARKED_BITS;
		int* endNextBlock = (int*) (ptr + sizeNextBlock + size - SIZE_T_SIZE);
		printf("nextblock %u, sizenextblock %u, busy ? %u \n",ptrNextBl, sizeNextBlock, ((*ptrNextBl) & 1));
		if(!((*ptrNextBl) & 1) && (ptr + size) < (mem_heap_hi() -5)){//next block is not busy
				size += sizeNextBlock;
				*ptrAsInt = size & (-2);
				*endNextBlock = *ptrAsInt;
				printf("coalese next : value coalesed %u \n", *ptrAsInt);
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
