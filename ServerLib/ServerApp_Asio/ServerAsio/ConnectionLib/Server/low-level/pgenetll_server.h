#ifndef PGENETLL_SERVER_H
#define PGENETLL_SERVER_H

#include <asio.hpp>
#include <functional>

#include <ConnectionLib/Shared/util/ThreadedQueue.h>
#include "pgenetll_session.h"

using asio::ip::tcp;

class PGENETLL_Server
{
public:
    PGENETLL_Server(asio::io_service& io_service, short port);

    void startAccepting();


    void setRawPacketToPush(const std::shared_ptr<ThreadedQueue_RawData> &packetToPush);
    void setIncomingConnectionHandler(const std::function<void (std::shared_ptr<PGENETLL_Session>)> &value);

private:
    // Will be forwarded to the session:
    std::shared_ptr<ThreadedQueue_RawData> m_rawPacketToPush;

    std::function<void(std::shared_ptr<PGENETLL_Session>)> m_incomingConnectionHandler;

    tcp::acceptor m_pgenetll_acceptor;
    tcp::socket m_pgenetll_nextsocket;
};

#endif // PGENETLL_SERVER_H
