/*
 *  COPYRIGHT (c) 1989-2009.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#include <pmacros.h>
#include <time.h>
#include <errno.h>

void check_enosys(int status);

void check_enosys(int status)
{
  if ( (status == -1) && (errno == ENOSYS) ) 
    return;
  puts( "ERROR -- did not return ENOSYS as expected" );
  rtems_test_exit(0);
}

rtems_task Init(
  rtems_task_argument argument
)
{
  struct timespec tv;
  struct timespec tr;
  int             sc;
  int             priority;
  pthread_t       thread_id;
  time_t          seconds;
  time_t          seconds1;
  unsigned int    remaining;
  struct tm       tm;
  useconds_t      useconds;

  puts( "\n\n*** POSIX CLOCK TEST ***" );

  tm_build_time( &tm, TM_FRIDAY, TM_MAY, 24, 96, 11, 5, 0 );

  /* error cases in clock_gettime and clock_settime */

  puts( "Init: clock_gettime - EINVAL (NULL timespec)" );
  sc = clock_gettime( CLOCK_REALTIME, NULL );
  rtems_test_assert( sc == -1 );
  rtems_test_assert( errno == EINVAL );

  puts( "Init: clock_gettime - EINVAL (invalid clockid)" );
  sc = clock_gettime( (clockid_t)-1, &tv );
  rtems_test_assert( sc == -1 );
  rtems_test_assert( errno == EINVAL );

  puts( "Init: clock_settime - EINVAL (invalid clockid)" );
  sc = clock_settime( (clockid_t)-1, &tv );
  rtems_test_assert( sc == -1 );
  rtems_test_assert( errno == EINVAL );

  /* way back near the dawn of time :D */
  tv.tv_sec = 1;
  tv.tv_nsec = 0;
  printf( ctime( &tv.tv_sec ) );
  puts( "Init: clock_settime - before 1988 EINVAL" );
  sc = clock_settime( CLOCK_REALTIME, &tv );
  rtems_test_assert( sc == -1 );
  rtems_test_assert( errno == EINVAL );

  /* exercise clock_getres */

  puts( "Init: clock_getres - EINVAL (invalid clockid)" );
  sc = clock_getres( (clockid_t) -1, &tv );
  rtems_test_assert( sc == -1 );
  rtems_test_assert( errno == EINVAL );

  puts( "Init: clock_getres - EINVAL (NULL resolution)" );
  sc = clock_getres( CLOCK_REALTIME, NULL );
  rtems_test_assert( sc == -1 );
  rtems_test_assert( errno == EINVAL );

  puts( "Init: clock_getres - SUCCESSFUL" );
  sc = clock_getres( CLOCK_REALTIME, &tv );
  printf( "Init: resolution = sec (%ld), nsec (%ld)\n", tv.tv_sec, tv.tv_nsec );
  rtems_test_assert( !sc );

  /* set the time of day, and print our buffer in multiple ways */

  tv.tv_sec = mktime( &tm );
  rtems_test_assert( tv.tv_sec != -1 );

  tv.tv_nsec = 0;

  /* now set the time of day */

  empty_line();

  printf( asctime( &tm ) );
  puts( "Init: clock_settime - SUCCESSFUL" );
  sc = clock_settime( CLOCK_REALTIME, &tv );
  rtems_test_assert( !sc );

  printf( asctime( &tm ) );
  printf( ctime( &tv.tv_sec ) );

  /* use sleep to delay */

  remaining = sleep( 3 );
  rtems_test_assert( !remaining );
  
  /* print new times to make sure it has changed and we can get the realtime */
  sc = clock_gettime( CLOCK_PROCESS_CPUTIME, &tv );
  rtems_test_assert( !sc );
  printf("Time since boot: (%d, %d)\n", tv.tv_sec,tv.tv_nsec );

  sc = clock_gettime( CLOCK_REALTIME, &tv );
  rtems_test_assert( !sc );

  printf( ctime( &tv.tv_sec ) );

  seconds = time( NULL );
  printf( ctime( &seconds ) );

  /*  just to have the value copied out through the parameter */

  seconds = time( &seconds1 );
  rtems_test_assert( seconds == seconds1 );

  /* check the time remaining */

  printf( "Init: seconds remaining (%d)\n", (int)remaining );
  rtems_test_assert( !remaining );

  /* error cases in nanosleep */

  empty_line();
  puts( "Init: nanosleep - EINVAL (NULL time)" );
  sc = nanosleep ( NULL, &tr );
  rtems_test_assert( sc == -1 );
  rtems_test_assert( errno == EINVAL );

  tv.tv_sec = 0;
  tv.tv_nsec = TOD_NANOSECONDS_PER_SECOND * 2;
  puts( "Init: nanosleep - EINVAL (too many nanoseconds)" );
  sc = nanosleep ( &tv, &tr );
  rtems_test_assert( sc == -1 );
  rtems_test_assert( errno == EINVAL );

  /* this is actually a small delay or yield */
  tv.tv_sec = -1;
  tv.tv_nsec = 0;
  puts( "Init: nanosleep - negative seconds small delay " );
  sc = nanosleep ( &tv, &tr );
  rtems_test_assert( sc == -1 );
  rtems_test_assert( errno == EINVAL );

  /* use nanosleep to yield */

  tv.tv_sec = 0;
  tv.tv_nsec = 0;

  puts( "Init: nanosleep - yield with remaining" );
  sc = nanosleep ( &tv, &tr );
  rtems_test_assert( !sc );
  rtems_test_assert( !tr.tv_sec );
  rtems_test_assert( !tr.tv_nsec );

  puts( "Init: nanosleep - yield with NULL time remaining" );
  sc = nanosleep ( &tv, NULL );
  rtems_test_assert( !sc );
  rtems_test_assert( !tr.tv_sec );
  rtems_test_assert( !tr.tv_nsec );

  /* use nanosleep to delay */

  tv.tv_sec = 3;
  tv.tv_nsec = 500000;

  puts( "Init: nanosleep - 1.05 seconds" );
  sc = nanosleep ( &tv, &tr );
  rtems_test_assert( !sc );

  /* print the current real time again */
  sc = clock_gettime( CLOCK_REALTIME, &tv );
  rtems_test_assert( !sc );
  printf( ctime( &tv.tv_sec ) );

  /* check the time remaining */

  printf( "Init: sec (%ld), nsec (%ld) remaining\n", tr.tv_sec, tr.tv_nsec );
  rtems_test_assert( !tr.tv_sec && !tr.tv_nsec );

  puts( "Init: usleep - 1.35 seconds" );
  useconds = usleep ( 1350000 );
  rtems_test_assert( useconds < 1350000 );
  
  /* print the current real time again */
  sc = clock_gettime( CLOCK_REALTIME, &tv );
  rtems_test_assert( !sc );
  printf( ctime( &tv.tv_sec ) );

  puts( "" );
  puts( "clock_gettime - CLOCK_THREAD_CPUTIME -- ENOSYS" );
  #if defined(_POSIX_THREAD_CPUTIME)
    {
      struct timespec tp;
      sc = clock_gettime( CLOCK_THREAD_CPUTIME, &tp );
      check_enosys( sc );
    }
  #endif

  puts( "clock_settime - CLOCK_PROCESS_CPUTIME -- ENOSYS" );
  #if defined(_POSIX_CPUTIME)
    {
      struct timespec tp;
      sc = clock_settime( CLOCK_PROCESS_CPUTIME, &tp );
      check_enosys( sc );
    }
  #endif

  puts( "clock_settime - CLOCK_THREAD_CPUTIME -- ENOSYS" );
  #if defined(_POSIX_THREAD_CPUTIME)
    {
      struct timespec tp;
      sc = clock_settime( CLOCK_THREAD_CPUTIME, &tp );
      check_enosys( sc );
    }
  #endif

  puts( "*** END OF POSIX CLOCK TEST ***" );
  rtems_test_exit(0);
}


/* configuration information */
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_TASKS             1

#define CONFIGURE_INIT
#include <rtems/confdefs.h>
