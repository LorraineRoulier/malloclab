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
#define VAL(ptr) (*((int*) (ptr)))
#define SIZE(ptr) ((VAL(ptr) >> MARKED_BITS) << MARKED_BITS)
#define BUSY(ptr) (VAL(ptr) & 1)

void * heapLo;
/*
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{
		mem_sbrk((int)mem_pagesize());
		heapLo = mem_heap_lo() + SIZE_T_SIZE;
		int* hLAsInt = (int*) heapLo;
		VAL(heapLo) = mem_pagesize() - SIZE_T_SIZE*2;
		int* endPtr = (int*) (heapLo + mem_pagesize() - SIZE_T_SIZE*3 );
		VAL(heapLo + mem_pagesize() - SIZE_T_SIZE * 3) = mem_pagesize() - SIZE_T_SIZE*2;
		(*(int *)(heapLo + mem_pagesize() - SIZE_T_SIZE * 3)) = mem_pagesize() - SIZE_T_SIZE*2;
		//printf("init Hl %u and endPtr %u and blocksize %u \n", heapLo,endPtr,*endPtr);
		return 0;
}

/*
 *Checks memory state : 
 * heapLo = mem_heap_lo + SIZE_T_SIZE (for alignment)
 * for every block encountered :
 *	are the begining and end of it of the right format ?
 *	if it is free ais the previous one busy ?
 * 
 *
 */
