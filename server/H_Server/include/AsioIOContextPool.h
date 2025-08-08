#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <boost/asio.hpp>
#include "Singleton.h"

class AsioIOContextPool : public Singleton<AsioIOContextPool> {
    friend class Singleton<AsioIOContextPool>;
    using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
public:
    ~AsioIOContextPool(){}
    boost::asio::io_context& GetIOContext();
    void Stop();
private:
    AsioIOContextPool();
    std::vector<std::thread> _threads;
    std::vector<boost::asio::io_context> _io_contexts;
    std::vector<WorkGuard> _work_guards;
    size_t _pool_size;
    size_t _next_idx;
    std::mutex _mutex;
};