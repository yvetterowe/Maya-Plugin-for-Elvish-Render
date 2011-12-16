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

#include <eiCORE/ei_rwlock.h>

void ei_create_rwlock(eiRWLock *lock)
{
	eiDBG_ASSERT(lock != NULL);

	ei_atomic_set(&lock->ref_count, 0);
}

void ei_delete_rwlock(eiRWLock *lock)
{
	eiDBG_ASSERT(lock != NULL);
}

eiBool ei_try_write_lock(eiRWLock *lock)
{
	eiInt initial;

	eiDBG_ASSERT(lock != NULL);

	initial = ei_atomic_cas(&lock->ref_count, eiMIN_INT, 0);

	return (initial == 0);
}

void ei_write_lock(eiRWLock *lock)
{
	eiInt initial;

	eiDBG_ASSERT(lock != NULL);

	for (;;)
	{
		initial = ei_atomic_cas(&lock->ref_count, eiMIN_INT, 0);

		if (initial == 0)
		{
			return;
		}
		else
		{
			while (ei_atomic_read(&lock->ref_count) != 0)
			{
				ei_pause();
			}
		}
	}
}

void ei_write_unlock(eiRWLock *lock)
{
	eiDBG_ASSERT(lock != NULL);

	ei_atomic_swap(&lock->ref_count, 0);
}

void ei_upgrade_lock(eiRWLock *lock)
{
	eiDBG_ASSERT(lock != NULL);

	ei_read_unlock(lock);
	ei_write_lock(lock);
}

void ei_downgrade_lock(eiRWLock *lock)
{
	eiDBG_ASSERT(lock != NULL);

	ei_atomic_swap(&lock->ref_count, 1);
}

eiBool ei_is_read_locked(eiRWLock *lock)
{
	eiDBG_ASSERT(lock != NULL);

	return (ei_atomic_read(&lock->ref_count) > 0);
}

eiBool ei_is_write_locked(eiRWLock *lock)
{
	eiDBG_ASSERT(lock != NULL);

	return (ei_atomic_read(&lock->ref_count) < 0);
}

eiBool ei_is_locked(eiRWLock *lock)
{
	eiDBG_ASSERT(lock != NULL);

	return (ei_atomic_read(&lock->ref_count) != 0);
}
