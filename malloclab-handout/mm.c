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
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

static void* head_listp;
static void* linkhead;

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *recoalesce(void *bp,size_t size);
static void *find_fit(size_t size);
static void del_chunk_list(void *bp);//delete_chunk_from_free_list
static void put_chunk_list(void *bp);//put_chunk_in_free_list
static void place(void *bp,size_t size);
static int find_index(size_t);
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void){

    //printf("init!\n");
    if ((head_listp=mem_sbrk(12*WSIZE)) == (void *)-1)
        return -1;

    PUT(head_listp, NULL);
    PUT(head_listp+1*WSIZE, NULL);
    PUT(head_listp+2*WSIZE, NULL);
    PUT(head_listp+3*WSIZE, NULL);
    PUT(head_listp+4*WSIZE, NULL);
    PUT(head_listp+5*WSIZE, NULL);
    PUT(head_listp+6*WSIZE, NULL);
    PUT(head_listp+7*WSIZE, NULL);
    PUT(head_listp+8*WSIZE, NULL);
    PUT(head_listp + (9*WSIZE),PACK(DSIZE,1));
    PUT(head_listp + (10*WSIZE),PACK(DSIZE,1));
    PUT(head_listp + (11*WSIZE),PACK(0,1));  //Epilogue header

    linkhead = head_listp;
    head_listp += (10*WSIZE);

    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
    {
        return -1;
    }
    return 0;
}

static void *extend_heap(size_t words){

    //printf("extend!\n");
    char *bp;
    size_t size;

    size=(words %2) ? (words+1)*WSIZE : words*WSIZE;
    if ((bp=mem_sbrk(size)) == -1)
        return NULL;

    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1)); //New Epilogue header
    PUT(PRED(bp),NULL);
    PUT(SUCC(bp),NULL);

    return coalesce(bp);
}

static void *coalesce(void *bp){

    //printf("coalesce!\n");
    size_t prev_alloc=GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc){
        put_chunk_list(bp);
        return bp;
    }else if (prev_alloc && !next_alloc){
        del_chunk_list(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
        put_chunk_list(bp);
        return bp;
    }else if (!prev_alloc && next_alloc){
        del_chunk_list(PREV_BLKP(bp));
        size+=GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        put_chunk_list(PREV_BLKP(bp));
        return PREV_BLKP(bp);
    }else{
        del_chunk_list(PREV_BLKP(bp));
        del_chunk_list(NEXT_BLKP(bp));
        size+=(GET_SIZE(HDRP(NEXT_BLKP(bp)))+GET_SIZE(FTRP(PREV_BLKP(bp))));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        put_chunk_list(PREV_BLKP(bp));
        return PREV_BLKP(bp);
    }
}

static void del_chunk_list(void *bp){

    //printf("del_chunk_list!\n");
    PUT(SUCC(PRED_BLKP(bp)), SUCC_BLKP(bp));
    if(SUCC_BLKP(bp)!=NULL)
        PUT(PRED(SUCC_BLKP(bp)), PRED_BLKP(bp));
}

static void put_chunk_list(void *bp){

    //printf("put_chunk_list\n");
    int index=find_index(GET_SIZE(HDRP(bp)));
    void* root=linkhead+index*WSIZE;

    if (SUCC_BLKP(root) != NULL){
        PUT(PRED(SUCC_BLKP(root)),bp);
        PUT(SUCC(bp),SUCC_BLKP(root));
    } else{
        PUT(SUCC(bp),NULL);
    }

    PUT(SUCC(root),bp);
    PUT(PRED(bp),root);
    //printf("end put_chunk_list\n");
}

