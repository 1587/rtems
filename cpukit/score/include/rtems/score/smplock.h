/**
 * @file
 *
 * @ingroup ScoreSMPLock
 *
 * @brief SMP Lock API
 */

/*
 * COPYRIGHT (c) 1989-2011.
 * On-Line Applications Research Corporation (OAR).
 *
 * Copyright (c) 2013, 2016 embedded brains GmbH
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.org/license/LICENSE.
 */

#ifndef _RTEMS_SCORE_SMPLOCK_H
#define _RTEMS_SCORE_SMPLOCK_H

#include <rtems/score/cpuopts.h>

#if defined(RTEMS_SMP)

#include <rtems/score/smplockstats.h>
#include <rtems/score/smplockticket.h>
#include <rtems/score/isrlevel.h>

#if defined(RTEMS_PROFILING) || defined(RTEMS_DEBUG)
#define RTEMS_SMP_LOCK_DO_NOT_INLINE
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup ScoreSMPLock SMP Locks
 *
 * @ingroup Score
 *
 * @brief The SMP lock provides mutual exclusion for SMP systems at the lowest
 * level.
 *
 * The SMP lock is implemented as a ticket lock.  This provides fairness in
 * case of concurrent lock attempts.
 *
 * This SMP lock API uses a local context for acquire and release pairs.  Such
 * a context may be used to implement for example the Mellor-Crummey and Scott
 * (MCS) locks in the future.
 *
 * @{
 */

/**
 * @brief SMP lock control.
 */
typedef struct {
  SMP_ticket_lock_Control Ticket_lock;
#if defined(RTEMS_DEBUG)
  /**
   * @brief The index of the owning processor of this lock.
   *
   * The processor index is used instead of the executing thread, so that this
   * works in interrupt and system initialization context.  It is assumed that
   * thread dispatching is disabled in SMP lock critical sections.
   *
   * In case the lock is free, then the value of this field is
   * SMP_LOCK_NO_OWNER.
   *
   * @see _SMP_lock_Is_owner().
   */
  uint32_t owner;
#endif
#if defined(RTEMS_PROFILING)
  SMP_lock_Stats Stats;
#endif
} SMP_lock_Control;

/**
 * @brief Local SMP lock context for acquire and release pairs.
 */
typedef struct {
  ISR_Level isr_level;
#if defined(RTEMS_DEBUG)
  SMP_lock_Control *lock_used_for_acquire;
#endif
#if defined(RTEMS_PROFILING)
  SMP_lock_Stats_context Stats_context;
#endif
} SMP_lock_Context;

#if defined(RTEMS_DEBUG)
#define SMP_LOCK_NO_OWNER 0xffffffff
#endif

/**
 * @brief SMP lock control initializer for static initialization.
 */
#if defined(RTEMS_DEBUG) && defined(RTEMS_PROFILING)
  #define SMP_LOCK_INITIALIZER( name ) \
    { \
      SMP_TICKET_LOCK_INITIALIZER, \
      SMP_LOCK_NO_OWNER, \
      SMP_LOCK_STATS_INITIALIZER( name ) \
    }
#elif defined(RTEMS_DEBUG)
  #define SMP_LOCK_INITIALIZER( name ) \
    { SMP_TICKET_LOCK_INITIALIZER, SMP_LOCK_NO_OWNER }
#elif defined(RTEMS_PROFILING)
  #define SMP_LOCK_INITIALIZER( name ) \
    { SMP_TICKET_LOCK_INITIALIZER, SMP_LOCK_STATS_INITIALIZER( name ) }
#else
  #define SMP_LOCK_INITIALIZER( name ) { SMP_TICKET_LOCK_INITIALIZER }
#endif

/**
 * @brief Initializes an SMP lock.
 *
 * Concurrent initialization leads to unpredictable results.
 *
 * @param[in] lock The SMP lock control.
 * @param[in] name The name for the SMP lock statistics.  This name must be
 * persistent throughout the life time of this statistics block.
 */
#if defined(RTEMS_SMP_LOCK_DO_NOT_INLINE)
void _SMP_lock_Initialize(
  SMP_lock_Control *lock,
  const char *name
);

static inline void _SMP_lock_Initialize_body(
#else
static inline void _SMP_lock_Initialize(
#endif
  SMP_lock_Control *lock,
  const char *name
)
{
  _SMP_ticket_lock_Initialize( &lock->Ticket_lock );
#if defined(RTEMS_DEBUG)
  lock->owner = SMP_LOCK_NO_OWNER;
#endif
#if defined(RTEMS_PROFILING)
  _SMP_lock_Stats_initialize( &lock->Stats, name );
#else
  (void) name;
#endif
}

/**
 * @brief Destroys an SMP lock.
 *
 * Concurrent destruction leads to unpredictable results.
 *
 * @param[in] lock The SMP lock control.
 */
#if defined(RTEMS_SMP_LOCK_DO_NOT_INLINE)
void _SMP_lock_Destroy( SMP_lock_Control *lock );

static inline void _SMP_lock_Destroy_body( SMP_lock_Control *lock )
#else
static inline void _SMP_lock_Destroy( SMP_lock_Control *lock )
#endif
{
  _SMP_ticket_lock_Destroy( &lock->Ticket_lock );
  _SMP_lock_Stats_destroy( &lock->Stats );
}

/**
 * @brief Acquires an SMP lock.
 *
 * This function will not disable interrupts.  The caller must ensure that the
 * current thread of execution is not interrupted indefinite once it obtained
 * the SMP lock.
 *
 * @param[in] lock The SMP lock control.
 * @param[in] context The local SMP lock context for an acquire and release
 * pair.
 */
#if defined(RTEMS_SMP_LOCK_DO_NOT_INLINE)
void _SMP_lock_Acquire(
  SMP_lock_Control *lock,
  SMP_lock_Context *context
);

static inline void _SMP_lock_Acquire_body(
#else
static inline void _SMP_lock_Acquire(
#endif
  SMP_lock_Control *lock,
  SMP_lock_Context *context
)
{
  (void) context;
  _SMP_ticket_lock_Acquire(
    &lock->Ticket_lock,
    &lock->Stats,
    &context->Stats_context
  );
}

/**
 * @brief Releases an SMP lock.
 *
 * @param[in] lock The SMP lock control.
 * @param[in] context The local SMP lock context for an acquire and release
 * pair.
 */
#if defined(RTEMS_SMP_LOCK_DO_NOT_INLINE)
void _SMP_lock_Release(
  SMP_lock_Control *lock,
  SMP_lock_Context *context
);

static inline void _SMP_lock_Release_body(
#else
static inline void _SMP_lock_Release(
#endif
  SMP_lock_Control *lock,
  SMP_lock_Context *context
)
{
  (void) context;
  _SMP_ticket_lock_Release(
    &lock->Ticket_lock,
    &context->Stats_context
  );
}

/**
 * @brief Disables interrupts and acquires the SMP lock.
 *
 * @param[in] lock The SMP lock control.
 * @param[in] context The local SMP lock context for an acquire and release
 * pair.
 */
#if defined(RTEMS_SMP_LOCK_DO_NOT_INLINE)
void _SMP_lock_ISR_disable_and_acquire(
  SMP_lock_Control *lock,
  SMP_lock_Context *context
);

static inline void _SMP_lock_ISR_disable_and_acquire_body(
#else
static inline void _SMP_lock_ISR_disable_and_acquire(
#endif
  SMP_lock_Control *lock,
  SMP_lock_Context *context
)
{
  _ISR_Local_disable( context->isr_level );
  _SMP_lock_Acquire( lock, context );
}

/**
 * @brief Releases the SMP lock and enables interrupts.
 *
 * @param[in] lock The SMP lock control.
 * @param[in] context The local SMP lock context for an acquire and release
 * pair.
 */
#if defined(RTEMS_SMP_LOCK_DO_NOT_INLINE)
void _SMP_lock_Release_and_ISR_enable(
  SMP_lock_Control *lock,
  SMP_lock_Context *context
);
#else
static inline void _SMP_lock_Release_and_ISR_enable(
  SMP_lock_Control *lock,
  SMP_lock_Context *context
)
{
  _SMP_lock_Release( lock, context );
  _ISR_Local_enable( context->isr_level );
}
#endif

#if defined(RTEMS_DEBUG)
/**
 * @brief Returns true, if the SMP lock is owned by the current processor,
 * otherwise false.
 *
 * @param[in] lock The SMP lock control.
 */
bool _SMP_lock_Is_owner( const SMP_lock_Control *lock );
#endif

/**@}*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RTEMS_SMP */

#endif /* _RTEMS_SCORE_SMPLOCK_H */
