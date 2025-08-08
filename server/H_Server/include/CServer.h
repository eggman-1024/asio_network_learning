#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include "CSession.h"
#include <map>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "AsioIOContextPool.h"

class CSession;
class CServer {
public:
    CServer(boost::asio::io_context& io_context, unsigned short port);
    void ClearSession(const std::string& session_id);
private:
    void WaitAccept();
    std::string GenerateSessionId();

    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::io_context& _io_context;
    std::map<std::string, std::shared_ptr<CSession>> _sessions; // 存储Session指针，键为Session的唯一id
};