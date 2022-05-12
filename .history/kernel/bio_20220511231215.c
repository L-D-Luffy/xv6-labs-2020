// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKETS 13

extern uint ticks;


struct {
  // 用于驱逐时顺序化
  struct spinlock lock;
  struct buf buf[NBUF];

  // 
  struct buf hashhead[BUCKETS];
  struct spinlock buclocks[BUCKETS];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;
  char name[9];
  for (int i = 0; i < BUCKETS; i++) {
    snprintf(name, 9, "bcache_%d", i);
    initlock(&bcache.buclocks[i], name);
  }

  for (int i = 0; i < NBUF; i++) {
    int n = i % BUCKETS;
    bcache.buf[i].next = bcache.hashhead[n].next;
    bcache.buf[i].prev = &bcache.hashhead[n];
    if (bcache.hashhead[n].next) bcache.hashhead[n].next->prev = &bcache.buf[i];
    bcache.hashhead[n].next = &bcache.buf[i];
    // bcache.buf[i].btick = 0;
  }

  // initlock(&bcache.lock, "bcache");

  // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  // for(b = bcache.buf; b < bcache.buf+NBUF; b++){
  //   b->next = bcache.head.next;
  //   b->prev = &bcache.head;
  //   initsleeplock(&b->lock, "buffer");
  //   bcache.head.next->prev = b;
  //   bcache.head.next = b;
  // }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  // acquire(&bcache.lock);

  // // Is the block already cached?
  // for(b = bcache.head.next; b != &bcache.head; b = b->next){
  //   if(b->dev == dev && b->blockno == blockno){
  //     b->refcnt++;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }

  //
  int hashid = blockno % BUCKETS;
  acquire(&bcache.buclocks[hashid]);

  for(b = bcache.hashhead[hashid].next; b; b = b->next){
    if (b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.buclocks[hashid]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  // 为了防止死锁，先释放
  release(&bcache.buclocks[hashid]);
  //加大锁，防止重复开启驱逐
  acquire(&bcache.lock);
  // 加完大锁还得重复判断一下
  acquire(&bcache.buclocks[hashid]);
  for(b = bcache.hashhead[hashid].next; b; b = b->next){
    if (b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.buclocks[hashid]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  release(&bcache.buclocks[hashid]);
  // new not cached
  int find = -1;
  uint min_btick = 0;
  struct buf *evicbuf;
  int newevicid = -1;
  int oldevicid = -1;
  for(int i = 0; i < BUCKETS; i++){
    // if (i == hashid) continue;
    acquire(&bcache.buclocks[i]);
    for(b = bcache.hashhead[i].next; b; b = b->next){
      if (b->refcnt == 0 && (min_btick == 0 || min_btick > b->btick)){
        // 如果找到有，就打标记，不用panic
        find = 1;
        min_btick = b->btick;
        evicbuf = b;
        newevicid = i;
      }
    }
    if(newevicid != oldevicid && oldevicid != -1){
      release(&bcache.buclocks[oldevicid]);
    }
    oldevicid = newevicid;
  }
  // 此时，还持有选中的那个buf的链表的锁
  if(find == -1){
    panic("bget: no buffers");
  }
  evicbuf->dev = dev;
  evicbuf->blockno = blockno;
  evicbuf->valid = 0;
  evicbuf->refcnt = 1;
  // 判断是否是同一个链表
  if (newevicid != hashid){
    acquire(&bcache.buclocks[hashid]);
    // 在原来的链表中删除
    evicbuf->prev->next = evicbuf->next;
    if (evicbuf->next) evicbuf->next->prev = evicbuf->prev;
    release(&bcache.buclocks[newevicid]);
    // 添加到新链表
    evicbuf->next = bcache.hashhead[hashid].next;
    evicbuf->prev = &bcache.hashhead[hashid];
    if (bcache.hashhead[hashid].next) bcache.hashhead[hashid].next->prev = evicbuf;
    bcache.hashhead[hashid].next = evicbuf;
    release(&bcache.buclocks[hashid]);
    acquiresleep(&evicbuf->lock);
  } else {
    release(&bcache.buclocks[hashid]);
    acquiresleep(&evicbuf->lock);
  }
  release(&bcache.lock);
  return evicbuf;
  // // Not cached.
  // // Recycle the least recently used (LRU) unused buffer.
  // for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
  //   if(b->refcnt == 0) {
  //     b->dev = dev;
  //     b->blockno = blockno;
  //     b->valid = 0;
  //     b->refcnt = 1;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }
  // panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  uint n = b->blockno % BUCKETS;
  acquire(&bcache.buclocks[n]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    // b->next->prev = b->prev;
    // b->prev->next = b->next;
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    // bcache.head.next->prev = b;
    // bcache.head.next = b;
    b->btick = ticks;
  }
  
  release(&bcache.buclocks[n]);
}

void
bpin(struct buf *b) {
  uint n = b->blockno % BUCKETS; 
  acquire(&bcache.buclocks[n]);
  b->refcnt++;
  release(&bcache.buclocks[n]);
}

void
bunpin(struct buf *b) {
  uint n = b->blockno % BUCKETS; 
  acquire(&bcache.buclocks[n]);
  b->refcnt--;
  release(&bcache.buclocks[n]);
}


