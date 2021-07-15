/*
//@HEADER
// ************************************************************************
//
//                        Kokkos v. 3.0
//       Copyright (2020) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY NTESS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NTESS OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Christian R. Trott (crtrott@sandia.gov)
//
// ************************************************************************
//@HEADER
*/
#pragma once

#ifndef KOKKOS_ATOMIC_WINDOWS_HPP
#define KOKKOS_ATOMIC_WINDOWS_HPP

#if (defined(_MSC_VER) || defined(WIN32) || defined(_WIN32) || \
     defined(WIN64) || defined(_WIN64) || defined(_WINDOWS))

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <type_traits>
#include <intrin.h>

//#ifdef __MINGW64__
//#if !__has_builtin(_InterlockedCompareExchange128)
__MINGW_INTRIN_INLINE unsigned char InterlockedCompareExchange128(
    __int64 volatile *_Destination, __int64 _ExchangeHigh, __int64 _ExchangeLow,
    __int64 *_ComparandResult) noexcept {
  bool swapped = false;

  __asm__ __volatile__(
      "lock cmpxchg16b %1\n\t"
      "setz %0"
      : "=q"(swapped), "+m"(*_Destination), "+d"(_ComparandResult[1]),
        "+a"(_ComparandResult[0])
      : "c"(_ExchangeHigh), "b"(_ExchangeLow), "q"(swapped));

  return (swapped ? 1 : 0);
}

//mov rax,r8
//lock cmpxchg qword ptr [rcx],rdx





#if defined(_InterlockedCompareExchange128)
#undef _InterlockedCompareExchange128 InterlockedCompareExchange128
#endif

//#endif
//#endif

//#if !defined(InterlockedCompareExchange128)
//#define InterlockedCompareExchange128 _InterlockedCompareExchange128
//#endif

#if !defined(InterlockedAnd64)
#define InterlockedAnd64 _InterlockedAnd64
#endif

#if !defined(InterlockedDecrement64)
#define InterlockedDecrement64 _InterlockedDecrement64
#endif

#if !defined(InterlockedExchange64)
#define InterlockedExchange64 _InterlockedExchange64
#endif

#if !defined(InterlockedExchangeAdd64)
#define InterlockedExchangeAdd64 _InterlockedExchangeAdd64
#endif

#if !defined(InterlockedExchangeSub64)
#define InterlockedExchangeSub64 _InterlockedExchangeSub64
#endif

#if !defined(InterlockedIncrement64)
#define InterlockedIncrement64 _InterlockedIncrement64
#endif

#if !defined(InterlockedOr64)
#define InterlockedOr64 _InterlockedOr64
#endif

#if !defined(InterlockedXor64)
#define InterlockedXor64 _InterlockedXor64
#endif

#if !defined(InterlockedCompareExchangePointer)
#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer
#endif

#if !defined(InterlockedExchangePointer)
#define InterlockedExchangePointer _InterlockedExchangePointer
#endif

#if !defined(InterlockedAnd8)
#define InterlockedAnd8 _InterlockedAnd8
#endif

#if !defined(InterlockedAnd16)
#define InterlockedAnd16 _InterlockedAnd16
#endif

#if !defined(InterlockedAnd)
#define InterlockedAnd _InterlockedAnd
#endif

#if !defined(InterlockedCompareExchange8)
#define InterlockedCompareExchange8 _InterlockedCompareExchange8
#endif

#if !defined(InterlockedCompareExchange16)
#define InterlockedCompareExchange16 _InterlockedCompareExchange16
#endif

#if !defined(InterlockedCompareExchange)
#define InterlockedCompareExchange _InterlockedCompareExchange
#endif

#if !defined(InterlockedCompareExchange64)
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#endif

#if !defined(InterlockedCompareExchangePointer64)
#define InterlockedCompareExchangePointer64 _InterlockedCompareExchangePointer64
#endif

#if !defined(InterlockedCompareExchangePointer_nf)
#define InterlockedCompareExchangePointer_nf \
  _InterlockedCompareExchangePointer_nf
#endif

#if !defined(InterlockedDecrement16)
#define InterlockedDecrement16 _InterlockedDecrement16
#endif

#if !defined(InterlockedDecrement64)
#define InterlockedDecrement64 _InterlockedDecrement64
#endif

#if !defined(InterlockedDecrement)
#define InterlockedDecrement _InterlockedDecrement
#endif

#if !defined(InterlockedExchange64)
#define InterlockedExchange64 _InterlockedExchange64
#endif

