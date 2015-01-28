/*
//@HEADER
// ************************************************************************
//
//   Kokkos: Manycore Performance-Portable Multidimensional Arrays
//              Copyright (2012) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
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
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact  H. Carter Edwards (hcedwar@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#include <gtest/gtest.h>

#include <iostream>
#include <Kokkos_Core.hpp>

/*--------------------------------------------------------------------------*/

namespace {

template<class ExecutionSpace>
struct TestMemory {
  int* data;
  int offset;

  typedef int value_type;
  typedef ExecutionSpace execution_space;

  TestMemory(int* data_, int offset_):data(data_),offset(offset_){}

  KOKKOS_INLINE_FUNCTION
  void operator() (const int& i) const {
    data[i+offset] = i+offset;
  }

  KOKKOS_INLINE_FUNCTION
  void operator() (const int& i, int& val) const {
    val += data[i];
  }

};

template<class Arg1>
class TestMemorySpace {
public:

  typedef typename Arg1::memory_space MemorySpace;
  TestMemorySpace() { run_test(); }

  void run_test()
  {
    int count;

    Kokkos::View<int* ,Arg1, Kokkos::MemoryUnmanaged> invalid;
    count  = MemorySpace::count(invalid.ptr_on_device());
    ASSERT_TRUE(count==0);

    {
      Kokkos::View<int* ,Arg1> a("A",10);

      count  = MemorySpace::count(a.ptr_on_device());
      ASSERT_TRUE(count==1);

      count  = MemorySpace::count(a.ptr_on_device()+5);
      ASSERT_TRUE(count==1);

      {
        Kokkos::View<int* ,Arg1> b = a;
        count  = MemorySpace::count(b.ptr_on_device());
        ASSERT_TRUE(count==2);

        Kokkos::View<int* ,Arg1> D("D",10);
        count  = MemorySpace::count(D.ptr_on_device());
        ASSERT_TRUE(count==1);

        {
          Kokkos::View<int* ,Arg1> E("E",10);
          count  = MemorySpace::count(E.ptr_on_device());
          ASSERT_TRUE(count==1);

          Kokkos::View<int* ,Arg1, Kokkos::MemoryUnmanaged> c = a;
          count  = MemorySpace::count(c.ptr_on_device());
          ASSERT_TRUE(count==2);
        }

        count  = MemorySpace::count(b.ptr_on_device());
        ASSERT_TRUE(count==2);
      }

      count  = MemorySpace::count(a.ptr_on_device()+5);
      ASSERT_TRUE(count==1);

      invalid = a;

      count  = MemorySpace::count(invalid.ptr_on_device()+5);
      ASSERT_TRUE(count==1);
    }

    count  = MemorySpace::count(invalid.ptr_on_device());
    ASSERT_TRUE(count==0);

#ifdef KOKKOS_HAVE_CXX11
    int* data = (int*) Kokkos::kokkos_malloc<MemorySpace>(100*sizeof(int));
    int sum = 0;

    Kokkos::parallel_for(100, TestMemory<typename Arg1::execution_space>(data,0));
    Kokkos::parallel_reduce(100, TestMemory<typename Arg1::execution_space>(data,0),sum);
    ASSERT_TRUE(sum == 100*99/2);

    sum = 0;
    ASSERT_NO_THROW(data = (int*) Kokkos::kokkos_realloc<MemorySpace>(data,120*sizeof(int)));

    Kokkos::parallel_for(20, TestMemory<typename Arg1::execution_space>(data,100));
    Kokkos::parallel_reduce(120, TestMemory<typename Arg1::execution_space>(data,0),sum);

    ASSERT_TRUE(sum == 120*119/2);

    int* data2 = data + 50;
    ASSERT_ANY_THROW(data = (int*) Kokkos::kokkos_realloc<MemorySpace>(data2,140*sizeof(int)));

    Kokkos::kokkos_free<MemorySpace>(data);
#endif

  }
};

}

/*--------------------------------------------------------------------------*/



