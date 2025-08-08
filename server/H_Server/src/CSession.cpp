#include "../include/CSession.h"

CSession::CSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string& session_id, CServer* server)
    : _socket(socket), _session_id(session_id), _server(server) {
    _cur_recv_msg = std::make_shared<CRecvMsgNode>();
}

CSession::~CSession() {
    std::cout << "Session " << _session_id << " destructed" << std::endl;
}

std::string CSession::GetSessionId(bool full){
    if(full) return _session_id;
    return _session_id.substr(24); // 返回去掉前24个字符的session_id
}

// 开始异步读，由server调用
void CSession::Start() {
    // 注册读事件
    boost::asio::async_read(
        *_socket,
        boost::asio::buffer(_recv_pack_data, CRecvMsgNode::_HEAD_LEN),
        [this, self_ptr = shared_from_this()](const boost::system::error_code& error, size_t recv_pack_size) {
            HandleReadHead(error, recv_pack_size, self_ptr);
        }
    );
}

// 发送消息
void CSession::SendMsg(uint16_t msg_id, char* msg, uint16_t msg_len){
    std::lock_guard<std::mutex> lock(_msg_queue_mutex);
    bool is_sending = !_sending_msg_queue.empty(); // 如果发送信息队列里还有消息说明有消息正在发送
    _sending_msg_queue.push(std::make_shared<CSendMsgNode>(msg_id, msg, msg_len));
    if (is_sending) {
        return;
    }
    // 如果没有正在发送的消息，则开始发送
    boost::asio::async_write(
        *_socket,
        boost::asio::buffer(_sending_msg_queue.front()->GetData(), _sending_msg_queue.front()->GetDataLen()),
        [this, self_ptr = shared_from_this()](const boost::system::error_code& error, size_t send_pack_size) {
            HandleWrite(error, self_ptr);
        }
    );
}

// 发送消息
void CSession::SendMsg(uint16_t msg_id, std::string msg_str){
    std::lock_guard<std::mutex> lock(_msg_queue_mutex);
    bool is_sending = !_sending_msg_queue.empty();  // 如果发送信息队列里还有消息说明有消息正在发送
    _sending_msg_queue.push(std::make_shared<CSendMsgNode>(msg_id, msg_str));  // 将消息打包为发送结点后加入发送消息队列
    if(is_sending) return;
    // 如果没有消息正在发送，则注册发送消息事件
    boost::asio::async_write(
        *_socket,
        boost::asio::buffer(_sending_msg_queue.front()->GetData(), _sending_msg_queue.front()->GetDataLen()),
        [this, self_ptr = shared_from_this()](const boost::system::error_code &error, size_t send_pack_size) {
            HandleWrite(error, self_ptr);
        }
    );
}

// 读取完消息头后的回调函数
void CSession::HandleReadHead(const boost::system::error_code& error, size_t recv_pack_size, std::shared_ptr<CSession> self_ptr) {
    if (!error) {
        _cur_recv_msg->AppendData(_recv_pack_data, recv_pack_size);
        // uint16_t msg_id = _cur_recv_msg->GetMsgId();
        uint16_t msg_len = _cur_recv_msg->GetMsgLen();
        // if(msg_id != 901 && msg_id != 1001){  // 消息id不是901且不是1001，直接断开连接
        //     std::cerr << "Invalid message id" << std::endl;
        //     _server->ClearSession(_session_id);
        //     return;
        // }
        // 对读取到的消息进行处理（进行打印）
        if(msg_len > CRecvMsgNode::_MAX_MSG_LENGTH){ // 消息长度超过消息最大长度，直接断开连接
            std::cerr << "Msg too long, disconnecting session " << _session_id << std::endl;
            _server->ClearSession(_session_id); // 清除会话
            return;
        }
        if(msg_len == 0){ // 消息长度为0，直接断开连接(说明解析失败)
            std::cerr << "Msg length is 0, disconnecting session " << _session_id << std::endl;
            _server->ClearSession(_session_id); // 清除会话
            return;
        }

        // 接收到的消息头正常
        boost::asio::async_read(
            *_socket,
            boost::asio::buffer(_recv_pack_data, msg_len),
            [this, self_ptr = shared_from_this()](const boost::system::error_code& error, size_t recv_pack_size) {
                HandleReadMsg(error, recv_pack_size, self_ptr);
            }
        );
    } else {
        std::cerr << "Read error in session " << _session_id << ": " << error.message() << std::endl;
        _server->ClearSession(_session_id); // 清除会话
    }
}

// 读取完消息体后的回调函数
void CSession::HandleReadMsg(const boost::system::error_code& error, size_t recv_pack_size, std::shared_ptr<CSession> self_ptr) {
    if (!error) {
        _cur_recv_msg->AppendData(_recv_pack_data, recv_pack_size);
        // 将消息投递给逻辑系统的逻辑队列（内部将会进行深拷贝），处理接收到的数据
        CLogicSystem::GetInstance()->PostMsgToQue(shared_from_this(), _cur_recv_msg);

        // 继续监听消息头，无需等待消息处理完成(消息被传给逻辑层处理)
        // 重置当前接收消息
        _cur_recv_msg->Reset();
        boost::asio::async_read(
            *_socket,
            boost::asio::buffer(_recv_pack_data, CRecvMsgNode::_HEAD_LEN),
            [this, self_ptr = shared_from_this()](const boost::system::error_code& error, size_t recv_pack_size) {
                HandleReadHead(error, recv_pack_size, self_ptr);
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
                boost::asio::buffer(send_msg_node->GetData(), send_msg_node->GetDataLen()),
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

