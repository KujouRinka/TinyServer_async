#ifndef TINYSERVER_ASYNC_THREADPOOL_H
#define TINYSERVER_ASYNC_THREADPOOL_H


class ThreadPool {
public:
    ThreadPool(int max_threads, int max_queue_size);

    void addTask();
};


#endif //TINYSERVER_ASYNC_THREADPOOL_H
