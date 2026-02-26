#include "sys.h"
#include "Application.h"
#include "ConfigSessionDecoder.h"
#include "evio/EventLoop.h"
#include "evio/SocketAddress.h"
#include "evio/AcceptedSocket.h"
#include "evio/ListenSocket.h"
#include "utils/threading/Gate.h"
#include "utils/debug_ostream_operators.h"      // Needed to write error to Dout.
#include "utils/AIAlert.h"
#include "debug.h"
#ifdef CWDEBUG
#include <libcwd/buf2str.h>
#endif

// Decoder for the accepted socket.
// ST = Socket Tap.
class STDecoder : public evio::protocol::Decoder
{
 public:
  STDecoder() = default;

 protected:
  // Call decode() with chunks ending on a newline (the default).
  void decode(int& allow_deletion_count, evio::MsgBlock&& msg) override
  {
    // Just print what was received.
    DoutEntering(dc::notice, "STDecoder::decode({" << allow_deletion_count <<
        "}, \"" << buf2str(msg.get_start(), msg.get_size()) << "\") [" << this << ']');

#if 0
    // Close this connection as soon as we received "Hello world!\n".
    if (std::string_view(msg.get_start(), msg.get_size()) == "Hello world!\n")
      close_input_device(allow_deletion_count);
#endif

    std::string_view const line = msg.view();
    if (line.starts_with("<config-session>"))
    {
      config_session_decoder_.begin(*this);
      switch_protocol_decoder(config_session_decoder_);
    }
  }

 private:
  ConfigSessionDecoder config_session_decoder_;
};

// The type of the accepted socket uses STDecoder as decoder.
using STAcceptedSocket = evio::AcceptedSocket<STDecoder, evio::OutputStream>;

// The type of the listen socket of this daemon.
class STListenSocket : public evio::ListenSocket<STAcceptedSocket>
{
  void spawn_accepted(int fd, evio::SocketAddress const& remote_address) override
  {
    auto sock = evio::create<STAcceptedSocket>();
    sock->on_disconnected([](int& UNUSED_ARG(allow_deletion_count), bool UNUSED_ARG(cleanly_closed)) {
      // Terminate application if the (only) client exits.
      Application::instance().quit();
    });
    sock->init(fd, remote_address);
    new_connection(*sock);
  }

  // As soon as the first connection is received, close this listen socket.
  void new_connection(accepted_socket_type& UNUSED_ARG(accepted_socket)) override
  {
    close();
  }
};

int main(int argc, char* argv[])
{
  Debug(NAMESPACE_DEBUG::init());
  Dout(dc::notice, "Entering main()");

  Application application;
  application.initialize(argc, argv);

  std::string projectdir = ::getenv("TOPPROJECT");
  std::string socket_address = projectdir + "/shell_exec.sock";
  evio::SocketAddress endpoint(socket_address);
  Dout(dc::notice, "endpoint = " << endpoint);

  // An object that allows us to write to an ostream
  // in order to write to (the buffer of) the socket.
  evio::OutputStream socket_stream;

  try
  {
    // Create a listen socket.
    auto listen_socket = evio::create<STListenSocket>();
    listen_socket->listen(endpoint);

    // Run the application.
    application.run();
  }
  catch (AIAlert::Error const& error)
  {
    Dout(dc::warning, error);
  }

  Dout(dc::notice, "Leaving main()");
}
