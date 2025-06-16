#include "../include/CSession.h"

CServer::CServer(boost::asio::io_context& io_context, unsigned short port)
    : _acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), _io_context(io_context) {
    WaitAccept();
}

// 异步等待连接
void CServer::WaitAccept(){
    std::shared_ptr<boost::asio::ip::tcp::socket> socket = std::make_shared<boost::asio::ip::tcp::socket>(_io_context);
    // 注册连接事件
    _acceptor.async_accept(
        *socket,
        [this, socket](const boost::system::error_code& error){
            if(!error){
                // 获取客户端端点信息
                boost::asio::ip::tcp::endpoint client_endpoint = socket->remote_endpoint();
                std::string client_ip = client_endpoint.address().to_string(); // 客户端IP
                unsigned short client_port = client_endpoint.port();          // 客户端端口

                std::string session_id = GenerateSessionId();
                std::shared_ptr<CSession> session = std::make_shared<CSession>(socket, session_id, this);
                _sessions[session_id] = session; // 保存会话到map中
                std::cout << "New session connected with ID: " << session_id << std::endl;
                std::cout << "session use_count: " << session.use_count() << std::endl;
                // 输出客户端连接信息
                std::cout << "新会话 ID: " << session_id 
                        << " | 客户端: " << client_ip << ":" << client_port << std::endl;
                session->Start(); // 启动会话处理
                std::cout << "after start session use_count: " << session.use_count() << std::endl;
                WaitAccept(); // 继续等待下一个连接
            }else{
                std::cerr << "Accept error: " << error.message() << std::endl;
            }
        }
    );
}

// 为每个Session生成一个唯一id
std::string CServer::GenerateSessionId() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uuid);
}

// 移除指定id的session
void CServer::ClearSession(const std::string& session_id) {
    _sessions.erase(session_id);
}