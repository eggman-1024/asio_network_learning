#include "../include/CSession.h"

CSession::CSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string& session_id, CServer* server)
    : _socket(socket), _session_id(session_id), _server(server) {}

CSession::~CSession() {
    std::cout << "Session " << _session_id << " destructed" << std::endl;
}

// 开始异步读，由server调用
void CSession::Start(){
    // 注册读事件
    std::cout << "Start session use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
    std::cout << "shared_from_this() address: " << shared_from_this().get() << std::endl;
    _socket->async_read_some(
        boost::asio::buffer(_recv_pack_data, _PACK_MAX_SIZE),
         // 使用shared_from_this()获取当前对象的shared_ptr
        [this, self_ptr = shared_from_this()](const boost::system::error_code& error, size_t recv_pack_size) {
            std::cout << "1 self_ptr use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
            HandleRead(error, recv_pack_size, self_ptr);
            std::cout << "2 self_ptr use_count: " << _server->_sessions[_session_id].use_count() << std::endl; 
        }
    );
    std::cout << "Start end session use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
}

void CSession::HandleRead(const boost::system::error_code& error, size_t recv_pack_size, std::shared_ptr<CSession> self_ptr) {
    if (!error) {
         // 对读取到的消息进行处理（进行打印）
        std::cout << _session_id.substr(24) << ": ";
        std::cout.write(_recv_pack_data, recv_pack_size);
        std::cout << std::endl;

        // 准备数据发送给客户端
        _recv_pack_data[recv_pack_size] = '~';
        std::cout << "HandleRead session use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
        // 注册写事件
        boost::asio::async_write(
            *_socket,
            boost::asio::buffer(_recv_pack_data, recv_pack_size + 1),
            [this, self_ptr](const boost::system::error_code& error, size_t send_pack_size){
                std::cout << "3 self_ptr use_count: " << _server->_sessions[_session_id].use_count() << std::endl; 
                HandleWrite(error, self_ptr);
                std::cout << "4 self_ptr use_count: " << _server->_sessions[_session_id].use_count() << std::endl; 
            }
        );
    } else {
        std::cout << "read error session use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
        std::cerr << "Read error in session " << _session_id << ": " << error.message() << std::endl;
        _server->ClearSession(_session_id); // 清除会话
        std::cout << "read error end session use_count: " << self_ptr.use_count() << std::endl;
    }
    std::cout << "HandleRead end session use_count: " << self_ptr.use_count() << std::endl;
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> self_ptr){
    if(!error){
        std::cout << "HandleWrite session use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
        // 注册读事件
        _socket->async_read_some(
            boost::asio::buffer(_recv_pack_data, _PACK_MAX_SIZE),
            [this, self_ptr](const boost::system::error_code& error, size_t recv_pack_size){
                HandleRead(error, recv_pack_size, self_ptr);
            }
        );
    }else{
        std::cout << "write error session use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
        std::cerr << "Send msg error: " << error.message() << std::endl;
        _server->ClearSession(_session_id);
        std::cout << "write error end session use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
    }
    std::cout << "HandleWrite end session use_count: " << _server->_sessions[_session_id].use_count() << std::endl;
}