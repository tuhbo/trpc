#include "trpc/coroutine/coroutine.h"
#include "coroutine.h"
#include <atomic>
#include <cstdio>
#include <functional>
#include <assert.h>

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

void Coroutine::SetCoroutineSwapFlag(bool value) {
  t_enable_coroutine_swap = value;
}

bool Coroutine::GetCoroutineSwapFlag() {
  return t_enable_coroutine_swap;
}

Coroutine::Coroutine() {
  m_cor_id = 0;
  t_coroutine_count++;
  memset(&m_coctx, 0, sizeof(m_coctx));
  t_cur_coroutine = this;
  fprintf(stdout, "coroutine[ %d ] create\n", m_cor_id);
}

Coroutine::Coroutine(int size, char *stack_ptr)
  : m_stack_size(size), m_stack_sp(stack_ptr) {
  assert(stack_ptr);

  if (!t_main_coroutine) {
    t_main_coroutine = new Coroutine();
  }
  m_cor_id = t_cur_coroutine_id++;
  t_coroutine_count++;
  fprintf(stdout, "coroutine[ %d ] create\n", m_cor_id);
}

Coroutine::Coroutine(int size, char *stack_ptr, std::function<void()> cb)
  : m_stack_size(size), m_stack_sp(stack_ptr) {
  assert(m_stack_sp);

  if (!t_main_coroutine) {
    t_main_coroutine = new Coroutine();
  }

  setCallBack(cb);
  m_cor_id = t_cur_coroutine_id++;
  t_coroutine_count++;

  fprintf(stdout, "coroutine[ %d ] create\n", m_cor_id);
}

bool Coroutine::setCallBack(std::function<void()> cb) {
  if (this == t_main_coroutine) {
    fprintf(stderr, "main coroutine can't set callback\n");
    return false;
  }
  if (m_is_in_cofunc) {
    fprintf(stderr, "this coroutine is in CoFunction\n");
    return false;
  }

  m_call_back = cb;

  char *top = m_stack_sp + m_stack_size;

  top = reinterpret_cast<char *>(
                (reinterpret_cast<unsigned long>(top)) & -16LL);
  
  memset(&m_coctx, 0, sizeof(m_coctx));

  m_coctx.regs[kRSP] = top;
  m_coctx.regs[kRBP] = top;
  m_coctx.regs[kRETAddr] = reinterpret_cast<char *>(CoFunction);
  m_coctx.regs[kRDI] = reinterpret_cast<char *>(this);

  m_can_resume = true;

  return true;
}

Coroutine::~Coroutine() {
  t_coroutine_count--;
  fprintf(stdout, "coroutine[ %d ] die\n", m_cor_id);
}

Coroutine* Coroutine::GetCurrentCoroutine() {
  if (t_cur_coroutine == nullptr) {
    t_main_coroutine = new Coroutine();
    t_cur_coroutine = t_main_coroutine();
  }
  return t_cur_coroutine;
}

Coroutine* Coroutine::GetMainCoroutine() {
  if (t_main_coroutine) {
    return t_main_coroutine;
  }
  t_main_coroutine = new Coroutine();
  return t_main_coroutine;
}

Coroutine* Coroutine::IsMainCoroutine() {
  if (t_main_coroutine == nullptr || 
      t_cur_coroutine == t_main_coroutine) {
    return true;
  }
  return false;
}


void Coroutine::Yield() {
  if (!t_enable_coroutine_swap) {
    fprintf(stderr, "can't yield, because disable coroutine swap\n");;
    return;
  }
  if (t_main_coroutine == nullptr) {
    fprintf(stderr, "main coroutine is nullptr\n");
    return;
  }
  if (t_cur_coroutine == t_main_coroutine) {
    fprintf(stderr, "current coroutine is main coroutine\n");
    return;
  }

  Coroutine *co = t_cur_coroutine;
  t_cur_coroutine = t_main_coroutine;
  coctx_swap(&(co->m_coctx), &(t_main_coroutine->m_coctx));
}

void Coroutine::Resume() {
  if (t_cur_coroutine != t_main_coroutine) {
    fprintf(stderr, 
      "swap error, current coroutine must be main coroutine\n");
    return;
  }

  if (!t_main_coroutine) {
    fprintf(stderr, "main coroutine is nullptr\n");
    return;
  }
  if (!co || !co->m_can_resume) {
    fprintf(stderr, 
      "pending coroutine is nullptr or can_resume is false\n");
    return;
  }

  if (t_cur_coroutine == co) {
    fprintf(stderr, "current coroutine is pending cor, need't swap\n");
    return;
  }

  t_cur_coroutine = co;

  coctx_swap(&(t_main_coroutine->m_coctx), &(co->m_coctx));
  // DebugLog << "swap back";
}

}