int mm_check(void){
	if(heapLo != mem_heap_lo() + SIZE_T_SIZE){
		printf("mem heap lo is bad\n");
		return 0;
	}
	
	void* ptr = heapLo;
	int prevIsFree = 0;
	while(ptr < mem_heap_hi() - SIZE_T_SIZE){
		if(SIZE(ptr) == 0){
			printf("ptr of size 000000000000000\n");
			return 0;
		}
	
		//check if format mathces what is at the end of the block
		if(VAL(ptr) != VAL(ptr + SIZE(ptr) - SIZE_T_SIZE)){
			printf("begining and end of ptr %u (end at %u )don't match : deb %u et fin %u\n", ptr, ptr + SIZE(ptr) - SIZE_T_SIZE, VAL(ptr), VAL(ptr + SIZE(ptr) - SIZE_T_SIZE));
			printf("size : %u and busy : %u from deb\n", SIZE(ptr), BUSY(ptr));
			return 0;
		}
		//if block is free checks that prevIs not
		if(! BUSY(ptr)){
			if(prevIsFree){
				printf("lack of coalescing : at ptr %u, of val %u\n", ptr, VAL(ptr));
			}
			prevIsFree = 1;
		}
		else
			prevIsFree = 0;
		ptr += SIZE(ptr);
		}
	return 1;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{		
		int a = mm_check();
		//if (size !=0){
		int newsize = ALIGN_BIS(size + 2*SIZE_T_SIZE);
		//printf("allocation : %u et new %u \n ", size, newsize);
		char * heapCur = heapLo;
		//printf("hL : %u, val : %d\n", heapLo, *((int *)heapLo));
		int busy = 0;
		int blockSize = 0;
		int emptySpace = 0;
		int flag = 0;
		while(heapCur < ((char*) mem_heap_hi()- SIZE_T_SIZE)){
				busy = (*((int *)heapCur)) & (~(-2));
				//printf("heapCur %u memheapHi %u ",heapCur, mem_heap_hi());
				blockSize = ((*(( int *)heapCur)) >> MARKED_BITS)<< MARKED_BITS;
				//printf("ptr : %u size : %u busy : %d\n", heapCur, blockSize, busy);
				if (blockSize == 0){
						printf("BLOCKSIZE 0 --> SOMETHING IS NOT OK\n");
						break;
				}
				if (busy){
						heapCur += blockSize;
				}
				else{
						if (blockSize >= newsize){
								flag = 1;
								emptySpace = blockSize; 
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
						emptySpace = mem_pagesize()*nb_sbrk + blockSize;
				}
				else {
						emptySpace = mem_pagesize()*nb_sbrk;
						heapCur = mem_heap_hi() +1 - SIZE_T_SIZE - mem_pagesize()*nb_sbrk;
				}
				//printf("create nb sbrk %u, new heapCur %u, new blocksize %u\n",nb_sbrk,heapCur,blockSize);
		}


		//int* heapCAsInt = (int*) heapCur;
		VAL(heapCur) = newsize | 1;
		//int* endPtrAsInt = (int*) (heapCur + newsize - SIZE_T_SIZE);
		VAL(heapCur + newsize - SIZE_T_SIZE) = newsize | 1;
		if (emptySpace - newsize !=0){
				/*int* emptyBegin = (int*) (heapCur + newsize);
				int* emptyEnd = (int*) (heapCur + emptySpace - SIZE_T_SIZE);
				*emptyBegin = emptySpace - newsize;
				*emptyEnd = emptySpace - newsize;*/
				VAL(heapCur + newsize) = emptySpace - newsize;
				VAL(heapCur + emptySpace - SIZE_T_SIZE) = emptySpace - newsize;
		}
		a = mm_check();
		return (heapCur + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{		
	int a = mm_check();	
	if (ptr != NULL){
				//printf("freeing ptr : %u\n", ptr);
				ptr = ptr - SIZE_T_SIZE;

				//let's mark the block as free
				//check that it is indeed allocated :
				//TODO
				int* ptrAsInt = (int*) ptr;
				*ptrAsInt = *ptrAsInt & (-2);
				size_t size = ((*ptrAsInt) >> MARKED_BITS) << MARKED_BITS;
				int* endPtrAsInt = (int*) (ptr + size - SIZE_T_SIZE);
				*endPtrAsInt = *ptrAsInt;
				//printf("size of block ; %u\n", size);

				//let's try to coalesce with previous block
				int* ptrToPrevBl = (int*) (ptr - SIZE_T_SIZE);
				size_t sizePrevBlock = ((*ptrToPrevBl) >> MARKED_BITS) << MARKED_BITS;
				int* prevBlock = (int*) (ptr - sizePrevBlock);
				//printf("prevblock %u, sizeprevblock %u, busy ? %u \n",prevBlock, sizePrevBlock, ((*prevBlock) & 1));
				if(!((*prevBlock) & 1) && (ptr > mem_heap_lo()+5)){//prev block is not busy
						size += sizePrevBlock;
						*prevBlock = size & (-2);
						*endPtrAsInt = *prevBlock;
						ptrAsInt = prevBlock;
						ptr = ptr - sizePrevBlock;
						//printf("coalese prev : value coalesed %u \n", *ptrAsInt);
				}

				//let's try to coalesce with next block
				int* ptrNextBl = (int*) (ptr + size);
				size_t sizeNextBlock = ((*ptrNextBl) >> MARKED_BITS) << MARKED_BITS;
				int* endNextBlock = (int*) (ptr + sizeNextBlock + size - SIZE_T_SIZE);
				//printf("nextblock %u, sizenextblock %u, busy ? %u \n",ptrNextBl, sizeNextBlock, ((*ptrNextBl) & 1));
				if(!((*ptrNextBl) & 1) && (ptr + size) < (mem_heap_hi() -5)){//next block is not busy
						size += sizeNextBlock;
						*ptrAsInt = size & (-2);
						*endNextBlock = *ptrAsInt;
						//printf("coalese next : value coalesed %u \n", *ptrAsInt);
				}
		}
		a = mm_check();	
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
		int a = mm_check();
		if (ptr == NULL){
				mm_malloc(size);
		}
		else if (size == 0){
				mm_free(ptr);
		}
		int blockSize = 0;
		int busy = 0;
		int newBlockSize = ALIGN_BIS(size + 2*SIZE_T_SIZE);
		//printf("reallocating ptr %u of size %u \n ",ptr, size);
		ptr -= SIZE_T_SIZE;		
		blockSize = ((*(( int *)ptr)) >> MARKED_BITS)<< MARKED_BITS;
		busy = (*((int *)ptr)) & 1;
		//printf("old block is busy %u of size %u \n",busy,blockSize);
		if (blockSize == newBlockSize){
				return ptr;
		}
		int* newPtr = (int*) ptr;
		int* newPtrEnd =(int*) (ptr + newBlockSize - SIZE_T_SIZE);
		int* nextPtr = (int*) (ptr + blockSize);
		if (blockSize > newBlockSize){
				//printf(" newBlockSize is smaller");
				*newPtr = newBlockSize | busy;
				*newPtrEnd = newBlockSize | busy;
				//rest of the pointer freed
				//printf(" size freed : %u\n", blockSize - newBlockSize);
				*(int*)(ptr + newBlockSize) = (blockSize-newBlockSize) |1; 
				*(int*)(ptr + blockSize - SIZE_T_SIZE) = (blockSize-newBlockSize) |1;
				mm_free(ptr + newBlockSize + SIZE_T_SIZE);
		}
		if (blockSize < newBlockSize){
				//printf(" newBlockSize is bigger");
				int nextBlockSize = ((*nextPtr) >> MARKED_BITS)<< MARKED_BITS;
				int nextBusy = *nextPtr &1;
				//printf("  nextBlockSize : %u busy %d\n", nextBlockSize, nextBusy);
				if(!nextBusy && blockSize + nextBlockSize >= newBlockSize){ //we have room in the next block
						//printf("room in next block : size %u busy %u\n", nextBlockSize, *nextPtr|1);
						*newPtr = newBlockSize | 1;
						*newPtrEnd = *newPtr;
						if(blockSize + nextBlockSize < newBlockSize){
								*(int*) (ptr + newBlockSize) = (blockSize + nextBlockSize - newBlockSize) & (-2);
								*(int*) (ptr + blockSize + nextBlockSize - SIZE_T_SIZE) = *(int*) (ptr + newBlockSize);
						}
				}
				else{//we don't
						void *oldptr = ptr + SIZE_T_SIZE;
						size_t copySize;

						copySize = (*(int*) ptr) & (-2);
						ptr = mm_malloc(size);
						if (ptr == NULL)
								return NULL;
						//copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
  					memcpy(ptr, oldptr, copySize);
						mm_free(oldptr);
						return ptr;
				}
		}
		mm_check();
		return ptr + SIZE_T_SIZE;
		/*
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
		   return newptr;*/
}
