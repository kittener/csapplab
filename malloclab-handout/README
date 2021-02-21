宏定义如下

```
//字大小和双字大小
#define WSIZE 4
#define DSIZE 8
//当堆内存不够时，向内核申请的堆空间
#define CHUNKSIZE (1<<12)
//将val放入p开始的4字节中
#define GET(p) (*(unsigned int*)(p))
#define PUT(p,val) (*(unsigned int*)(p) = (val))
//获得头部和脚部的编码
#define PACK(size, alloc) ((size) | (alloc))
//从头部或脚部获得块大小和已分配位
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
//获得块的头部和脚部
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
//获得上一个块和下一个块
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))

//获得块中记录后继和前驱的地址
#define PRED(bp) ((char*)(bp) + WSIZE)
#define SUCC(bp) ((char*)bp)
//获得块的后继和前驱的地址
#define PRED_BLKP(bp) (GET(PRED(bp)))
#define SUCC_BLKP(bp) (GET(SUCC(bp)))

//获取块的实际大小(去头去脚)
#define GET_PAYLOAD(bp) (GET_SIZE(HDRP(bp))-DSIZE) 
#define MAX(x,y) ((x)>(y)?(x):(y))
```

本次我选择的堆块结构，为课本上讲述的显式空闲链表

```
|-----------------|
|  块大小  |  a/f  |
|-----------------|
|	pred(祖先)     |
|-----------------|
|	succ(后继)	 |
|-----------------|
|				  |
|				  |
|-----------------|
|  块大小  |  a/f  |
|-----------------|
```

采用分离存储的方法，即根据空闲块的大小，以2的次方幂为界限，维护一个空闲块链表组。当分配器需要一个大小为n的块的时候，就搜索相应的空闲链表，如果在此空闲链表中找不到合适的块，就去下一个空闲链表找

因为规定了不允许定义全局数组或结构体，所以我能想到的方法就是在初始化的时候，额外申请地址空间，以此来维护空闲链表组，我采用的分组为

```
{0~31}{32~63}{64~127}{128~255}{256~511}{512~1023}{1024~2047}{2048~4085}{4096~...}
```

init函数如下，课本上有给，除了额外申请的，跟课本上给的差不多

```
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
```

extend_heap函数课本也给了，照抄~

```
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
```

coalesce函数，书上也给了，分为四种情况：

- 前后都没有空闲块，不操作直接将该块放进相应大小的链表中
- 后面有空闲块，而前面没有，size变为两个空闲块的总大小，修改当前空闲块的size域
- 前面有空闲块，而后面没有，size变为两个空闲块的总大小，修改前一个空闲块的size域
- 前后都有空闲块，size变为三者总和大小，修改前一个空闲块的size域

当然，在合并之前，首先要将该空闲块从相应的空闲链表组中取下来，合并完再重新加到相对应的空闲链表中

```
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
```

del_chunk_list函数

就是简单的将此空闲块的祖先的后继指针指向该空闲块的后继(好绕)

```
static void del_chunk_list(void *bp){

    //printf("del_chunk_list!\n");
    PUT(SUCC(PRED_BLKP(bp)), SUCC_BLKP(bp));
    if(SUCC_BLKP(bp)!=NULL)
        PUT(PRED(SUCC_BLKP(bp)), PRED_BLKP(bp));
}
```

put_chunk_list函数

就是查找空闲块相对应的空闲链表，插入进去，我用的是LIFO方法，就，操作挺简单的直接塞进去就好了(

```
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
```

place函数

因为分配器可能找不到恰好满足需要大小的块，所以place函数用来切割空闲块，以减少内部碎片的大小，先将要分配的块从空闲链表中取下来，检测剩余大小够不够变成一个新空闲块，如果够，则切割，将新块放入空闲链表中

因为这次维护的域从最开始的头部和脚部两个，增加到了四个(增加了前驱指针和后继指针)，所以一个空闲块的最小大小由双字变成了四字，所以分离块的大小也要随之相应改变。

```
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
```

find_fit函数

唔，思路挺简单的，遍历链表，找到合适的为止，next_fit可能会让分数变得更高一些，但我没写(怕写着写着又炸了)，准备换一下试试

```
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
```

mm_malloc函数

课本上给了，照抄~

```
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
```

mm_free函数

课本上给了，没啥大问题，照抄~

```
void mm_free(void *ptr){

    //printf("mm_free!\n");
    size_t size=GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);

}
```

find_index函数

简单的根据大小判断下标

```
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
```

好了，到了最最最坑爹的realloc函数了，这个函数...

mm_realloc函数

两个特殊情况：

- 如果当前指针为空，则分配新的内存块，跟malloc一样
- 如果size为0，释放当前指向的块，并返回null

一般情况：

- 如果当前指针指向的内存块的大小小于参数size的大小(oldsize < size)，先尝试合并周围的空闲块
  - 如果能成功，则返回合并块后的指针，可能是没变，也可能是变了的，取决于前后空余块
  - 如果失败，则申请一个对应大小的空余块，malloc函数申请
- 如果当前指针指向的内存块的大小大于等于参数size的大小(oldsize >= size)，直接返回

但是死活过不去- -，调试的时候发现好像是coalesce函数立即存放空闲链表的方式会导致chunk块指针跑飞，直接报错payload覆盖掉了下一个块了(其实后来想想肯定的啊，当时犯蠢了)

然后我参考网上的方法，为realloc函数重新写了个recoalesce函数

```
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
```

这里在调试的时候发现，memcpy就是复制不过去旧值，特别离谱，在查询以前lab的Q&A的时候，发现了有人问教授是否可以用memmove函数时教授说可以，然后我就用了memmove函数，然后过了2333

http://www.cs.cmu.edu/afs/cs/academic/class/15213-s02/www/applications/qa/lab6.html



recoalesce函数

大体上跟coalesce函数差不多，但是取消了立即存放的方式，就简简单单返回一个当前块就好了~

合并之前需要判断，如果合并完之后还是满足不了需要，就不合并，返回NULL

```
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
```

malloc lab完结撒花~

最后得分89/100

这个lab真的让我对块分配有了更深的理解，对指针操作更加熟练



思考题

loader在磁盘上打开目标程序的文件，读取文件并计算程序段的大小，根据大小申请物理内存并映射到虚拟内存。然后初始化栈空间，之后加载动态链接库，并对程序中用到的动态链接的符号进行重定位。

execve发送一个异常给CPU，CPU发送一个VA给MMU，MMU再发送一个PTEA给高速缓存，高速缓存返回一个PTE这时候会有两种情况

- PTE有效位是0，则触发缺页异常，进行上下文切换，切换到操作系统中的内核的缺页异常处理程序，缺页异常处理程序会选中一个物理内存的牺牲页，如果这个牺牲页被修改过，则把他换出到磁盘，然后把新页换上去，更新PTE。然后CPU重新启动引起缺页异常的指令，这时候就可以正常运行了
- PTE是1，则直接访问