#if !defined(InterlockedExchange)
#define InterlockedExchange _InterlockedExchange
#endif

#if !defined(InterlockedExchange8)
#define InterlockedExchange8 _InterlockedExchange8
#endif

#if !defined(InterlockedExchange16)
#define InterlockedExchange16 _InterlockedExchange16
#endif

#if !defined(InterlockedExchangeAdd8)
#define InterlockedExchangeAdd8 _InterlockedExchangeAdd8
#endif

#if !defined(InterlockedExchangeAdd16)
#define InterlockedExchangeAdd16 _InterlockedExchangeAdd16
#endif

#if !defined(InterlockedExchangeAdd)
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#endif

#if !defined(InterlockedExchangeAdd64)
#define InterlockedExchangeAdd64 _InterlockedExchangeAdd64
#endif

#if !defined(InterlockedExchangePointer64)
#define InterlockedExchangePointer64 _InterlockedExchangePointer64
#endif

#if !defined(InterlockedExchangeSub8)
#define InterlockedExchangeSub8 _InterlockedExchangeSub8
#endif

#if !defined(InterlockedExchangeSub16)
#define InterlockedExchangeSub16 _InterlockedExchangeSub16
#endif

#if !defined(InterlockedExchangeSub)
#define InterlockedExchangeSub _InterlockedExchangeSub
#endif

#if !defined(InterlockedIncrement16)
#define InterlockedIncrement16 _InterlockedIncrement16
#endif

#if !defined(InterlockedIncrement)
#define InterlockedIncrement _InterlockedIncrement
#endif

#if !defined(InterlockedOr8)
#define InterlockedOr8 _InterlockedOr8
#endif

#if !defined(InterlockedOr16)
#define InterlockedOr16 _InterlockedOr16
#endif

#if !defined(InterlockedOr)
#define InterlockedOr _InterlockedOr
#endif

#if !defined(InterlockedXor8)
#define InterlockedXor8 _InterlockedXor8
#endif

#if !defined(InterlockedXor16)
#define InterlockedXor16 _InterlockedXor16
#endif

#if !defined(InterlockedXor)
#define InterlockedXor _InterlockedXor
#endif

//#pragma intrinsic(_InterlockedCompareExchange128)

#if !defined(KOKKOS_ATOMIC_ASSEMBLY_HPP)
#include "Kokkos_Atomic_Assembly.hpp"
#endif

