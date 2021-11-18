#include "thread_pool/thread_pool.h"

int main() {
  ThreadPool thread_pool;

  auto f = thread_pool.enqueue([](int value) { return value; }, 42);

  std::cout << f.get() << std::endl;

  pthread_exit(nullptr);
}
