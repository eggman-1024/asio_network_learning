#include "../include/CMsgNode.h"

CSendMsgNode::CSendMsgNode(char* data, uint32_t msg_len){
    _msg_len = msg_len;
    _data = new char[_msg_len];
    memcpy(_data, data, _msg_len);
}

CSendMsgNode::CSendMsgNode(std::string str) {
    _msg_len = str.length();
    _data = new char[_msg_len];
    memcpy(_data, str.c_str(), _msg_len);
}

CRecvMsgNode::CRecvMsgNode() {
    _msg_len = 0;
    _data = new char[_MAX_MSG_LENGTH];
}
