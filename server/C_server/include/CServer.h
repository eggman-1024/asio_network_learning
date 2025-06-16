#pragma once
#include <boost/asio.hpp>
#include "CSession.h"
#include <map>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

class CSession;
class CServer {
public:
    CServer(boost::asio::io_context& io_context, unsigned short port);
    void ClearSession(const std::string& session_id);
    std::map<std::string, std::shared_ptr<CSession>> _sessions;

private:
    void WaitAccept();
    std::string GenerateSessionId();

    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::io_context& _io_context;
};