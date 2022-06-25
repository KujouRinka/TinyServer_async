#include "threadpool.h"

#include <iostream>
#include <mutex>

using namespace std;

ThreadPool::ThreadPool(int max_threads, int max_queue_size)
  : _max_queue_size(max_queue_size) {
  if (max_threads <= 0 || max_queue_size <= 0) {
    cerr << "invalid thread pool init parameter" << endl;
    exit(0);
  }
  while (max_threads--) {
    thread t(&ThreadPool::worker, this);
    t.detach();
  }
  _running = true;
}

bool ThreadPool::appendTask(function<void()> task) {
  lock_guard<mutex> g(_queue_lock);
  if (_task_queue.size() >= _max_queue_size)
    return false;
  _task_queue.push(task);
  _cv.notify_one();
  return true;
}

void ThreadPool::worker() {
  while (_running) {
    unique_lock<mutex> lk(_queue_lock);
    _cv.wait(lk, [this]() { return !_task_queue.empty(); });
    auto task_fun = _task_queue.front();
    _task_queue.pop();
    lk.unlock();
    if (task_fun != nullptr)
      task_fun();
  }
}
