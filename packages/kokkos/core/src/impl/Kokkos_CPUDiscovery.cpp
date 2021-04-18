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

#if (defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_WINDOWS))
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#error "_WINDOWS" 
#include <unistd.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>
#include <iostream>

namespace Kokkos {
namespace Impl {

static unsigned long CountSetBits(const unsigned long long bitMask) {
  const unsigned long LSHIFT = sizeof(unsigned long long) * 8 - 1;
  unsigned long long bitTest = static_cast<unsigned long long>(1) << LSHIFT;

  unsigned long bitSetCount = 0;

  for (unsigned long i = 0; i <= LSHIFT; ++i) {
    bitSetCount += ((bitMask & bitTest) ? 1 : 0);
    bitTest /= 2;
  }

  return bitSetCount;
}

int processors_per_node() {
#ifdef _SC_NPROCESSORS_ONLN
  int const num_procs     = sysconf(_SC_NPROCESSORS_ONLN);
  int const num_procs_max = sysconf(_SC_NPROCESSORS_CONF);
  if ((num_procs < 1) || (num_procs_max < 1)) {
    return -1;
  }
  return num_procs;
#elif defined(__APPLE__)
  int ncpu;
  int activecpu;
  size_t size = sizeof(int);
  sysctlbyname("hw.ncpu", &ncpu, &size, nullptr, 0);
  sysctlbyname("hw.activecpu", &activecpu, &size, nullptr, 0);
  if (ncpu < 1 || activecpu < 1)
    return -1;
  else
    return activecpu;
// #elif defined(_WINDOWS)

//   const unsigned nSLPI = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

//   BOOL done = FALSE;
//   DWORD success;
//   PCACHE_DESCRIPTOR Cache;

//   PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr;
//   PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr    = nullptr;

//   DWORD returnLength          = 0;
//   DWORD logicalProcessorCount = 0;
//   DWORD numaNodeCount         = 0;
//   DWORD processorCoreCount    = 0;
//   DWORD processorL1CacheCount = 0;
//   DWORD processorL2CacheCount = 0;
//   DWORD processorL3CacheCount = 0;
//   DWORD processorPackageCount = 0;
//   DWORD byteOffset            = 0;

//   while (!done) {
//     success = GetLogicalProcessorInformation(buffer, &returnLength);

//     if (success == FALSE) {
//       if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
//         if (buffer) {
//           free(buffer);
//         }

//         buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);

//         if (buffer == nullptr) {
//           std::cerr << TEXT("\nError: Allocation failure\n");
//           return (2);
//         }
//       } else {
//         std::cerr << TEXT("\nError %d\n") << GetLastError();
//         return (3);
//       }
//     } else {
//       done = TRUE;
//     }
//   }

//   ptr = buffer;

//   while (byteOffset + nSLPI <= returnLength) {
//     switch (ptr->Relationship) {
//       case RelationNumaNode: {
//         // Non-NUMA systems report a single record of this type.
//         numaNodeCount++;
//         break;
//       }
//       case RelationProcessorCore: {
//         processorCoreCount++;

//         // A hyperthreaded core supplies more than one logical processor.
//         logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
//         break;
//       }
//       case RelationProcessorPackage: {
//         // Logical processors share a physical package.
//         processorPackageCount++;
//         break;
//       }
//       default: {
//         std::cerr << TEXT(
//             "\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n");
//         break;
//       }
//     }

//     byteOffset += nSLPI;
//     ptr++;
//   }

//   const DWORD available_numa_count       = numaNodeCount;
//   const DWORD available_cores_per_numa   = processorCoreCount;
//   const DWORD available_threads_per_core = logicalProcessorCount / processorCoreCount;

//   return available_numa_count * available_cores_per_numa * available_threads_per_core;
#else
  return -1;
#endif
}

int mpi_ranks_per_node() {
  char *str;
  int ppn = 1;
  // if ((str = getenv("SLURM_TASKS_PER_NODE"))) {
  //  ppn = std::stoi(str);
  //  if(ppn<=0) ppn = 1;
  //}
  if ((str = getenv("MV2_COMM_WORLD_LOCAL_SIZE"))) {
    ppn = std::stoi(str);
    if (ppn <= 0) ppn = 1;
  }
  if ((str = getenv("OMPI_COMM_WORLD_LOCAL_SIZE"))) {
    ppn = std::stoi(str);
    if (ppn <= 0) ppn = 1;
  }
  return ppn;
}

int mpi_local_rank_on_node() {
  char *str;
  int local_rank = 0;
  // if ((str = getenv("SLURM_LOCALID"))) {
  //  local_rank = std::stoi(str);
  //}
  if ((str = getenv("MV2_COMM_WORLD_LOCAL_RANK"))) {
    local_rank = std::stoi(str);
  }
  if ((str = getenv("OMPI_COMM_WORLD_LOCAL_RANK"))) {
    local_rank = std::stoi(str);
  }
  return local_rank;
}

}  // namespace Impl
}  // namespace Kokkos
