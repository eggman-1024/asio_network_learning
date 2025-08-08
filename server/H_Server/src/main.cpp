#include <iostream>
#include <boost/asio.hpp>
#include "../include/CServer.h"
#include "../include/AsioIOContextPool.h"

int main(){
    try{
        unsigned short port = 12345;
        std::shared_ptr<AsioIOContextPool> io_context_pool = AsioIOContextPool::GetInstance();
        boost::asio::io_context io_context;
        // 使用信号监听对象监听SIGINT（ctrl+c）和SIGTERM（kill）信号
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait(
            [&io_context, io_context_pool](auto, auto){
                io_context.stop();
                io_context_pool->Stop(); // 停止IO上下文池
            }
        );
        CServer server(io_context, port);
        std::cout << "Server Started" << std::endl;
        io_context.run(); // 开启事件循环，阻塞主线程，直到所有异步任务完成或显示停止
        std::cout << "Server Closed" << std::endl;
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}