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

#ifndef EI_ATOMIC_OPS_H
#define EI_ATOMIC_OPS_H

/** \brief This file contains macros for atomic operations.
 * \file ei_atomic_ops.h
 * \author Elvic Liang
 *
 * Atomic operations are all represented as macros for efficiency.
 * be aware that atomic integer requires serialized 
 * instructions which can be very inefficient, so do not use it whenever you 
 * could, for example, you can use many thread-local counters instead, and 
 * sum them at later stage if you just want some statistics.
 */

#include <eiCORE/ei_atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

/* early versions of SSE may have no _mm_pause defined, do not use 
   this intrinsic in that case to get the code compiled. */
#ifdef _mm_pause
#define ei_pause() _mm_pause()
#else
#define ei_pause() 
#endif

#define ei_atomic_read(v)	((v)->counter)
#define ei_atomic_set(v, i)	(((v)->counter) = (i))

#ifdef EI_OS_WINDOWS

#include <windows.h>

/* Use intrinsics for Interlocked functions to get faster assembly code */
void __cdecl __nop(void);
void __cdecl _ReadBarrier(void);
void __cdecl _WriteBarrier(void);
void __cdecl _ReadWriteBarrier(void);

LONG __cdecl _InterlockedExchangeAdd(LONG volatile *Addend, LONG Value);
LONG __cdecl _InterlockedIncrement(LONG volatile *Addend);
LONG __cdecl _InterlockedDecrement(LONG volatile *Addend);
LONG __cdecl _InterlockedExchange(LONG volatile *Target, LONG Value);
LONG __cdecl _InterlockedCompareExchange(LONG volatile *Dest, LONG Exchange, LONG Comp);
LONG __cdecl _InterlockedAnd(LONG volatile *Dest, LONG Value);
LONG __cdecl _InterlockedOr(LONG volatile *Dest, LONG Value);

#pragma intrinsic(__nop)
#pragma intrinsic(_ReadBarrier)
#pragma intrinsic(_WriteBarrier)
#pragma intrinsic(_ReadWriteBarrier)

#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedAnd)
#pragma intrinsic(_InterlockedOr)

#define ei_nop() __nop()
#define ei_read_barrier() _ReadBarrier()
#define ei_write_barrier() _WriteBarrier()
#define ei_read_write_barrier() _ReadWriteBarrier()

/* to workaround the differences of atomic operations on Windows and Linux, 
   we manually add value on Windows to return the added value to emulate Linux. */
eiFORCEINLINE eiInt ei_atomic_add(eiAtomic *destination, eiInt value)
{
	return _InterlockedExchangeAdd(&(destination->counter), value) + value;
}
#define ei_atomic_inc(destination) _InterlockedIncrement(&(destination)->counter)
#define ei_atomic_dec(destination) _InterlockedDecrement(&(destination)->counter)
#define ei_atomic_swap(destination, value) _InterlockedExchange(&(destination)->counter, (value))
#define ei_atomic_cas(destination, exchange, comperand) _InterlockedCompareExchange(&(destination)->counter, (exchange), (comperand))
#define ei_atomic_clear_mask(destination, value) _InterlockedAnd(&(destination)->counter, ~(value))
#define ei_atomic_set_mask(destination, value) _InterlockedOr(&(destination)->counter, (value))

#ifdef EI_ARCH_X64

/* to workaround the differences of atomic operations on Windows and Linux, 
   we manually add value on Windows to return the added value to emulate Linux. */
eiFORCEINLINE eiInt64 ei_atomic_add64(eiAtomic64 *destination, eiInt64 value)
{
	return _InterlockedExchangeAdd64(&(destination->counter), value) + value;
}
#define ei_atomic_inc64(destination) _InterlockedIncrement64(&(destination)->counter)
#define ei_atomic_dec64(destination) _InterlockedDecrement64(&(destination)->counter)
#define ei_atomic_swap64(destination, value) _InterlockedExchange64(&(destination)->counter, (value))
#define ei_atomic_cas64(destination, exchange, comperand) _InterlockedCompareExchange64(&(destination)->counter, (exchange), (comperand))

#endif

#else

#include <asm/atomic.h>
#include <asm/system.h>

#define ei_nop() nop()
#define ei_read_barrier() rmb()
#define ei_write_barrier() wmb()
#define ei_read_write_barrier() mb()

#define ei_atomic_add(destination, value) atomic_add_return((value), (destination))
#define ei_atomic_inc(destination) atomic_inc_return((destination))
#define ei_atomic_dec(destination) atomic_dec_return((destination))
#define ei_atomic_swap(destination, value) atomic_xchg((destination), (value))
#define ei_atomic_cas(destination, exchange, comperand) atomic_cmpxchg((destination), (comperand), (exchange))
#define ei_atomic_clear_mask(destination, value) atomic_clear_mask((value), (destination))
#define ei_atomic_set_mask(destination, value) atomic_set_mask((value), (destination))

#ifdef EI_ARCH_X64

#define ei_atomic_add64(destination, value) atomic64_add_return((value), (destination))
#define ei_atomic_inc64(destination) atomic64_inc_return((destination))
#define ei_atomic_dec64(destination) atomic64_dec_return((destination))
#define ei_atomic_swap64(destination, value) atomic_xchg((destination), (value))
#define ei_atomic_cas64(destination, exchange, comperand) atomic_cmpxchg((destination), (comperand), (exchange))

#endif

#endif

#define ei_atomic_sub(destination, value) ei_atomic_add((destination), -((eiInt)(value)))
#define ei_atomic_sub64(destination, value) ei_atomic_add64((destination), -((eiInt64)(value)))

#ifdef EI_ARCH_X64

#define ei_atomic_add_ptr ei_atomic_add64
#define ei_atomic_inc_ptr ei_atomic_inc64
#define ei_atomic_dec_ptr ei_atomic_dec64
#define ei_atomic_swap_ptr ei_atomic_swap64
#define ei_atomic_cas_ptr ei_atomic_cas64
#define ei_atomic_sub_ptr ei_atomic_sub64

#else

#define ei_atomic_add_ptr ei_atomic_add
#define ei_atomic_inc_ptr ei_atomic_inc
#define ei_atomic_dec_ptr ei_atomic_dec
#define ei_atomic_swap_ptr ei_atomic_swap
#define ei_atomic_cas_ptr ei_atomic_cas
#define ei_atomic_sub_ptr ei_atomic_sub

#endif

#ifdef __cplusplus
}
#endif

#endif
