#include "trpc/coroutine/coroutine.h"
#include "coroutine.h"
#include <atomic>

namespace trpc {
  // main coroutine, every io thread have a main coroutine
  static thread_local Coroutine *t_main_coroutine = NULL; 

  static thread_local Coroutine *t_cur_coroutine = NULL;

  static thread_local bool t_enable_coroutine_swap = true;

  static std::atomic_int t_coroutine_count {0};

  static std::atomic_int t_cur_coroutine_id {1};

  int getCoroutineIndex() {
    return t_cur_coroutine_id;
  }
  
  void CoFunction(Coroutine *co) {

    if (co != nullptr) {
      co->setIsInCoFunc(true);

      co->m_call_back();

      co->setIsInCoFunc(false);
    }

    Coroutine::Yield();
  }
}