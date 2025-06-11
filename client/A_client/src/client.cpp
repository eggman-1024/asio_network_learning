#include <iostream>
#include <cstring>
#include <boost/asio.hpp>

const size_t BUF_MAX_SIZE = 1024;

// 与服务器进行通信
void session(boost::asio::ip::tcp::socket &socket);

int main(){
    try{
        std::string target_ip = "127.0.0.1";
        unsigned short target_port = 12345;
        // 创建一个io_context对象
        boost::asio::io_context io_context;
        // 创建一个TCP socket
        boost::asio::ip::tcp::socket socket(io_context);
        // 构造连接端点
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(target_ip), target_port);
        // 连接到服务器
        boost::system::error_code error = boost::asio::error::host_not_found;
        socket.connect(endpoint, error);
        if (error) {
            std::cerr << "Error connecting to server: " << error.message() << std::endl;
            return 1;
        }
        // 与服务器通信
        session(socket);
    }catch(std::exception &e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}

void session(boost::asio::ip::tcp::socket &socket){
    try{
        char buf[BUF_MAX_SIZE];
        while(true){
            // 准备发送数据
            std::cout << "Enter message to send(q to quit): ";
            std::cin.getline(buf, BUF_MAX_SIZE);
            size_t msg_len = strlen(buf);
            if(msg_len == 1 && buf[0] == 'q'){
                std::cout << "Exiting session." << std::endl;
                break;
            }

            // 发送数据到服务器
            boost::asio::write(socket, boost::asio::buffer(buf, msg_len));

            // 从服务器接收数据
            msg_len = socket.read_some(boost::asio::buffer(buf));

            // 处理接收到的数据(此处进行打印)
            std::cout << "Received from server: ";
            std::cout.write(buf, msg_len);
            std::cout << std::endl;
        }
    }catch(std::exception &e){
        throw e;
    }
}