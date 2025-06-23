#pragma once
#include <iostream>
#include <cstring>

class CMsgNode{
public:
    CMsgNode() = default;
    virtual ~CMsgNode() {
        delete[] _data;
    }
    char* GetData() {
        return _data;
    }
    uint32_t GetMsgLen() {
        return _msg_len;
    }


protected:
    char *_data;
    uint32_t _msg_len; // 消息长度(字节)
};

// 用于存放发送的消息的节点
class CSendMsgNode : public CMsgNode {
public:
    CSendMsgNode() = default;
    CSendMsgNode(char* data, uint32_t msg_len);
    CSendMsgNode(std::string str);
};

// 用于存放接收的消息的节点
class CRecvMsgNode : public CMsgNode {
public:
    CRecvMsgNode();
    enum { _MAX_MSG_LENGTH = 2048 }; // 最大消息长度, 消息超过该长度则断开连接
};