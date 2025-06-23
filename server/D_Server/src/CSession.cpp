#include "../include/CSession.h"

CSession::CSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string& session_id, CServer* server)
    : _socket(socket), _session_id(session_id), _server(server) {
    _cur_recv_msg = std::make_shared<CRecvMsgNode>();
}

CSession::~CSession() {
    std::cout << "Session " << _session_id << " destructed" << std::endl;
}

// 开始异步读，由server调用
void CSession::Start() {
    // 注册读事件
    _socket->async_read_some(
        boost::asio::buffer(_recv_pack_data, _PACK_MAX_SIZE),
        [this, self_ptr = shared_from_this()](const boost::system::error_code& error, size_t recv_pack_size) {
            HandleRead(error, recv_pack_size, self_ptr);
        }
    );
}

// 发送消息
void CSession::SendMsg(char* msg, uint32_t msg_len){
    std::lock_guard<std::mutex> lock(_msg_queue_mutex);
    bool is_sending = !_sending_msg_queue.empty(); // 如果发送信息队列里还有消息说明有消息正在发送
    _sending_msg_queue.push(std::make_shared<CSendMsgNode>(msg, msg_len));
    if (is_sending) {
        return;
    }
    // 如果没有正在发送的消息，则开始发送
    boost::asio::async_write(
        *_socket,
        boost::asio::buffer(_sending_msg_queue.front()->GetData(), _sending_msg_queue.front()->GetMsgLen()),
        [this, self_ptr = shared_from_this()](const boost::system::error_code& error, size_t send_pack_size) {
            HandleWrite(error, self_ptr);
        }
    );
}

// 读操作完成后的回调函数
void CSession::HandleRead(const boost::system::error_code& error, size_t recv_pack_size, std::shared_ptr<CSession> self_ptr) {
    if (!error) {
        // 对读取到的消息进行处理（进行打印）
        if(recv_pack_size > CRecvMsgNode::_MAX_MSG_LENGTH){
            std::cerr << "Msg too long, disconnecting session " << _session_id << std::endl;
            _server->ClearSession(_session_id); // 清除会话
            return;
        }
        std::cout << _session_id.substr(24) << ": ";
        std::cout.write(_recv_pack_data, recv_pack_size);
        std::cout << std::endl;

        // 准备数据发送给客户端
        _recv_pack_data[recv_pack_size] = '~';

        // 向对端发送数据
        SendMsg(_recv_pack_data, recv_pack_size + 1);

        // 注册读事件
        _socket->async_read_some(
            boost::asio::buffer(_recv_pack_data, _PACK_MAX_SIZE),
            [this, self_ptr](const boost::system::error_code& error, size_t recv_pack_size) {
                HandleRead(error, recv_pack_size, self_ptr);
            }
        );
    } else {
        std::cerr << "Read error in session " << _session_id << ": " << error.message() << std::endl;
        _server->ClearSession(_session_id); // 清除会话
    }
}

// 写操作完成后的回调函数
void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> self_ptr) {
    if (!error) {
        std::lock_guard<std::mutex> lock(_msg_queue_mutex);
        _sending_msg_queue.pop();  // 发送完成后将消息从队列中删除

        // 如果队列中还有消息，则继续注册写事件
        if(!_sending_msg_queue.empty()){
            auto &send_msg_node = _sending_msg_queue.front();
            boost::asio::async_write(
                *_socket,
                boost::asio::buffer(send_msg_node->GetData(), send_msg_node->GetMsgLen()),
                [this, self_ptr](const boost::system::error_code &error, size_t send_pack_size) {
                    HandleWrite(error, self_ptr);
                }
            );
        }
    }
    else{
        std::cerr << "Send msg error: " << error.message() << std::endl;
        _server->ClearSession(_session_id);
    }
}

