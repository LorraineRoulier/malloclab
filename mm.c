/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this approach, we use an explicit list to allocate blocks. 
 * There is 4 free bytes at the beginning and at the end of the list. 
 * Each block has its 4 first and last bytes used to enter its size (aligned to 4) and a bit declaring if it is busy. 
 * To allocate, we search for the first free block, if there is no free block large enough, 
 * we create the sufficient number of new pages and initialize the new blocks created. If the last existing page wasn't entirely used,
 * the space at the end of the last block is taken into acount for the allocation. 
 * When freeing a pointer, we try to coallese with the previous and next blocks. 
 * Realloc is implemented naively.  
 *
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
/* convert ptr to int* ptr */
#define VAL(ptr) (*((int*) (ptr)))
/* get the size of the block */
#define SIZE(ptr) ((VAL(ptr) >> MARKED_BITS) << MARKED_BITS)
/* find out if the block is busy */
#define BUSY(ptr) (VAL(ptr) & 1)
/* get the last bytes of the block (where size and busy are entered) */
#define ENDPTR(ptr) (ptr + SIZE(ptr) - SIZE_T_SIZE)

void * heapLo;
/*
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{
		mem_sbrk((int)mem_pagesize());
		heapLo = mem_heap_lo() + SIZE_T_SIZE;
		// set size of the first block
		VAL(heapLo) = mem_pagesize() - SIZE_T_SIZE*2;
		VAL(ENDPTR(heapLo)) = mem_pagesize() - SIZE_T_SIZE*2;
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
		printf("\n");
		if(heapLo != mem_heap_lo() + SIZE_T_SIZE){
				printf("error with mem heap lo\n");
				return 0;
		}

		void* ptr = heapLo;
		int prevIsFree = 0;
		while(ptr < mem_heap_hi() - SIZE_T_SIZE){
				printf("	 mm_check : ptr %u , size %u, busy %u\n", (unsigned int)ptr, (unsigned int)SIZE(ptr), (unsigned int)BUSY(ptr));
				if(SIZE(ptr) == 0){
						printf("PTR OF SIZE 0\n");
						return 0;
				}

				//check if format matches what is at the end of the block
				if(VAL(ptr) != VAL(ptr + SIZE(ptr) - SIZE_T_SIZE)){
						printf("begining and end of ptr %u (end at %u )don't match : deb %u et fin %u\n", (unsigned int)ptr, (unsigned int)(ptr + SIZE(ptr) - SIZE_T_SIZE), VAL(ptr), VAL(ptr + SIZE(ptr) - SIZE_T_SIZE));
						printf("size : %u and busy : %u from deb\n", (unsigned int)SIZE(ptr), (unsigned int)BUSY(ptr));
						return 0;
				}
				//if block is free checks that prevIs not
				if(! BUSY(ptr)){
						if(prevIsFree){
								printf("lack of coalescing : at ptr %u, of val %u\n", (unsigned int)ptr, VAL(ptr));
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
 * mm_malloc - Allocates the first free block large enough. Sets its footer and header, and those of the free block after. 
 * If there is not enough space available, creates new pages, and takes into account the last block of the old memory if it was free.
 */
