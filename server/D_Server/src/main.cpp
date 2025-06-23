#include <iostream>
#include <boost/asio.hpp>
#include "../include/CServer.h"

int main(){
    std::cout << "Server Started" << std::endl;
    try{
        unsigned short port = 12345;
        boost::asio::io_context io_context;
        CServer server(io_context, port);
        io_context.run(); // 开启事件循环，阻塞主线程，直到所有异步任务完成或显示停止
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}