namespace Kokkos {

namespace Windows {

#if defined(MemoryBarrier)
#undef MemoryBarrier
#endif

#if defined(WIN64) || defined(_WIN64) || defined(_AMD64) || \
    defined(_AMD64_) || defined(__x86_64) || defined(__x86_64__)
#define MemoryBarrier __faststorefence
#elif defined(_IA64_)
#define MemoryBarrier __mf
#else
__forceinline void MemoryBarrier() {
  long Barrier;
  __asm {
				xchg Barrier, eax
  }
}
#endif

#pragma region SIGNED_INTERLOCKED

#define SIGNED_INTERLOCKED(TYPE, INTERLOCKED_SUF)                              \
  __inline TYPE Exchange(TYPE volatile *const dest, const TYPE value) {        \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedExchange##INTERLOCKED_SUF(dest, value));   \
                                                                               \
    return oldValue;                                                           \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  __inline T Exchange(                                                         \
      T volatile *const dest,                                                  \
      typename std::enable_if<sizeof(T) == sizeof(TYPE) &&                     \
                                  !std::is_same<T, TYPE>::value,               \
                              const T &>::type value) {                        \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedExchange##INTERLOCKED_SUF(                 \
                             (TYPE volatile *)dest, *(TYPE *)&value));         \
                                                                               \
    return *(T *)&oldValue;                                                    \
  }                                                                            \
                                                                               \
  __inline TYPE CompareExchange(TYPE volatile *const dest,                     \
                                const TYPE &compare, const TYPE &value) {      \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedCompareExchange##INTERLOCKED_SUF(          \
                             dest, value, compare));                           \
                                                                               \
    return oldValue;                                                           \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  __inline T CompareExchange(                                                  \
      T volatile *const dest, const T &compare,                                \
      typename std::enable_if<sizeof(T) == sizeof(TYPE) &&                     \
                                  !std::is_same<T, TYPE>::value,               \
                              const T &>::type value) {                        \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue !=                                                       \
             InterlockedCompareExchange##INTERLOCKED_SUF(                      \
                 (TYPE volatile *)dest, *(TYPE *)&value, *(TYPE *)&compare));  \
                                                                               \
    return *(T *)&oldValue;                                                    \
  }                                                                            \
                                                                               \
  __inline TYPE ExchangeAdd(TYPE volatile *const dest, const TYPE &value) {    \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue !=                                                       \
             InterlockedExchangeAdd##INTERLOCKED_SUF(dest, value));            \
                                                                               \
    return oldValue;                                                           \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  __inline T ExchangeAdd(                                                      \
      T volatile *const dest,                                                  \
      typename std::enable_if<sizeof(T) == sizeof(TYPE) &&                     \
                                  !std::is_same<T, TYPE>::value,               \
                              const T &>::type value) {                        \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedExchangeAdd##INTERLOCKED_SUF(              \
                             (TYPE volatile *)dest, *(TYPE *)&value));         \
                                                                               \
    return *(T *)&oldValue;                                                    \
  }                                                                            \
                                                                               \
  __inline TYPE And(TYPE volatile *const dest, const TYPE &value) {            \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedAnd##INTERLOCKED_SUF(dest, value));        \
                                                                               \
    return oldValue;                                                           \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  __inline T And(T volatile *const dest,                                       \
                 typename std::enable_if<sizeof(T) == sizeof(TYPE) &&          \
                                             !std::is_same<T, TYPE>::value,    \
                                         const T &>::type value) {             \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedAnd##INTERLOCKED_SUF(                      \
                             (TYPE volatile *)dest, *(TYPE *)&value));         \
                                                                               \
    return *(T *)&oldValue;                                                    \
  }                                                                            \
                                                                               \
  __inline TYPE Or(TYPE volatile *const dest, const TYPE &value) {             \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedOr##INTERLOCKED_SUF(dest, value));         \
                                                                               \
    return oldValue;                                                           \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  __inline T Or(T volatile *const dest,                                        \
                typename std::enable_if<sizeof(T) == sizeof(TYPE) &&           \
                                            !std::is_same<T, TYPE>::value,     \
                                        const T &>::type value) {              \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedOr##INTERLOCKED_SUF((TYPE volatile *)dest, \
                                                        *(TYPE *)&value));     \
                                                                               \
    return *(T *)&oldValue;                                                    \
  }                                                                            \
                                                                               \
  __inline TYPE Xor(TYPE volatile *const dest, const TYPE &value) {            \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedXor##INTERLOCKED_SUF(dest, value));        \
                                                                               \
    return oldValue;                                                           \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  __inline T Xor(T volatile *const dest, const TYPE &value) {                  \
    const TYPE oldValue = *dest;                                               \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedXor##INTERLOCKED_SUF(                      \
                             (TYPE volatile *)dest, *(TYPE *)&value));         \
                                                                               \
    return *(T *)&oldValue;                                                    \
  }                                                                            \
                                                                               \
  __inline TYPE Add(TYPE volatile *const dest, const TYPE &value) {            \
    const TYPE oldValue = *dest;                                               \
    const TYPE calcVal  = (oldValue + value);                                  \
    const TYPE newVal   = calcVal;                                             \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedCompareExchange##INTERLOCKED_SUF(          \
                             dest, newVal, oldValue));                         \
                                                                               \
    return oldValue;                                                           \
  }                                                                            \
                                                                               \
  template <typename T>                                                        \
  __inline T Add(T volatile *const dest,                                       \
                 typename std::enable_if<sizeof(T) == sizeof(TYPE) &&          \
                                             !std::is_same<T, TYPE>::value,    \
                                         const T &>::type value) {             \
    const TYPE oldValue = *dest;                                               \
    const T calcVal     = (oldValue + value);                                  \
    const TYPE newVal   = *(TYPE *)&calcVal;                                   \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedCompareExchange##INTERLOCKED_SUF(          \
                             (TYPE volatile *)dest, newVal, oldValue));        \
                                                                               \
    return *(T *)&oldValue;                                                    \
  }                                                                            \
                                                                               \
  __inline TYPE Sub(TYPE volatile *const dest, const TYPE &value) {            \
    const TYPE oldValue = *dest;                                               \
    const TYPE calcVal  = (oldValue - value);                                  \
    const TYPE newVal   = calcVal;                                             \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedCompareExchange##INTERLOCKED_SUF(          \
                             dest, newVal, oldValue));                         \
                                                                               \
    return oldValue;                                                           \
  }                                                                            \
  template <typename T>                                                        \
  __inline T Sub(T volatile *const dest,                                       \
                 typename std::enable_if<sizeof(T) == sizeof(TYPE) &&          \
                                             !std::is_same<T, TYPE>::value,    \
                                         const T &>::type value) {             \
    const TYPE oldValue = *dest;                                               \
    const T calcVal     = (oldValue - value);                                  \
    const TYPE newVal   = *(TYPE *)&calcVal;                                   \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldValue != InterlockedCompareExchange##INTERLOCKED_SUF(          \
                             (TYPE volatile *)dest, newVal, oldValue));        \
                                                                               \
    return *(T *)&oldValue;                                                    \
  }