static void place(void *bp,size_t asize){

    //printf("place!\n");
    size_t left=GET_SIZE(HDRP(bp))-asize;
    //int alloc=GET_ALLOC(HDRP(bp));
    //if (alloc == 0)
        del_chunk_list(bp);

    if (left >= (DSIZE*2)){
        PUT(HDRP(bp),PACK(asize,1));
        PUT(FTRP(bp),PACK(asize,1));

        PUT(HDRP(NEXT_BLKP(bp)),PACK(left,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(left,0));

        coalesce(NEXT_BLKP(bp));
    } else{
        size_t allsize =GET_SIZE(HDRP(bp));
        PUT(HDRP(bp),PACK(allsize,1));
        PUT(FTRP(bp),PACK(allsize,1));
    }
}

static void *find_fit(size_t size){

    //printf("find_fit!\n");
    int index=find_index(size);
    void *addr;
    for (int i = index;i <= 8;i++){
        addr = linkhead+i*WSIZE;
        while ((addr = SUCC_BLKP(addr)) != NULL){
            if (GET_SIZE(HDRP(addr)) >= size && !GET_ALLOC(addr)){
                //printf("end_find_fit\n");
	        return addr;
            }
        }
    }

    return NULL;
}



/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    
    //printf("mm_malloc!\n");
    size_t asize;
    size_t extendsize;
    char *bp;

    if (size == 0){
        return NULL;
    }

    if(size <= DSIZE)
        asize=2*DSIZE;
    else
        asize=DSIZE*((size+(DSIZE)+(DSIZE-1))/DSIZE);

    if ((bp=find_fit(asize))!=NULL)
    {
        place(bp,asize);
        return bp;
    }

    if ((bp = extend_heap(MAX(CHUNKSIZE, asize)/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr){

    //printf("mm_free!\n");
    size_t size=GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);

}

static void *recoalesce(void *bp,size_t needsize){

    size_t prev_alloc=GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc){
        return NULL;
    }else if (prev_alloc && !next_alloc){
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        if (size<needsize)
            return NULL;
        else{
            del_chunk_list(NEXT_BLKP(bp));
            PUT(HDRP(bp),PACK(size,1));
            PUT(FTRP(bp),PACK(size,1));
            return bp;
        }
    }else if (!prev_alloc && next_alloc){
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        if(size<needsize)
            return NULL;
        else{
            size_t thissize=GET_PAYLOAD(bp);
            void* prev_point=PREV_BLKP(bp);
            del_chunk_list(prev_point);
            PUT(FTRP(bp),PACK(size,1));
            PUT(HDRP(prev_point),PACK(size,1));
            memmove(prev_point,bp,thissize);
            return prev_point;
        }
    }else{
        size+=(GET_SIZE(HDRP(NEXT_BLKP(bp)))+GET_SIZE(FTRP(PREV_BLKP(bp))));
        if (size<needsize)
            return NULL;
        else
        {
            size_t thissize=GET_PAYLOAD(bp);
            void* prev_point=PREV_BLKP(bp);
            del_chunk_list(prev_point);
            del_chunk_list(NEXT_BLKP(bp));
            PUT(FTRP(NEXT_BLKP(bp)),PACK(size,1));
            PUT(HDRP(PREV_BLKP(bp)),PACK(size,1));
            memmove(prev_point,bp,thissize);
            return prev_point;
        }
    }
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{

    if(ptr==NULL){
        return mm_malloc(size);
    }
    if(size==0)
    { 
        mm_free(ptr);
        return ptr;
    }

    size_t asize=0;

    if(size<=DSIZE)
        asize=2*DSIZE;
    else
        asize=DSIZE*((size+(DSIZE)+(DSIZE-1))/DSIZE);

    size_t oldsize=GET_PAYLOAD(ptr);

    if(oldsize<size){
        void* newptr=recoalesce(ptr,asize);
        if(newptr==NULL){
            newptr=mm_malloc(asize);
            memmove(newptr,ptr,oldsize);
            mm_free(ptr);
            return newptr;
        }else{
            return newptr;
        }
    }else if(oldsize==size){
        return ptr;
    }else{
        return ptr;
    }
    return NULL;
}

static int find_index(size_t size){
    
    if (size<32)
        return 0;
    else if (size<64)
        return 1;
    else if (size<128)
        return 2;
    else if (size<256)
        return 3;
    else if (size<512)
        return 4;
    else if (size<1024)
        return 5;
    else if (size<2048)
        return 6;
    else if (size<4096)
        return 7;
    else
        return 8;
}