void *mm_malloc(size_t size)
{		
		/*int a = mm_check();
		if(a == 0)
			printf("mem check failed begin malloc\n");*/
		int newsize = ALIGN_BIS(size + 2*SIZE_T_SIZE);
		//printf("allocating : %u bytes\n ", newsize);
		// heapCur is the current pointer
		char * heapCur = heapLo;
		int emptySpace = 0;
		int blockSize = 0;
		int busy = 0;
		int flag = 0;
		while(heapCur < ((char*) mem_heap_hi()- SIZE_T_SIZE)){
				blockSize = SIZE(heapCur);
				busy = BUSY(heapCur);
				//printf("heapCur %u memheapHi %u ",heapCur, mem_heap_hi());
				//printf("ptr : %u size : %u busy : %d\n", heapCur, blockSize, busy);
				if (blockSize == 0){
						printf("BLOCKSIZE 0 -> SOMETHING IS NOT OK\n");
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
				if (!busy){
						//printf("heapCur %u is not busy\n",heapCur);
						emptySpace = mem_pagesize()*nb_sbrk + blockSize;
						heapCur -= blockSize;
						
				}
				else {
						//printf("heapCur %u is busy\n",heapCur);
						emptySpace = mem_pagesize()*nb_sbrk;
						heapCur = mem_heap_hi() +1 - SIZE_T_SIZE - mem_pagesize()*nb_sbrk;
				}
				//printf("create nb sbrk %u, new heapCur %u, new blocksize %u\n",nb_sbrk,heapCur,blockSize);
		}
		VAL(heapCur) = newsize | 1;
		VAL(ENDPTR(heapCur)) = newsize | 1;
		if (emptySpace - newsize !=0){
				VAL(heapCur + newsize) = emptySpace - newsize;
				VAL(heapCur +emptySpace - SIZE_T_SIZE) = emptySpace - newsize;
		}
		/*a = mm_check();
		if(a == 0)
			printf("mem check failed end malloc\n");*/
		return (heapCur + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block changes it's header and footer and tries to coallese with the blocks before and after.
 */
void mm_free(void *ptr)
{		
		/*int a = mm_check();
		if(a == 0)
			printf("mem check failed begin free\n");*/	
		if(ptr == NULL)
				return;
		//printf("freeing ptr : %u\n", ptr);
		ptr = ptr - SIZE_T_SIZE;

		//let's mark the block as free
		VAL(ptr) = VAL(ptr) & (-2);
		VAL(ENDPTR(ptr)) = VAL(ptr);
		//printf("size of block ; %u\n", SIZE(ptr));

		//let's try to coalesce with previous block
		void* ptrPrevBlock = ptr - SIZE(ptr - SIZE_T_SIZE);
		//printf("prevblock %u, sizeprevblock %u, busy ? %u \n",ptrPrevBlock, SIZE(ptrPrevBlock), SIZE(ptrPrevBlock));
		if(!BUSY(ptrPrevBlock) && (ptr > mem_heap_lo()+5)){//prev block is not busy and ptr is not the first block
				VAL(ptrPrevBlock) = SIZE(ptr) + SIZE(ptrPrevBlock);
				VAL(ENDPTR(ptrPrevBlock)) = VAL(ptrPrevBlock);
				ptr = ptrPrevBlock;
				//printf("coalese prev : value coalesed %u \n", VAL(ptr));
		}

		//let's try to coalesce with next block
		void* ptrNextBlock = ptr + SIZE(ptr);
		//printf("nextblock %u, sizenextblock %u, busy ? %u \n",ptrNextBlock, SIZE(ptrNextBlock), BUSY(ptrNextBlock));
		if(! BUSY(ptrNextBlock) && (ptr + SIZE(ptr)) < (mem_heap_hi() -5)){//next block is not busy and ptr is not the last block
				VAL(ptr) = SIZE(ptr) + SIZE(ptrNextBlock);
				VAL(ENDPTR(ptr)) = VAL(ptr);
				//printf("coalese next : value coalesed %u \n", VAL(ptr));
		}
	/*a = mm_check();
	if(a == 0)
		printf("mem check failed end free\n");*/
}

/*
 * mm_realloc - If the old block is bigger than the new block, free the end of the old block, set footers and headers.
 * If the old block is smaller than the new block, we see the state of the next blocks until we reach the new size of block, 
 * and update accordingly
 */
void *mm_realloc(void *ptr, size_t size)
{
		//printf("reallocating ptr %u of size %u \n ",ptr, size);
		/*int a = mm_check();
		if(a == 0)
			printf("mem check failed begin realloc\n");*/
		if (ptr == NULL){
				return mm_malloc(size);
		}
		else if (size == 0){
				mm_free(ptr);
				return NULL;
		}
		int newBlockSize = ALIGN_BIS(size + 2*SIZE_T_SIZE);
		ptr -= SIZE_T_SIZE;		
		//printf("old block is busy %u of size %u \n", BUSY(ptr), SIZE(ptr));
		if (SIZE(ptr) == newBlockSize){
				return ptr + SIZE_T_SIZE;
		}
		if (SIZE(ptr) > newBlockSize){
				int oldSize = SIZE(ptr);
				//printf(" newBlockSize is smaller\n");
				VAL(ptr) = newBlockSize | BUSY(ptr);
				VAL(ENDPTR(ptr)) = newBlockSize | BUSY(ptr);
				//rest of the pointer freed
				//printf(" size freed : %u\n", blockSize - newBlockSize);
				VAL(ptr + newBlockSize) = (oldSize-newBlockSize) |1; 
				VAL(ENDPTR(ptr + newBlockSize)) = (oldSize-newBlockSize) |1;
				mm_free(ptr + newBlockSize + SIZE_T_SIZE);
		}
		if (SIZE(ptr) < newBlockSize){
				//printf(" newBlockSize is bigger\n");
				void* nextBl = ptr + SIZE(ptr);
				//printf("  nextBlockSize : %u busy %d\n", SIZE(nextBl), BUSY(nextBl));
				int isLastBlock = nextBl >=  (void*)mem_heap_hi() - 3;
				if(!isLastBlock && !BUSY(nextBl) && SIZE(ptr) + SIZE(nextBl) >= newBlockSize){ //we have room in the next block
						//printf("room in next block\n");
						int oldSize = SIZE(ptr);
						int oldNextSize = SIZE(nextBl);
						VAL(ptr) = newBlockSize | 1;
						VAL(ENDPTR(ptr)) = VAL(ptr);
						if(oldSize + oldNextSize > newBlockSize){
								VAL(ptr + SIZE(ptr)) = (oldSize + oldNextSize - newBlockSize) & (-2);
								VAL(ENDPTR(ptr + SIZE(ptr))) = VAL(ptr + SIZE(ptr));
						}
				}
				else{//we don't
					  //printf("nor room in next block\n");
						void *oldptr = ptr + SIZE_T_SIZE;
						size_t copySize = SIZE(ptr);
						//printf("on malloc : ");
						ptr = mm_malloc(size);
						//printf("new ptr : %u\n", ptr);
						if (ptr == NULL){
								//printf("malloc failed in realloc");
								return NULL;
						}
						memcpy(ptr, oldptr, copySize);
						//printf("on free : ");
						mm_free(oldptr);
						//printf("freed %u and new at %u\n", oldptr, ptr);
						/*int b = mm_check();
						if(b == 0)
							printf("mem check failed end realloc b\n");*/
						return ptr;
				}
		}
		/*a = mm_check();
		if(a == 0)
			printf("mem check failed end realloc\n");*/
		return ptr + SIZE_T_SIZE;
}