#pragma endregion

template <typename T>
__inline auto CompareExchange(T volatile *const dest, const T &compare,
                              const T &value) ->
    typename std::enable_if<std::is_pointer<T>::value, T>::type

{
  return (T)InterlockedCompareExchangePointer((void *volatile *)dest,
                                              (void *)value, (void *)compare);
}

template <typename T>
__inline auto Exchange(T volatile *const dest, const T &value) ->
    typename std::enable_if<std::is_pointer<T>::value, T>::type

{
  return (T)InterlockedExchangePointer((void *volatile *)dest, (void *)value);
}

SIGNED_INTERLOCKED(char, 8)

SIGNED_INTERLOCKED(short, 16)

SIGNED_INTERLOCKED(long, )

SIGNED_INTERLOCKED(long long, 64)

#undef SIGNED_INTERLOCKED

#define UNSIGNED_INTERLOCKED(TYPE, SIGNED_TYPE, INTERLOCKED_SUF)               \
  __inline TYPE Exchange(TYPE volatile *const dest, const TYPE value) {        \
    union V {                                                                  \
      SIGNED_TYPE i;                                                           \
      TYPE t;                                                                  \
      __inline V() {}                                                          \
    } newval, oldval;                                                          \
                                                                               \
    newval.t = value;                                                          \
                                                                               \
    union Vol {                                                                \
      SIGNED_TYPE volatile *i;                                                 \
      TYPE volatile *t;                                                        \
      __inline Vol() {}                                                        \
    } newdest;                                                                 \
                                                                               \
    oldval.t  = *dest;                                                         \
    newdest.t = dest;                                                          \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldval.i !=                                                       \
             InterlockedExchange##INTERLOCKED_SUF(newdest.i, newval.i));       \
    return oldval.t;                                                           \
  }                                                                            \
                                                                               \
  __inline TYPE CompareExchange(TYPE volatile *const dest,                     \
                                const TYPE &compare, const TYPE &value) {      \
    union V {                                                                  \
      SIGNED_TYPE i;                                                           \
      TYPE t;                                                                  \
      __inline V() {}                                                          \
    } newval, oldval, assume;                                                  \
                                                                               \
    newval.t = value;                                                          \
    assume.t = compare;                                                        \
                                                                               \
    union Vol {                                                                \
      SIGNED_TYPE volatile *i;                                                 \
      TYPE volatile *t;                                                        \
      __inline Vol() {}                                                        \
    } newdest;                                                                 \
                                                                               \
    oldval.t  = *dest;                                                         \
    newdest.t = dest;                                                          \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldval.i != InterlockedCompareExchange##INTERLOCKED_SUF(          \
                             newdest.i, newval.i, assume.i));                  \
    return oldval.t;                                                           \
  }                                                                            \
                                                                               \
  __inline TYPE ExchangeAdd(TYPE volatile *const dest, const TYPE &value) {    \
    union V {                                                                  \
      SIGNED_TYPE i;                                                           \
      TYPE t;                                                                  \
      __inline V() {}                                                          \
    } newval, oldval;                                                          \
                                                                               \
    newval.t = value;                                                          \
                                                                               \
    union Vol {                                                                \
      SIGNED_TYPE volatile *i;                                                 \
      TYPE volatile *t;                                                        \
      __inline Vol() {}                                                        \
    } newdest;                                                                 \
                                                                               \
    oldval.t  = *dest;                                                         \
    newdest.t = dest;                                                          \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldval.i !=                                                       \
             InterlockedExchangeAdd##INTERLOCKED_SUF(newdest.i, newval.i));    \
                                                                               \
    return oldval.t;                                                           \
  }                                                                            \
                                                                               \
  __inline TYPE And(TYPE volatile *const dest, const TYPE &value) {            \
    union V {                                                                  \
      SIGNED_TYPE i;                                                           \
      TYPE t;                                                                  \
      __inline V() {}                                                          \
    } newval, oldval;                                                          \
                                                                               \
    newval.t = value;                                                          \
                                                                               \
    union Vol {                                                                \
      SIGNED_TYPE volatile *i;                                                 \
      TYPE volatile *t;                                                        \
      __inline Vol() {}                                                        \
    } newdest;                                                                 \
                                                                               \
    oldval.t  = *dest;                                                         \
    newdest.t = dest;                                                          \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldval.i !=                                                       \
             InterlockedAnd##INTERLOCKED_SUF(newdest.i, newval.i));            \
                                                                               \
    return oldval.t;                                                           \
  }                                                                            \
                                                                               \
  __inline TYPE Or(TYPE volatile *const dest, const TYPE &value) {             \
    union V {                                                                  \
      SIGNED_TYPE i;                                                           \
      TYPE t;                                                                  \
      __inline V() {}                                                          \
    } newval, oldval;                                                          \
                                                                               \
    newval.t = value;                                                          \
                                                                               \
    union Vol {                                                                \
      SIGNED_TYPE volatile *i;                                                 \
      TYPE volatile *t;                                                        \
      __inline Vol() {}                                                        \
    } newdest;                                                                 \
                                                                               \
    oldval.t  = *dest;                                                         \
    newdest.t = dest;                                                          \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldval.i != InterlockedOr##INTERLOCKED_SUF(newdest.i, newval.i)); \
                                                                               \
    return oldval.t;                                                           \
  }                                                                            \
                                                                               \
  __inline TYPE Xor(TYPE volatile *const dest, const TYPE &value) {            \
    union V {                                                                  \
      SIGNED_TYPE i;                                                           \
      TYPE t;                                                                  \
      __inline V() {}                                                          \
    } newval, oldval;                                                          \
                                                                               \
    newval.t = value;                                                          \
                                                                               \
    union Vol {                                                                \
      SIGNED_TYPE volatile *i;                                                 \
      TYPE volatile *t;                                                        \
      __inline Vol() {}                                                        \
    } newdest;                                                                 \
                                                                               \
    oldval.t  = *dest;                                                         \
    newdest.t = dest;                                                          \
                                                                               \
    do {                                                                       \
      ;                                                                        \
    } while (oldval.i !=                                                       \
             InterlockedXor##INTERLOCKED_SUF(newdest.i, newval.i));            \
                                                                               \
    return oldval.t;                                                           \
  }                                                                            \
                                                                               \
  __inline TYPE Add(TYPE volatile *const dest, const TYPE &value) {            \
    union V {                                                                  \
      SIGNED_TYPE i;                                                           \
      TYPE t;                                                                  \
      __inline V() {}                                                          \
    } newval, oldval;                                                          \
                                                                               \
    union Vol {                                                                \
      SIGNED_TYPE volatile *i;                                                 \
      TYPE volatile *t;                                                        \
      __inline Vol() {}                                                        \
    } newdest;                                                                 \
                                                                               \
    oldval.t  = *dest;                                                         \
    newdest.t = dest;                                                          \
                                                                               \
    do {                                                                       \
      newval.t = oldval.t + value;                                             \
    } while (oldval.i != InterlockedCompareExchange##INTERLOCKED_SUF(          \
                             newdest.i, newval.i, oldval.i));                  \
    return oldval.t;                                                           \
  }                                                                            \
                                                                               \
  __inline TYPE Sub(TYPE volatile *const dest, const TYPE &value) {            \
    union V {                                                                  \
      SIGNED_TYPE i;                                                           \
      TYPE t;                                                                  \
      __inline V() {}                                                          \
    } newval, oldval;                                                          \
                                                                               \
    union Vol {                                                                \
      SIGNED_TYPE volatile *i;                                                 \
      TYPE volatile *t;                                                        \
      __inline Vol() {}                                                        \
    } newdest;                                                                 \
                                                                               \
    oldval.t  = *dest;                                                         \
    newdest.t = dest;                                                          \
                                                                               \
    do {                                                                       \
      newval.t = oldval.t - value;                                             \
    } while (oldval.i != InterlockedCompareExchange##INTERLOCKED_SUF(          \
                             newdest.i, newval.i, oldval.i));                  \
    return oldval.t;                                                           \
  }
