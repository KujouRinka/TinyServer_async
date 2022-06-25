#ifndef TINYSERVER_ASYNC_THREADPOOL_H
#define TINYSERVER_ASYNC_THREADPOOL_H

#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

using namespace std;

class ThreadPool {
 public:
  ThreadPool(int max_threads = 16, int max_queue_size = 10000);

  bool appendTask(function<void()> task);
  void worker();

 private:
  bool _running;
  int _max_queue_size;
  mutex _queue_lock;
  queue<function<void()>> _task_queue;
  condition_variable _cv;
};

#endif //TINYSERVER_ASYNC_THREADPOOL_H
