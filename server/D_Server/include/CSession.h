#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "CServer.h"
#include "CMsgNode.h"
#include <queue>


class CServer;
class CSession : public std::enable_shared_from_this<CSession>{
public:
    CSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string& session_id, CServer* server); // 继承自enable_shared_from_this，用于在回调函数中获取this的shared_ptr
    ~CSession();
    void Start();
    void SendMsg(char* msg, uint32_t msg_len); //发送消息
private:
    void HandleRead(const boost::system::error_code& error, size_t pack_size, std::shared_ptr<CSession> self_ptr);  // 读操作完成后的回调函数
    void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> self_ptr);   // 写操作完成后的回调函数

    std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
    enum { _PACK_MAX_SIZE = 4096 };
    char _recv_pack_data[_PACK_MAX_SIZE]; // 用于存放接收到的数据
    CServer* _server; // 保存server指针，用于清除session
    std::string _session_id; // 该session的id
    std::queue<std::shared_ptr<CSendMsgNode>> _sending_msg_queue;
    std::mutex _msg_queue_mutex; // 保护发送消息队列的互斥锁
    std::shared_ptr<CRecvMsgNode> _cur_recv_msg; // 当前正在接受的消息
};