#include "sys.h"
#include "Sockettapd.h"
#include "STListenSocket.h"
#include "evio/EventLoop.h"
#include "utils/debug_ostream_operators.h"      // Needed to write error to Dout.
#include "utils/AIAlert.h"
#include "debug.h"

int main(int argc, char* argv[])
{
  Debug(NAMESPACE_DEBUG::init());
  Dout(dc::notice, "Entering main()");

  try
  {
    Sockettapd application(argc, argv);

    std::string projectdir = ::getenv("PROJECTDIR");
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
  }
  catch (NoError const&)
  {
  }

  Dout(dc::notice, "Leaving main()");
}
