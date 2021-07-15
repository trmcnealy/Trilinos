// Copyright (c) 2013, Sandia Corporation.
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#ifndef STK_UTIL_DIAG_SignalHandler_hpp
#define STK_UTIL_DIAG_SignalHandler_hpp

#include <map>                          // for multimap, etc
#include <stk_util/util/Callback.hpp>   // for CallbackBase
#include <string>                       // for string

#define _SIGSET_NWORDS (1024 / (8 * sizeof(unsigned long int)))
typedef struct
{
    unsigned long int __val[_SIGSET_NWORDS];
} __sigset_t;

typedef __sigset_t sigset_t;

typedef void (*__sighandler_t)(int);

union sigval {
    int   sival_int;
    void* sival_ptr;
};

typedef union sigval __sigval_t;

typedef int          __pid_t;
typedef unsigned int __uid_t;

typedef struct
{
    int si_signo;         /* Signal number.  */
    int si_errno;         /* If non-zero, an errno value associated with
                     this signal, as defined in <errno.h>.  */
    int        si_code;   /* Signal code.  */
    __pid_t    si_pid;    /* Sending process ID.  */
    __uid_t    si_uid;    /* Real user ID of sending process.  */
    void*      si_addr;   /* Address of faulting instruction.  */
    int        si_status; /* Exit value or signal.  */
    long int   si_band;   /* Band event for SIGPOLL.  */
    __sigval_t si_value;  /* Signal value.  */
} siginfo_t;

struct sigaction
{
#if 1
    union {
        /* Used if SA_SIGINFO is not set.  */
        __sighandler_t sa_handler;
        /* Used if SA_SIGINFO is set.  */
        void (*sa_sigaction)(int, siginfo_t*, void*);
    } __sigaction_handler;
#    define sa_handler __sigaction_handler.sa_handler
#    define sa_sigaction __sigaction_handler.sa_sigaction
#else
    __sighandler_t sa_handler;
#endif

    __sigset_t sa_mask;

    int sa_flags;
};

/* Bits in `sa_flags'.  */
#define SA_NOCLDSTOP 0x00000004 /* Don't send SIGCHLD when children stop.  */
#define SA_NOCLDWAIT 0x00000020 /* Don't create zombie on child death.  */
#define SA_SIGINFO                                     \
    0x00000040 /* Invoke signal-catching function with \
  three arguments instead of one.  */
#if defined __USE_XOPEN_EXTENDED || defined __USE_MISC
#    define SA_ONSTACK 0x00000001 /* Use signal stack by using `sa_restorer'. */
#endif
#if defined __USE_XOPEN_EXTENDED || defined __USE_XOPEN2K8
#    define SA_RESTART 0x00000002 /* Restart syscall on signal return.  */
#    define SA_NODEFER                                                      \
        0x00000008                  /* Don't automatically block the signal \
                       when its handler is being executed.  */
#    define SA_RESETHAND 0x00000010 /* Reset to SIG_DFL on entry to handler.  */
#endif
#ifdef __USE_MISC
#    define SA_INTERRUPT 0x20000000 /* Historical no-op.  */

/* Some aliases for the SA_ constants.  */
#    define SA_NOMASK SA_NODEFER
#    define SA_ONESHOT SA_RESETHAND
#    define SA_STACK SA_ONSTACK
#endif

/* Values for the HOW argument to `sigprocmask'.  */
#define SIG_BLOCK 1   /* Block signals.  */
#define SIG_UNBLOCK 2 /* Unblock signals.  */
#define SIG_SETMASK 3 /* Set the set of blocked signals.  */

/**
 * @file
 *
 * The signal handler
 */

namespace sierra {

/**
 * @brief Class <b>SignalHandler</b> ...
 *
 */
class SignalHandler
{
public:
  /**
   * @brief Member function <b>instance</b> ...
   *
   * @return a <b>Handler</b> ...
   */
  static SignalHandler &instance();

  static bool check_signal_name(const std::string &signal);

  /**
   * @brief Member function <b>handle_signal</b> ...
   *
   * @param signal	an <b>int</b> variable ...
   */
  void handle_signal(int signal);

  /**
   * @brief Member function <b>add_handler</b> ...
   *
   * @param signal	an <b>int</b> variable ...
   * @param callback	a <b>CallbackBase</b> variable ...
   */
  void add_handler(int signal, CallbackBase &callback);

  /**
   * @brief Member function <b>add_handler</b> ...
   *
   * @param signal_name	a <b>String</b> variable ...
   * @param callback	a <b>CallbackBase</b> variable ...
   */
  void add_handler(const std::string &signal_name, CallbackBase &callback);

  /**
   * @brief Member function <b>remove_handler</b> ...
   *
   * @param signal	an <b>int</b> variable ...
   * @param callback	a <b>CallbackBase</b> variable ...
   */
  void remove_handler(int signal, CallbackBase &callback);

  /**
   * @brief Member function <b>remove_handler</b> ...
   *
   * @param signal_name	a <b>String</b> variable ...
   * @param callback	a <b>CallbackBase</b> variable ...
   */
  void remove_handler(const std::string &signal_name, CallbackBase &callback);

  /**
   * @brief Member function <b>remove_all_handlers</b> ...
   *
   */
  void remove_all_handlers();

private:
  typedef std::multimap<int, CallbackBase *> HandlerMap;
  typedef std::multimap<int, struct sigaction *> OldActionMap;

  HandlerMap		m_handlerMap;
  OldActionMap		m_oldActionMap;
};

} // namespace sierra

#endif // STK_UTIL_DIAG_SignalHandler_h
