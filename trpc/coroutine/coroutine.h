#ifndef TRPC_COROUTINE_COROUTINE_H
#define TRPC_COROUTINE_COROUTINE_H

#include "trpc/coroutine/coctx.h"
#include <functional>
#include <memory>

namespace trpc {

class Coroutine{
  public:
    typedef std::shared_ptr<Coroutine> ptr;
    std::function<void()> m_call_back;
  public:
    Coroutine(int size, char *stack_ptr);

    Coroutine(int size, char *stack_ptr, std::function<void()> cb);

    ~Coroutine();

    bool setCallBack(std::function<void()> cb);

    int getCorId() const {
      return m_cor_id;
    }

    void setIsInCoFunc(const bool v) {
      m_is_in_cofunc = v;
    }

    bool getIsInCoFunc() const {
      return m_is_in_cofunc;
    }

    std::string getMsgNo() {
      return m_msg_no;
    }

    void setMsgNo(const std::string& msg_no) {
      m_msg_no = msg_no;
    }

    void setIndex(int index) {
      m_index = index;
    }

    int getIndex() {
      return m_index;
    }

    char* getStackPtr() {
      return m_stack_sp;
    }

    int getStackSize() {
      return m_stack_size;
    }

    void setCanResume(bool v) {
      m_can_resume = v;
    }

    static void Yield();

    static void Resume(Coroutine *cor);

    static Coroutine *GetCurrentCoroutine();

    static Coroutine *GetMainCoroutine();

    static bool IsMainCoroutine();

    static void SetCoroutineSwapFlag(bool value);

    static bool GetCoroutineSwapFlag();

  private:
    int m_cor_id {0}; // coroutine id
    coctx m_coctx; // coroutine context
    int m_stack_size {0}; // size of stack memory sapce
    char *m_stack_sp {NULL}; 
    bool m_is_in_cofunc {false};
    std::string m_msg_no;

    bool m_can_resume {true};

    int m_index {-1;} // index in coroutine pool
};

}
#endif