#pragma endregion

UNSIGNED_INTERLOCKED(unsigned char, char, 8)

UNSIGNED_INTERLOCKED(unsigned short, short, 16)

UNSIGNED_INTERLOCKED(unsigned long, long, )

UNSIGNED_INTERLOCKED(unsigned long long, long long, 64)

#undef UNSIGNED_INTERLOCKED

__inline static float Add(float volatile *const dest, const float &value) {
  union V {
    long i;
    float t;
    __inline V() {}
  } newval, oldval;

  union Vol {
    long volatile *i;
    float volatile *t;
    __inline Vol() {}
  } newdest;

  oldval.t  = *dest;
  newdest.t = dest;

  do {
    newval.t = oldval.t + value;
  } while (oldval.i !=
           InterlockedCompareExchange(newdest.i, newval.i, oldval.i));
  return oldval.t;
}

__inline static double Add(double volatile *const dest, const double &value) {
  union V {
    long long i;
    double t;
    __inline V() {}
  } newval, oldval;

  union Vol {
    long long volatile *i;
    double volatile *t;
    __inline Vol() {}
  } newdest;

  oldval.t  = *dest;
  newdest.t = dest;

  do {
    newval.t = oldval.t + value;
  } while (oldval.i !=
           InterlockedCompareExchange64(newdest.i, newval.i, oldval.i));
  return oldval.t;
}

