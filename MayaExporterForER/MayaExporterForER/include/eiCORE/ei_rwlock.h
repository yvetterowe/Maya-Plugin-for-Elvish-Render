/*
 * Copyright 2010 elvish render Team
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef EI_RWLOCK_H
#define EI_RWLOCK_H

/** \brief This file contains a simple lock-free implementation 
 * of reader-writer lock.
 * \file ei_rwlock.h
 */

#include <eiCORE/ei_core.h>
#include <eiCORE/ei_atomic_ops.h>
#include <eiCORE/ei_assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Reader-writer lock state structure.
 */
typedef struct eiRWLock {
	eiAtomic	ref_count;
} eiRWLock;

/** \brief Create a reader-writer lock.
 */
eiCORE_API void ei_create_rwlock(eiRWLock *lock);

/** \brief Delete a reader-writer lock.
 */
eiCORE_API void ei_delete_rwlock(eiRWLock *lock);

/** \brief Acquire a lock for reading.
 */
eiFORCEINLINE void ei_read_lock(eiRWLock *lock)
{
	eiInt result;

	eiDBG_ASSERT(lock != NULL);

	for (;;)
	{
		result = ei_atomic_inc(&lock->ref_count);

		if (result > 0)
		{
			return;
		}
		else
		{
			while (ei_atomic_read(&lock->ref_count) < 0)
			{
				ei_pause();
			}
		}
	}
}

/** \brief Release a lock for reading.
 */
eiFORCEINLINE void ei_read_unlock(eiRWLock *lock)
{
	eiDBG_ASSERT(lock != NULL);

	ei_atomic_dec(&lock->ref_count);
}

/** \brief Try to acquire a lock for writing, returns 
 * whether the lock has successfully been acquired.
 */
eiCORE_API eiBool ei_try_write_lock(eiRWLock *lock);

/** \brief Acquire a lock for writing.
 */
eiCORE_API void ei_write_lock(eiRWLock *lock);

/** \brief Release a lock for writing.
 */
eiCORE_API void ei_write_unlock(eiRWLock *lock);

/** \brief If reader lock has already been acquired, 
 * you can use this function to upgrade the lock 
 * to writing priviledge.
 */
eiCORE_API void ei_upgrade_lock(eiRWLock *lock);

/** \brief If writer lock has already been acquired, 
 * you can use this function to downgrade the lock 
 * to reading priviledge.
 */
eiCORE_API void ei_downgrade_lock(eiRWLock *lock);

/** \brief Is the lock in locked state for reading.
 */
eiCORE_API eiBool ei_is_read_locked(eiRWLock *lock);

/** \brief Is the lock in locked state for writing.
 */
eiCORE_API eiBool ei_is_write_locked(eiRWLock *lock);

/** \brief Is the lock in locked state for either 
 * reading or writing.
 */
eiCORE_API eiBool ei_is_locked(eiRWLock *lock);

#ifdef __cplusplus
}
#endif

#endif
