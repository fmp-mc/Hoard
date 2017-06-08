/*
  The Hoard Multiprocessor Memory Allocator
  www.hoard.org

  Author: Emery Berger, http://www.cs.umass.edu/~emery
  Author of this file: Yixiao Li, liyixiao7@gmail.com
 
  Copyright (c) 1998-2014 Emery Berger
  Copyright (c) 2016-     Yixiao Li

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#if !defined(_BARE_METAL)
#error "This file is intended for use with bare metal environment only."
#endif

//#include <dlfcn.h>
//#include <pthread.h>
#include <utility>

#include "Heap-Layers/heaplayers.h"
#include "hoard/hoardtlab.h"

#include "hoard_bme.h"

extern Hoard::HoardHeapType * getMainHoardHeap();

bool isCustomHeapInitialized() {
  // Always return true since getCustomHeap() can be called at any time
  return true;
}

static TheCustomHeapType * initializeCustomHeap() {
  TheCustomHeapType * heap =
    reinterpret_cast<TheCustomHeapType *>(hoard_get_thread_heap());
  if (heap == NULL) {
    // Defensive programming in case this is called twice.
    // Allocate a per-thread heap.
    size_t sz = sizeof(TheCustomHeapType);
    char * mh = reinterpret_cast<char *>(getMainHoardHeap()->malloc(sz));
    heap = new (mh) TheCustomHeapType(getMainHoardHeap());
    // Store it in the appropriate thread-local area.
    hoard_set_thread_heap(heap);
  }
  return heap;
}

TheCustomHeapType * getCustomHeap() {
  // Allocate a per-thread heap.
  TheCustomHeapType * heap =
    reinterpret_cast<TheCustomHeapType *>(hoard_get_thread_heap());
  if (heap == NULL)  {
    heap = initializeCustomHeap();
  }
  return heap;
}


//
// Hook functions for thread creation and destruction to flush the TLABs.
//

extern "C" void hoard_hook_on_thread_create() {
    // Make sure that the custom heap has been initialized,
    // then find an unused process heap for this thread, if possible.
    getCustomHeap();
    getMainHoardHeap()->findUnusedHeap();
}

// Called when the thread goes away.  This function clears out the
// TLAB and then reclaims the memory allocated to hold it.
extern "C" void hoard_hook_on_thread_destroy() {
  auto p = getCustomHeap();
  p->clear();
  getMainHoardHeap()->free(p);

  // Relinquish the assigned heap.
  getMainHoardHeap()->releaseHeap();
}

