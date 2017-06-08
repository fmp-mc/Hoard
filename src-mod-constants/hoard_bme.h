#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Provided API
 */

void * xxmalloc (size_t);
size_t xxmalloc_usable_size (void *);
void   xxfree (void *);

static inline
void * xxrealloc (void * ptr, size_t sz) {
    // NULL ptr = malloc.
    if (ptr == NULL) {
      return xxmalloc(sz);
    }

    if (sz == 0) {
      xxfree (ptr);
#if defined(__APPLE__)
      // 0 size = free. We return a small object.  This behavior is
      // apparently required under Mac OS X and optional under POSIX.
      return xxmalloc(1);
#else // Bare metal fallback to POSIX way
      // For POSIX, don't return anything.
      return NULL;
#endif
    }

    size_t objSize = xxmalloc_usable_size(ptr);
    
#if 0
    // Custom logic here to ensure we only do a logarithmic number of
    // reallocations (with a constant space overhead).

    // Don't change size if the object is shrinking by less than half.
    if ((objSize / 2 < sz) && (sz <= objSize)) {
      // Do nothing.
      return ptr;
    }
    // If the object is growing by less than 2X, double it.
    if ((objSize < sz) && (sz < objSize * 2)) {
      sz = objSize * 2;
    }
#endif

    void * buf = xxmalloc(sz);

    if (buf != NULL) {
      // Successful malloc.
      // Copy the contents of the original object
      // up to the size of the new block.
      size_t minSize = (objSize < sz) ? objSize : sz;
      memcpy (buf, ptr, minSize);
      xxfree (ptr);
    }

    // Return a pointer to the new one.
    return buf;
}

/**
 * Required interfaces
 * Note: following POSIX functions are also required currently.
 * - pthread_self
 * - sched_yield
 */

void  hoard_set_thread_heap(void *heap);
void *hoard_get_thread_heap();

void hl_lock_create(intptr_t *lock);
void hl_lock_destroy(intptr_t *lock);
void hl_lock_acquire(intptr_t *lock);
void hl_lock_release(intptr_t *lock);


void *hl_mmapwrapper_map (size_t sz);
void  hl_mmapwrapper_unmap (void * ptr, size_t sz);
//void  hl_mmapwrapper_protect (void * ptr, size_t sz);
//void  hl_mmapwrapper_unprotect (void * ptr, size_t sz);

#if defined(__cplusplus)
}
#endif

