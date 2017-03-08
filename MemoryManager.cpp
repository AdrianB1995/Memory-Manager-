/***********************************************************
* Adrian Borrego  02/20/17
* MemoryManager.cpp
*
* Summary: Simulate the C heap manager.
*
***********************************************************/

#include <cassert>
#include <iostream>
#include "dlUtils.h"
#include "MemoryManager.h"

MemoryManager::MemoryManager(unsigned int memtotal): memsize(memtotal)
{
   baseptr = new unsigned char[memsize];
   blockdata dummyBlock(0,false,0);
   blockdata originalBlock(memsize,true,baseptr);
   header = new dlNode<blockdata>(dummyBlock,nullptr,nullptr);
   trailer = new dlNode<blockdata>(dummyBlock,nullptr,nullptr);
   header->next = new dlNode<blockdata>(originalBlock,header,trailer);
   trailer->prev = header->next;
}

MemoryManager::~MemoryManager()
{
  delete [] baseptr;
  clearList(header);
}

void MemoryManager::showBlockList()
{
  printDlList(header,trailer,"->");
}

//Split node P of size chunksize, used in malloc function.
void MemoryManager::splitBlock(dlNode<blockdata> *p, unsigned int chunksize)
{
    //Assert that node P memsize is sufficent.
    if(p->info.blocksize > chunksize){
      unsigned int newsize = p->info.blocksize - chunksize;
      blockdata new_Block(newsize, true, baseptr + chunksize);
      //Use insertAfter function to insert new node.
      insertAfter(trailer, p, new_Block);
  }
}

//Used to satisfy a request for a specific number of consecutive blocks.
unsigned char * MemoryManager::malloc(unsigned int request)
{
  //Create pointer to 1st node in sequence.
  dlNode<blockdata> *ptr = header->next;
  //Traverse through list.
  while(ptr != trailer)
  {
    if(ptr->info.free == true && ptr->info.blocksize >= request)
    {
      if(ptr->info.blocksize > request){
        //If node found with sufficent memsize, use splitBlock function.
        splitBlock(ptr, request);
      }
      //Set conditions whether block is split or not.
      ptr->info.free = false;
      ptr->info.blocksize = request;
      return ptr->info.blockptr;
    }
    ptr = ptr->next;
  }
  //If malloc failed, return NULL.
  return NULL;
}

//Used in free method.
void MemoryManager::mergeForward(dlNode<blockdata> *p)
{
  //Create node pointer to next "free" node.
  dlNode<blockdata> *hold = p->next;
  //Update size of p node.
  p->info.blocksize += hold->info.blocksize;
  //Point P to proper nodes before deletion.
  p->next->next->prev = p;
  p->next = p->next->next;
  //Delete hold;
  delete hold;
  hold = NULL;
}

//Used in free method.
void MemoryManager::mergeBackward(dlNode<blockdata> *p)
{
  //Create node pointer to previous "free" node.
  dlNode<blockdata> *hold = p->prev;
  //Update size of p node.
  p->info.blocksize += hold->info.blocksize;
  //Point P to proper nodes before deletion.
  p->prev->prev->next = p;
  p->prev = p->prev->prev;
  p->info.blockptr = hold->info.blockptr;
  //Delete hold;
  delete hold;
  hold = NULL;
}

//Used to make allocated blocks available.
void MemoryManager::free(unsigned char *ptr2block)
{
  //Create node pointer to 1st node in sequence.
  dlNode<blockdata> *ptr = header->next;
  //Traverse list.
  while(ptr != trailer){
    //Test condition for finding ptr2block node.
    if(ptr->info.blockptr == ptr2block){
      ptr->info.free = true;
    }
    //Test for available node blocks following ptr.
    if(ptr->info.free == true && ptr->next->info.free == true){
      mergeForward(ptr);
    }
    //Test for previous available node blocks.
    if(ptr->info.free == true && ptr->prev->info.free == true){
      mergeBackward(ptr);
    }
    ptr = ptr->next;
  }
}