__inline static float Sub(float volatile *const dest, const float &value) {
  union V {
    long i;
    float t;
    __inline V() {}
  } newval, oldval;

  union Vol {
    long volatile *i;
    float volatile *t;
    __inline Vol() {}
  } newdest;

  oldval.t  = *dest;
  newdest.t = dest;

  do {
    newval.t = oldval.t - value;
  } while (oldval.i !=
           InterlockedCompareExchange(newdest.i, newval.i, oldval.i));
  return oldval.t;
}

__inline static double Sub(double volatile *const dest, const double &value) {
  union V {
    long long i;
    double t;
    __inline V() {}
  } newval, oldval;

  union Vol {
    long long volatile *i;
    double volatile *t;
    __inline Vol() {}
  } newdest;

  oldval.t  = *dest;
  newdest.t = dest;

  do {
    newval.t = oldval.t - value;
  } while (oldval.i !=
           InterlockedCompareExchange64(newdest.i, newval.i, oldval.i));
  return oldval.t;
}

#pragma region Interlocked 128

template <typename T>
__inline T CompareExchange(
    T volatile *const dest, const T &compare,
    typename std::enable_if<sizeof(T) == sizeof(Impl::cas128_t),
                            const T &>::type value) {
  union V {
    Impl::cas128_t i;
    T t;
    __inline V() {}
  } newval, oldval, assume;

  newval.t = value;
  assume.t = compare;

  union Vol {
    Impl::cas128_t volatile *i;
    long long volatile *l;
    T volatile *t;
    __inline Vol() {}
  } newdest;

  oldval.t  = *dest;
  newdest.t = dest;

  do {
    ;
  } while (InterlockedCompareExchange128(newdest.l, newval.i.upper,
                                         newval.i.lower,
                                         (long long *)&assume.i.lower) == 0);

  return oldval.t;
}

template <typename T>
__inline T Add(T volatile *const dest,
               typename std::enable_if<sizeof(T) == sizeof(Impl::cas128_t),
                                       const T &>::type value) {
  union V {
    Impl::cas128_t i;
    T t;
    __inline V() {}
  } newval, oldval;

  oldval.t = *dest;

  do {
    newval.t = oldval.t + value;
  } while (oldval.i != CompareExchange(dest, newval.i, oldval.i));

  return oldval.t;
}

template <typename T>
__inline T Sub(T volatile *const dest,
               typename std::enable_if<sizeof(T) == sizeof(Impl::cas128_t),
                                       const T &>::type value) {
  union V {
    Impl::cas128_t i;
    T t;
    __inline V() {}
  } newval, oldval;

  oldval.t = *dest;

  do {
    newval.t = oldval.t - value;
  } while (oldval.i != CompareExchange(dest, newval.i, oldval.i));

  return oldval.t;
}

// TODO
template <typename T>
__inline T Exchange(T volatile *const dest,
                    typename std::enable_if<sizeof(T) == sizeof(Impl::cas128_t),
                                            const T &>::type value) {
  while (!Impl::lock_address_host_space((void *)dest))
    ;
  MemoryBarrier();

  T return_val = *dest;
  *dest        = value;

  MemoryBarrier();
  Impl::unlock_address_host_space((void *)dest);

  return return_val;
}

#pragma endregion

}  // namespace Windows

}  // namespace Kokkos
#endif
#endif
