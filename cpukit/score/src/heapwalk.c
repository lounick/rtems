/*
 *  Heap Handler
 *
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/system.h>
#include <rtems/score/sysstate.h>
#include <rtems/score/heap.h>

/*PAGE
 *
 *  _Heap_Walk
 *
 *  This kernel routine walks the heap and verifies its correctness.
 *
 *  Input parameters:
 *    the_heap  - pointer to heap header
 *    source    - a numeric indicator of the invoker of this routine
 *    do_dump   - when TRUE print the information
 *
 *  Output parameters: NONE
 */

#if !defined(RTEMS_HEAP_DEBUG)

boolean _Heap_Walk(
  Heap_Control  *the_heap,
  int            source,
  boolean        do_dump
)
{
  return TRUE;
}

#else /* defined(RTEMS_HEAP_DEBUG) */

#include <stdio.h>

boolean _Heap_Walk(
  Heap_Control  *the_heap,
  int            source,
  boolean        do_dump
)
{
  Heap_Block *the_block = the_heap->start;
  Heap_Block *const end = the_heap->final;
  Heap_Block *const tail = _Heap_Tail(the_heap);
  int error = 0;
  int passes = 0;

  /*
   * We don't want to allow walking the heap until we have
   * transferred control to the user task so we watch the
   * system state.
   */

  if ( !_System_state_Is_up( _System_state_Get() ) )
    return FALSE;

  if (source < 0)
    source = the_heap->stats.instance;

  if (do_dump == TRUE)
    printf("\nPASS: %d start %p final %p first %p last %p begin %p end %p\n",
      source, the_block, end,
      _Heap_First(the_heap), _Heap_Last(the_heap),
      the_heap->begin, the_heap->end);

  /*
   * Handle the 1st block
   */

  if (!_Heap_Is_prev_used(the_block)) {
    printf("PASS: %d !HEAP_PREV_USED flag of 1st block isn't set\n", source);
    error = 1;
  }

  if (the_block->prev_size != HEAP_PREV_USED) {
    printf("PASS: %d !prev_size of 1st block isn't HEAP_PREV_USED\n", source);
    error = 1;
  }

  while ( the_block < end ) {
    uint32_t const the_size = _Heap_Block_size(the_block);
    Heap_Block *const next_block = _Heap_Block_at(the_block, the_size);
    boolean prev_used = _Heap_Is_prev_used(the_block);

    if (do_dump) {
      printf("PASS: %d block %p size %d(%c)",
        source, the_block, the_size, (prev_used ? 'U' : 'F'));
      if (prev_used)
        printf(" prev_size %d", the_block->prev_size);
      else
        printf(" (prev_size) %d", the_block->prev_size);
    }

    if (!_Heap_Is_block_in(the_heap, next_block)) {
      if (do_dump) printf("\n");
      printf("PASS: %d !block %p is out of heap\n", source, next_block);
      error = 1;
      break;
    }

    if (!_Heap_Is_prev_used(next_block)) {
      if (do_dump)
        printf( " prev %p next %p", the_block->prev, the_block->next);
      if (_Heap_Block_size(the_block) != next_block->prev_size) {
        if (do_dump) printf("\n");
        printf("PASS: %d !front and back sizes don't match", source);
        error = 1;
      }
      if (!prev_used) {
        if (do_dump || error) printf("\n");
        printf("PASS: %d !two consecutive blocks are free", source);
        error = 1;
      }

      { /* Check if 'the_block' is in the free block list */
        Heap_Block* block = _Heap_First(the_heap);
        while(block != the_block && block != tail)
          block = block->next;
        if(block != the_block) {
          if (do_dump || error) printf("\n");
          printf("PASS: %d !the_block not in the free list", source);
          error = 1;
        }
      }

    }
    if (do_dump || error) printf("\n");

    if (the_size < the_heap->min_block_size) {
      printf("PASS: %d !block size is too small\n", source);
      error = 1;
      break;
    }
    if (!_Heap_Is_aligned( the_size, the_heap->page_size)) {
      printf("PASS: %d !block size is misaligned\n", source);
      error = 1;
    }

    if (++passes > (do_dump ? 10 : 0) && error)
      break;

    the_block = next_block;
  }

  if (the_block != end) {
    printf("PASS: %d !last block address isn't equal to 'final'\n", source);
    error = 1;
  }

  if (_Heap_Block_size(the_block) != 0) {
    printf("PASS: %d !last block's size isn't 0\n", source);
    error = 1;
  }

  if(do_dump && error)
    abort();

  return error == 0;

}
#endif  /* defined(RTEMS_HEAP_DEBUG) */
