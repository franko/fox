/********************************************************************************
*                                                                               *
*                         A t o m i c   O p e r a t i o n s                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 2006,2024 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
********************************************************************************/
#ifndef FXATOMIC_H
#define FXATOMIC_H


namespace FX {


/// Thread fence
static inline void atomicThreadFence(){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  _ReadWriteBarrier();
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  __sync_synchronize();
#else
#warning "atomicThreadFence(): not implemented."
#endif
  }


///// Atomic integers


/// Atomically set variable at ptr to v, and return its old contents
static inline FXint atomicSet(volatile FXint* ptr,FXint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedExchange((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_exchange_n(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_lock_test_and_set(ptr,v);
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
  FXint ret=v;
  __asm__ __volatile__("xchgl %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(ret) : "memory", "cc");
  return ret;
#else
#warning "atomicSet(volatile FXint*,FXint): not implemented."
  FXint ret=*ptr; *ptr=v;
  return ret;
#endif
  }


/// Atomically compare variable at ptr against expect, setting it to v if equal; returns the old value at ptr
static inline FXint atomicCas(volatile FXint* ptr,FXint expect,FXint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedCompareExchange((volatile long*)ptr,(long)v,(long)expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  FXint ex=expect;
  __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
  return ex;
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_val_compare_and_swap(ptr,expect,v);
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
  FXint ret;
  __asm__ __volatile__("lock\n\t"
                       "cmpxchgl %2,(%1)\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#else
#warning "atomicCas(volatile FXint*,FXint,FXint): not implemented."
  FXint ret=*ptr;
  if(*ptr==expect){ *ptr=v; }
  return ret;
#endif
  }


/// Atomically compare variable at ptr against expect, setting it to v if equal and return true, or false otherwise
static inline FXbool atomicBoolCas(volatile FXint* ptr,FXint expect,FXint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return (_InterlockedCompareExchange((volatile long*)ptr,(long)v,(long)expect)==(long)expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_INT_LOCK_FREE == 2)
  FXint ex=expect;
  return __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_bool_compare_and_swap(ptr,expect,v);
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
  FXbool ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchgl %2,(%1)\n\t"
                        "sete %%al\n\t"
                        "andl $1, %%eax\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#else
#warning "atomicBoolCas(volatile FXint*,FXint,FXint): not implemented."
  if(*ptr==expect){ *ptr=v; return true; }
  return false;
#endif
  }


/// Atomically add v to variable at ptr, and return its old contents
static inline FXint atomicAdd(volatile FXint* ptr,FXint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedExchangeAdd((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_fetch_add(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_add(ptr,v);
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
  FXint ret=v;
  __asm__ __volatile__ ("lock\n\t"
                        "xaddl %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(ret) : "memory", "cc");
  return ret;
#else
#warning "atomicAdd(volatile FXint*,FXint): not implemented."
  FXint ret=*ptr; *ptr+=v;
  return ret;
#endif
  }


/// Atomically bitwise AND v to variable at ptr, and return its old contents
static inline FXint atomicAnd(volatile FXint* ptr,FXint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedAnd((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_fetch_and(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_and(ptr,v);
#else
#warning "atomicAnd(volatile FXint*,FXint): not implemented."
  FXint ret=*ptr; *ptr&=v;
  return ret;
#endif
  }


/// Atomically bitwise OR v to variable at ptr, and return its old contents
static inline FXint atomicOr(volatile FXint* ptr,FXint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedOr((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_fetch_or(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_or(ptr,v);
#else
#warning "atomicOr(volatile FXint*,FXint): not implemented."
  FXint ret=*ptr; *ptr|=v;
  return ret;
#endif
  }


/// Atomically bitwise XOR v to variable at ptr, and return its old contents
static inline FXint atomicXor(volatile FXint* ptr,FXint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedXor((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_fetch_xor(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_xor(ptr,v);
#else
#warning "atomicXor(volatile FXint*,FXint): not implemented."
  FXint ret=*ptr; *ptr^=v;
  return ret;
#endif
  }


/// Atomically set *ptr to minimum of *ptr and v, and return its old contents
static inline FXint atomicMin(volatile FXint* ptr,FXint v){
  FXint old;
  while(v<(old=*ptr) && !atomicBoolCas(ptr,old,v)){ }
  return old;
  }


/// Atomically set *ptr to maximum of *ptr and v, and return its old contents
static inline FXint atomicMax(volatile FXint* ptr,FXint v){
  FXint old;
  while((old=*ptr)<v && !atomicBoolCas(ptr,old,v)){ }
  return old;
  }


///// Atomic unsigned integers


/// Atomically set variable at ptr to v, and return its old contents
static inline FXuint atomicSet(volatile FXuint* ptr,FXuint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedExchange((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_exchange_n(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_lock_test_and_set(ptr,v);
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
  FXuint ret=v;
  __asm__ __volatile__("xchgl %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(ret) : "memory", "cc");
  return ret;
#else
#warning "atomicSet(volatile FXuint*,FXuint): not implemented."
  FXuint ret=*ptr; *ptr=v;
#endif
  }


/// Atomically compare variable at ptr against expect, setting it to v if equal; returns the old value at ptr
static inline FXuint atomicCas(volatile FXuint* ptr,FXuint expect,FXuint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedCompareExchange((volatile long*)ptr,(long)v,(long)expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  FXuint ex=expect;
  __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
  return ex;
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_val_compare_and_swap(ptr,expect,v);
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
  FXuint ret;
  __asm__ __volatile__("lock\n\t"
                       "cmpxchgl %2,(%1)\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#else
#warning "atomicCas(volatile FXuint*,FXuint,FXuint): not implemented."
  FXuint ret=*ptr;
  if(*ptr==expect){ *ptr=v; }
  return ret;
#endif
  }


/// Atomically compare variable at ptr against expect, setting it to v if equal and return true, or false otherwise
static inline FXbool atomicBoolCas(volatile FXuint* ptr,FXuint expect,FXuint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return (_InterlockedCompareExchange((volatile long*)ptr,(long)v,(long)expect)==(long)expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_INT_LOCK_FREE == 2)
  FXuint ex=expect;
  return __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_bool_compare_and_swap(ptr,expect,v);
#else
#warning "atomicBoolCas(volatile FXuint*,FXuint,FXuint): not implemented."
  if(*ptr==expect){ *ptr=v; return true; }
  return false;
#endif
  }


/// Atomically add v to variable at ptr, and return its old contents
static inline FXuint atomicAdd(volatile FXuint* ptr,FXuint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedExchangeAdd((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_fetch_add(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_add(ptr,v);
#elif (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
  FXuint ret=v;
  __asm__ __volatile__ ("lock\n\t"
                        "xaddl %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(ret) : "memory", "cc");
  return ret;
#else
#warning "atomicAdd(volatile FXuint*,FXuint): not implemented."
  FXuint ret=*ptr; *ptr+=v;
  return ret;
#endif
  }


/// Atomically bitwise AND v to variable at ptr, and return its old contents
static inline FXuint atomicAnd(volatile FXuint* ptr,FXuint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedAnd((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_fetch_and(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_and(ptr,v);
#else
#warning "atomicAnd(volatile FXuint*,FXuint): not implemented."
  FXuint ret=*ptr; *ptr&=v;
  return ret;
#endif
  }


/// Atomically bitwise OR v to variable at ptr, and return its old contents
static inline FXuint atomicOr(volatile FXuint* ptr,FXuint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedOr((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_fetch_or(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_or(ptr,v);
#else
#warning "atomicOr(volatile FXuint*,FXuint): not implemented."
  FXuint ret=*ptr; *ptr|=v;
  return ret;
#endif
  }


/// Atomically bitwise XOR v to variable at ptr, and return its old contents
static inline FXuint atomicXor(volatile FXuint* ptr,FXuint v){
#if defined(_WIN32) && (_MSC_VER >= 1500)
  return _InterlockedXor((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST)
  return __atomic_fetch_xor(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_xor(ptr,v);
#else
#warning "atomicXor(volatile FXuint*,FXuint): not implemented."
  FXuint ret=*ptr; *ptr^=v;
  return ret;
#endif
  }


/// Atomically set *ptr to minimum of *ptr and v, and return its old contents
static inline FXuint atomicMin(volatile FXuint* ptr,FXuint v){
  FXuint old;
  while(v<(old=*ptr) && !atomicBoolCas(ptr,old,v)){ }
  return old;
  }


/// Atomically set *ptr to maximum of *ptr and v, and return its old contents
static inline FXuint atomicMax(volatile FXuint* ptr,FXuint v){
  FXuint old;
  while((old=*ptr)<v && !atomicBoolCas(ptr,old,v)){ }
  return old;
  }


///// Atomic longs


/// Atomically set variable at ptr to v, and return its old contents
static inline FXlong atomicSet(volatile FXlong* ptr,FXlong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedExchange64(ptr,v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_exchange_n(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_lock_test_and_set(ptr,v);
#elif (defined(__GNUC__) && defined(__i386__) && (defined(__PIC__) || defined(__PIE__)))
  FXlong ret;
  __asm__ __volatile__ ("xchgl %%esi, %%ebx\n\t"
                        "1:\n\t"
                        "lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "jnz 1b\n\t"
                        "xchgl %%esi, %%ebx\n\t" : "=A"(ret) : "D"(ptr), "S"((TInt)v), "c"((TInt)(v>>32)), "A"(*ptr) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__i386__))
  FXlong ret;
  __asm__ __volatile__ ("1:\n\t"
                        "lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "jnz 1b\n\t" : "=A"(ret) : "D"(ptr), "b"((TInt)v), "c"((TInt)(v>>32)), "A"(*ptr) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXlong ret;
  __asm__ __volatile__("xchgq %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(v) : "memory", "cc");
  return ret;
#else
#warning "atomicSet(volatile FXlong*,FXlong): not implemented."
  FXlong ret=*ptr; *ptr=v;
  return ret;
#endif
  }


/// Atomically compare variable at ptr against expect, setting it to v if equal; returns the old value at ptr
static inline FXlong atomicCas(volatile FXlong* ptr,FXlong expect,FXlong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedCompareExchange64(ptr,v,expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  FXlong ex=expect;
  __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
  return ex;
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_val_compare_and_swap(ptr,expect,v);
#elif (defined(__GNUC__) && defined(__i386__) && (defined(__PIC__) || defined(__PIE__)))
  FXlong ret;
  __asm__ __volatile__ ("xchgl %%esi, %%ebx\n\t"
                        "lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "movl %%esi, %%ebx\n\t" : "=A"(ret) : "D"(ptr), "S"((FXuint)v), "c"((FXuint)(v>>32)), "A"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__i386__))
  FXlong ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchg8b (%1)\n\t" : "=A"(ret) : "D"(ptr), "b"((FXuint)v), "c"((FXuint)(v>>32)), "A"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXlong ret;
  __asm__ __volatile__("lock\n\t"
                       "cmpxchgq %2,(%1)\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#else
#warning "atomicCas(volatile FXlong*,FXlong,FXlong): not implemented."
  FXlong ret=*ptr;
  if(*ptr==expect){ *ptr=v; }
  return ret;
#endif
  }


/// Atomically compare variable at ptr against expect, setting it to v if equal and return true, or false otherwise
static inline FXbool atomicBoolCas(volatile FXlong* ptr,FXlong expect,FXlong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return (_InterlockedCompareExchange64(ptr,v,expect)==expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  FXlong ex=expect;
  return __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_bool_compare_and_swap(ptr,expect,v);
#elif (defined(__GNUC__) && defined(__i386__) && (defined(__PIC__) || defined(__PIE__)))
  FXbool ret;
  __asm__ __volatile__ ("xchgl %%esi, %%ebx\n\t"
                        "lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "setz %%al\n\t"
                        "andl $1, %%eax\n\t"
                        "xchgl %%esi, %%ebx\n\t" : "=A"(ret) : "D"(ptr), "S"((FXuint)v), "c"((FXuint)(v>>32)), "A"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__i386__))
  FXbool ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "setz %%al\n\t"
                        "andl $1, %%eax\n\t" : "=a"(ret) : "D"(ptr), "b"((FXuint)v), "c"((FXuint)(v>>32)), "A"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXbool ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchgq %2,(%1)\n\t"
                        "sete %%al\n\t"
                        "andq $1, %%rax\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#else
#warning "atomicBoolCas(volatile FXlong*,FXlong,FXlong): not implemented."
  if(*ptr==expect){ *ptr=v; return true; }
  return false;
#endif
  }


/// Atomically add v to variable at ptr, and return its old contents
static inline FXlong atomicAdd(volatile FXlong* ptr,FXlong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedExchangeAdd64(ptr,v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_fetch_add(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_add(ptr,v);
#elif (defined(__GNUC__) && defined(__i386__) && (defined(__PIC__) || defined(__PIE__)))
  FXuint inclo=v;
  FXuint inchi=(v>>32);
  FXlong ret;
  __asm __volatile("movl %%ebx, %%esi\n\t"
                   "1:\n\t"
                   "movl %2, %%ebx\n\t"
                   "movl %3, %%ecx\n\t"
                   "addl %%eax, %%ebx\n\t"
                   "addc %%edx, %%ecx\n\t"
                   "lock\n\t"
                   "cmpxchg8b (%1)\n\t"
                   "jnz 1b\n\t"
                   "movl %%esi, %%ebx\n\t" : "=A"(ret) : "D"(ptr), "m"(inclo), "m"(inchi), "A"(*ptr) : "esi", "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__i386__))
  FXuint inclo=v;
  FXuint inchi=(v>>32);
  FXlong ret;
  __asm __volatile("1:\n\t"
                   "movl %2, %%ebx\n\t"
                   "movl %3, %%ecx\n\t"
                   "addl %%eax, %%ebx\n\t"
                   "addc %%edx, %%ecx\n\t"
                   "lock\n\t"
                   "cmpxchg8b (%1)\n\t"
                   "jnz 1b\n\t" : "=A"(ret) : "D"(ptr), "m"(inclo), "m"(inchi), "A"(*ptr) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXlong ret;
  __asm__ __volatile__ ("lock\n\t"
                        "xaddq %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(v) : "memory", "cc");
  return ret;
#else
#warning "atomicAdd(volatile FXlong*,FXlong): not implemented."
  FXlong ret=*ptr; *ptr+=v;
  return ret;
#endif
  }


/// Atomically bitwise AND v to variable at ptr, and return its old contents
static inline FXlong atomicAnd(volatile FXlong* ptr,FXlong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedAnd64(ptr,v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_fetch_and(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_and(ptr,v);
#else
#warning "atomicAnd(volatile FXlong*,FXlong): not implemented."
  FXlong ret=*ptr; *ptr&=v;
  return ret;
#endif
  }


/// Atomically bitwise OR v to variable at ptr, and return its old contents
static inline FXlong atomicOr(volatile FXlong* ptr,FXlong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedOr64(ptr,v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_fetch_or(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_or(ptr,v);
#else
#warning "atomicOr(volatile FXlong*,FXlong): not implemented."
  FXlong ret=*ptr; *ptr|=v;
  return ret;
#endif
  }


/// Atomically bitwise XOR v to variable at ptr, and return its old contents
static inline FXlong atomicXor(volatile FXlong* ptr,FXlong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedXor64(ptr,v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_fetch_xor(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_xor(ptr,v);
#else
#warning "atomicXor(volatile FXlong*,FXlong): not implemented."
  FXlong ret=*ptr; *ptr^=v;
  return ret;
#endif
  }


/// Atomically set *ptr to minimum of *ptr and v, and return its old contents
static inline FXlong atomicMin(volatile FXlong* ptr,FXlong v){
  FXlong old;
  while(v<(old=*ptr) && !atomicBoolCas(ptr,old,v)){ }
  return old;
  }


/// Atomically set *ptr to maximum of *ptr and v, and return its old contents
static inline FXlong atomicMax(volatile FXlong* ptr,FXlong v){
  FXlong old;
  while((old=*ptr)<v && !atomicBoolCas(ptr,old,v)){ }
  return old;
  }


///// Atomic unsigned longs


/// Atomically set variable at ptr to v, and return its old contents
static inline FXulong atomicSet(volatile FXulong* ptr,FXulong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedExchange64((volatile FXlong*)ptr,(FXlong)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_exchange_n(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_lock_test_and_set(ptr,v);
#elif (defined(__GNUC__) && defined(__i386__) && (defined(__PIC__) || defined(__PIE__)))
  FXulong ret;
  __asm__ __volatile__ ("xchgl %%esi, %%ebx\n\t"
                        "1:\n\t"
                        "lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "jnz 1b\n\t"
                        "xchgl %%esi, %%ebx\n\t" : "=A"(ret) : "D"(ptr), "S"((FXuint)v), "c"((FXuint)(v>>32)), "A"(*ptr) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__i386__))
  FXulong ret;
  __asm__ __volatile__ ("1:\n\t"
                        "lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "jnz 1b\n\t" : "=A"(ret) : "D"(ptr), "b"((FXuint)v), "c"((FXuint)(v>>32)), "A"(*ptr) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXulong ret;
  __asm__ __volatile__("xchgq %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(v) : "memory", "cc");
  return ret;
#else
#warning "atomicSet(volatile FXulong*,FXulong): not implemented."
  FXulong ret=*ptr; *ptr=v;
  return ret;
#endif
  }


/// Atomically compare variable at ptr against expect, setting it to v if equal; returns the old value at ptr
static inline FXulong atomicCas(volatile FXulong* ptr,FXulong expect,FXulong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedCompareExchange64((volatile FXlong*)ptr,(FXlong)v,(FXlong)expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  FXulong ex=expect;
  __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
  return ex;
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_val_compare_and_swap(ptr,expect,v);
#elif (defined(__GNUC__) && defined(__i386__) && (defined(__PIC__) || defined(__PIE__)))
  FXulong ret;
  __asm__ __volatile__ ("xchgl %%esi, %%ebx\n\t"
                        "lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "movl %%esi, %%ebx\n\t" : "=A"(ret) : "D"(ptr), "S"((FXuint)v), "c"((FXuint)(v>>32)), "A"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__i386__))
  FXulong ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchg8b (%1)\n\t" : "=A"(ret) : "D"(ptr), "b"((FXuint)v), "c"((FXuint)(v>>32)), "A"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXulong ret;
  __asm__ __volatile__("lock\n\t"
                       "cmpxchgq %2,(%1)\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#else
#warning "atomicCas(volatile FXulong*,FXulong,FXulong): not implemented."
  FXulong ret=*ptr;
  if(*ptr==expect){ *ptr=v; }
  return ret;
#endif
  }


/// Atomically compare variable at ptr against expect, setting it to v if equal and return true, or false otherwise
static inline FXbool atomicBoolCas(volatile FXulong* ptr,FXulong expect,FXulong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return (_InterlockedCompareExchange64((volatile FXlong*)ptr,(FXlong)v,(FXlong)expect)==(FXlong)expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  FXulong ex=expect;
  return __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_bool_compare_and_swap(ptr,expect,v);
#elif (defined(__GNUC__) && defined(__i386__) && (defined(__PIC__) || defined(__PIE__)))
  FXbool ret;
  __asm__ __volatile__ ("xchgl %%esi, %%ebx\n\t"
                        "lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "setz %%al\n\t"
                        "andl $1, %%eax\n\t"
                        "xchgl %%esi, %%ebx\n\t" : "=A"(ret) : "D"(ptr), "S"((FXuint)v), "c"((FXuint)(v>>32)), "A"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__i386__))
  FXbool ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "setz %%al\n\t"
                        "andl $1, %%eax\n\t" : "=a"(ret) : "D"(ptr), "b"((FXuint)v), "c"((FXuint)(v>>32)), "A"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXbool ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchgq %2,(%1)\n\t"
                        "sete %%al\n\t"
                        "andq $1, %%rax\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#else
#warning "atomicBoolCas(volatile FXulong*,FXulong,FXulong): not implemented."
  if(*ptr==expect){ *ptr=v; return true; }
  return false;
#endif
  }


/// Atomically add v to variable at ptr, and return its old contents
static inline FXulong atomicAdd(volatile FXulong* ptr,FXulong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedExchangeAdd64((volatile FXlong*)ptr,(FXlong)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_fetch_add(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_add(ptr,v);
#elif (defined(__GNUC__) && defined(__i386__) && (defined(__PIC__) || defined(__PIE__)))
  FXuint inclo=v;
  FXuint inchi=(v>>32);
  FXulong ret;
  __asm __volatile("movl %%ebx, %%esi\n\t"
                   "1:\n\t"
                   "movl %2, %%ebx\n\t"
                   "movl %3, %%ecx\n\t"
                   "addl %%eax, %%ebx\n\t"
                   "addc %%edx, %%ecx\n\t"
                   "lock\n\t"
                   "cmpxchg8b (%1)\n\t"
                   "jnz 1b\n\t"
                   "movl %%esi, %%ebx\n\t" : "=A"(ret) : "D"(ptr), "m"(inclo), "m"(inchi), "A"(*ptr) : "esi", "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__i386__))
  FXuint inclo=v;
  FXuint inchi=(v>>32);
  FXulong ret;
  __asm __volatile("1:\n\t"
                   "movl %2, %%ebx\n\t"
                   "movl %3, %%ecx\n\t"
                   "addl %%eax, %%ebx\n\t"
                   "addc %%edx, %%ecx\n\t"
                   "lock\n\t"
                   "cmpxchg8b (%1)\n\t"
                   "jnz 1b\n\t" : "=A"(ret) : "D"(ptr), "m"(inclo), "m"(inchi), "A"(*ptr) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXulong ret;
  __asm__ __volatile__ ("lock\n\t"
                        "xaddq %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(v) : "memory", "cc");
  return ret;
#else
#warning "atomicAdd(volatile FXulong*,FXulong): not implemented."
  FXulong ret=*ptr; *ptr+=v;
  return ret;
#endif
  }


/// Atomically bitwise AND v to variable at ptr, and return its old contents
static inline FXulong atomicAnd(volatile FXulong* ptr,FXulong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedAnd64((volatile FXlong*)ptr,(FXlong)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_fetch_and(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_and(ptr,v);
#else
#warning "atomicAnd(volatile FXulong*,FXulong): not implemented."
  FXulong ret=*ptr; *ptr&=v;
  return ret;
#endif
  }


/// Atomically bitwise OR v to variable at ptr, and return its old contents
static inline FXulong atomicOr(volatile FXulong* ptr,FXulong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedOr64((volatile FXlong*)ptr,(FXlong)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_fetch_or(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_or(ptr,v);
#else
#warning "atomicOr(volatile FXulong*,FXulong): not implemented."
  FXulong ret=*ptr; *ptr|=v;
  return ret;
#endif
  }


/// Atomically bitwise XOR v to variable at ptr, and return its old contents
static inline FXulong atomicXor(volatile FXulong* ptr,FXulong v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedXor64((volatile FXlong*)ptr,(FXlong)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
  return __atomic_fetch_xor(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_xor(ptr,v);
#else
#warning "atomicXor(volatile FXulong*,FXulong): not implemented."
  FXulong ret=*ptr; *ptr|=v;
  return ret;
#endif
  }


/// Atomically set *ptr to minimum of *ptr and v, and return its old contents
static inline FXulong atomicMin(volatile FXulong* ptr,FXulong v){
  FXulong old;
  while(v<(old=*ptr) && !atomicBoolCas(ptr,old,v)){ }
  return old;
  }


/// Atomically set *ptr to maximum of *ptr and v, and return its old contents
static inline FXulong atomicMax(volatile FXulong* ptr,FXulong v){
  FXulong old;
  while((old=*ptr)<v && !atomicBoolCas(ptr,old,v)){ }
  return old;
  }


///// Atomic void pointers


/// Atomically set pointer variable at ptr to v, and return its old contents
static inline FXptr atomicSet(volatile FXptr* ptr,FXptr v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return (FXptr)_InterlockedExchangePointer(ptr, v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_POINTER_LOCK_FREE == 2)
  return __atomic_exchange_n(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return (TPtr)__sync_lock_test_and_set(ptr,v);
#elif (defined(__GNUC__) && defined(__i386__))
  FXptr ret=v;
  __asm__ __volatile__("xchgl %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(ret) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXptr ret=v;
  __asm__ __volatile__("xchgq %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(ret) : "memory", "cc");
  return ret;
#else
#warning "atomicSet(volatile FXptr*,FXptr): not implemented."
  FXptr ret=*ptr; *ptr=v;
  return ret;
#endif
  }


/// Atomically compare pointer variable at ptr against expect, setting it to v if equal; returns the old value at ptr
static inline FXptr atomicCas(volatile FXptr* ptr,FXptr expect,FXptr v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return _InterlockedCompareExchangePointer((void**)ptr,v,expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_POINTER_LOCK_FREE == 2)
  FXptr ex=expect;
  __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
  return ex;
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_val_compare_and_swap(ptr,expect,v);
#elif (defined(__GNUC__) && defined(__i386__))
  FXptr ret;
  __asm__ __volatile__("lock\n\t"
                       "cmpxchgl %2,(%1)\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXptr ret;
  __asm__ __volatile__("lock\n\t"
                       "cmpxchgq %2,(%1)\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#else
#warning "atomicCas(volatile FXptr*,FXptr,FXptr): not implemented."
  FXptr ret=*ptr;
  if(*ptr==expect){ *ptr=v; }
  return ret;
#endif
  }


/// Atomically compare pointer variable at ptr against expect, setting it to v if equal and return true, or false otherwise
static inline FXbool atomicBoolCas(volatile FXptr* ptr,FXptr expect,FXptr v){
#if defined(_WIN32) && (_MSC_VER >= 1800)
  return (_InterlockedCompareExchangePointer((void**)ptr,v,expect)==expect);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_POINTER_LOCK_FREE == 2)
  FXptr ex=expect;
  return __atomic_compare_exchange_n(ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_bool_compare_and_swap(ptr,expect,v);
#elif (defined(__GNUC__) && defined(__i386__))
  FXbool ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchgl %2,(%1)\n\t"
                        "sete %%al\n\t"
                        "andl $1, %%eax\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXbool ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchgq %2,(%1)\n\t"
                        "sete %%al\n\t"
                        "andq $1, %%rax\n\t" : "=a"(ret) : "r"(ptr), "r"(v), "a"(expect) : "memory", "cc");
  return ret;
#else
#warning "atomicBoolCas(volatile FXptr*,FXptr,FXptr): not implemented."
  if(*ptr==expect){ *ptr=v; return true; }
  return false;
#endif
  }


/// Atomically compare pair of variables at ptr against (cmpa,cmpb), setting them to (a,b) if equal and return true, or false otherwise
static inline FXbool atomicBoolDCas(volatile FXptr* ptr,FXptr cmpa,FXptr cmpb,FXptr a,FXptr b){
#if (defined(_WIN32) && (_MSC_VER >= 1500) && defined(_WIN64))
  FXlong duet[2]={(FXlong)a,(FXlong)b};
  return !!(_InterlockedCompareExchange128((volatile FXlong*)ptr,(FXlong)cmpb,(FXlong)cmpa,duet));
#elif (defined(_WIN32) && (_MSC_VER >= 1500))
  __int64 ab=(((__int64)(FXuval)a)|((__int64)(FXuval)b)<<32);
  __int64 compab=(((__int64)(FXuval)cmpa)|((__int64)(FXuval)cmpb)<<32);
  return (InterlockedCompareExchange64((__int64 volatile *)ptr,ab,compab)==compab);
//#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && defined(ILP32)
//  FXulong ex=(((FXulong)(FXuval)cmpa)|((FXulong)(FXuval)cmpb)<<32);
//  FXulong v=(((FXulong)(FXuval)a)|((FXulong)(FXuval)b)<<32);
//  return __atomic_compare_exchange_n((volatile TLong*)ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
//#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (defined(LLP64) || defined(LP64))
//  __int128 ex=(((__int128)(FXuval)cmpa)|((__int128)(FXuval)cmpb)<<32);
//  __int128 v=(((__int128)(FXuval)a)|((__int128)(FXuval)b)<<32);
//  return __atomic_compare_exchange_n((volatile __int128*)ptr,&ex,v,false,__ATOMIC_SEQ_CST,__ATOMIC_RELAXED);
//#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
//  __int128 ex=(((__int128)(FXuval)cmpa)|((__int128)(FXuval)cmpb)<<32);
//  __int128 v=(((__int128)(FXuval)a)|((__int128)(FXuval)b)<<32);
//  return __sync_bool_compare_and_swap((volatile __int128*)ptr,ex,v);
#elif (defined(__GNUC__) && defined(__i386__) && (defined(__PIC__) || defined(__PIE__)))
  FXptr ret;
  __asm__ __volatile__ ("xchgl %%esi, %%ebx\n\t"
                        "lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "setz %%al\n\t"
                        "andl $1, %%eax\n\t"
                        "xchgl %%esi, %%ebx\n\t" : "=a"(ret) : "D"(ptr), "a"(cmpa), "d"(cmpb), "S"(a), "c"(b) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__i386__))
  FXptr ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchg8b (%1)\n\t"
                        "setz %%al\n\t"
                        "andl $1, %%eax\n\t" : "=a"(ret) : "D"(ptr), "a"(cmpa), "d"(cmpb), "b"(a), "c"(b) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXptr ret;
  __asm__ __volatile__ ("lock\n\t"
                        "cmpxchg16b (%1)\n\t"
                        "setz %%al\n\t"
                        "andq $1, %%rax\n\t" : "=a"(ret) : "r"(ptr), "a"(cmpa), "d"(cmpb), "b"(a), "c"(b) : "memory", "cc");
  return ret;
#else
#warning "atomicBoolDCas(volatile FXptr*,FXptr,FXptr,FXptr,FXptr): not implemented."
  if(ptr[0]==cmpa && ptr[1]==cmpb){ ptr[0]=a; ptr[1]=b; return true; }
  return false;
#endif
  }


/// Atomically add v to pointer variable at ptr, and return its old contents
static inline FXptr atomicAdd(volatile FXptr* ptr,FXival v){
#if defined(_WIN32) && (_MSC_VER >= 1800) && defined(_WIN64)
  return (FXptr)_InterlockedExchangeAdd64((volatile FXlong*)ptr,(FXlong)v);
#elif defined(_WIN32) && (_MSC_VER >= 1800)
  return (FXptr)InterlockedExchangeAdd((volatile long*)ptr,(long)v);
#elif defined(__GNUC__) && defined(__ATOMIC_SEQ_CST) && (__GCC_ATOMIC_POINTER_LOCK_FREE == 2)
  return __atomic_fetch_add(ptr,v,__ATOMIC_SEQ_CST);
#elif ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 1)))
  return __sync_fetch_and_add(ptr,v);
#elif (defined(__GNUC__) && defined(__i386__))
  FXptr ret=(TPtr)v;
  __asm__ __volatile__ ("lock\n\t"
                        "xaddl %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0"(ret) : "memory", "cc");
  return ret;
#elif (defined(__GNUC__) && defined(__x86_64__))
  FXptr ret=(TPtr)v;
  __asm__ __volatile__ ("lock\n\t"
                        "xaddq %0,(%1)\n\t" : "=r"(ret) : "r"(ptr), "0" (ret) : "memory", "cc");
  return ret;
#else
#warning "atomicAdd(volatile FXptr*,FXival): not implemented."
  FXptr ret=*ptr; *ptr+=v;
  return ret;
#endif
  }


///// Atomic pointers to something else


/// Atomically set pointer variable at ptr to v, and return its old contents
template <typename EType>
static inline EType* atomicSet(EType *volatile *ptr,EType* v){
  return (EType*)atomicSet((volatile FXptr*)ptr,(FXptr)v);
  }


/// Atomically add v to pointer variable at ptr, and return its old contents
template <typename EType>
static inline EType* atomicAdd(EType *volatile *ptr,FXival v){
  return (EType*)atomicAdd((volatile FXptr*)ptr,v*((FXival)sizeof(EType)));
  }


/// Atomically compare pointer variable at ptr against expect, setting it to v if equal; returns the old value at ptr
template <typename EType>
static inline EType* atomicCas(EType *volatile *ptr,EType* expect,EType* v){
  return (EType*)atomicCas((volatile FXptr*)ptr,(FXptr)expect,(FXptr)v);
  }


/// Atomically compare pointer variable at ptr against expect, setting it to v if equal and return true, or false otherwise
template <typename EType>
static inline FXbool atomicBoolCas(EType *volatile *ptr,EType* expect,EType* v){
  return atomicBoolCas((volatile FXptr*)ptr,(FXptr)expect,(FXptr)v);
  }


/// Atomically compare pair of variables at ptr against (cmpa,cmpb), setting them to (a,b) if equal and return true, or false otherwise
template <typename EType>
static inline FXbool atomicBoolDCas(volatile EType* ptr,EType cmpa,EType cmpb,EType a,EType b){
  return atomicBoolDCas((volatile FXptr*)ptr,(FXptr)cmpa,(FXptr)cmpb,(FXptr)a,(FXptr)b);
  }

}

#endif
