#pragma once

#include "STDecoder.h"
#include "evio/AcceptedSocket.h"
#include "evio/ListenSocket.h"
#include "evio/SocketAddress.h"

// The type of the accepted socket uses STDecoder as decoder.
using STAcceptedSocket = evio::AcceptedSocket<STDecoder, evio::OutputStream>;

// The type of the listen socket of this daemon.
class STListenSocket : public evio::ListenSocket<STAcceptedSocket>
{
  void spawn_accepted(int fd, evio::SocketAddress const& remote_address) override
  {
    auto sock = evio::create<STAcceptedSocket>();
    if (Sockettapd::instance().one_shot())
    {
      Dout(dc::notice, "Calling sock->on_disconnected()");
      sock->on_disconnected([](int& UNUSED_ARG(allow_deletion_count), bool UNUSED_ARG(cleanly_closed)) {
        // If --one-shot then terminate the application if the (only) client exits.
        Application::instance().quit();
      });
    }
    sock->init(fd, remote_address);
    new_connection(*sock);
  }

  // As soon as the first connection is received, close this listen socket.
  void new_connection(accepted_socket_type& UNUSED_ARG(accepted_socket)) override
  {
    close();
  }
};
