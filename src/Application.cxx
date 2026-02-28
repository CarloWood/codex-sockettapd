#include "sys.h"
#include "Application.h"
#include "version.h"
#include "utils.h"
#include "evio/EventLoop.h"
#include "utils/AIAlert.h"
#ifdef CWDEBUG
#include "utils/debug_ostream_operators.h"
#endif
#include "debug.h"

//static
Application* Application::s_instance;

// Construct the base class of the Application object.
//
// Because this is a base class, virtual functions can't be used in the constructor.
// Therefore initialization happens after construction.
Application::Application()
{
  DoutEntering(dc::notice, "Application::Application()");
  s_instance = this;
}

// This instantiates the destructor of our std::unique_ptr's.
// Because it is here instead of the header we can use forward declarations for EventLoop.
Application::~Application()
{
  DoutEntering(dc::notice, "Application::~Application()");
  // Revoke global access.
  s_instance = nullptr;
}

//virtual
void Application::parse_command_line_parameters(int argc, char* argv[])
{
  DoutEntering(dc::notice, "Application::parse_command_line_parameters(" << argc << ", " << NAMESPACE_DEBUG::print_argv(argv) << ")");

  for (int i = 1; i < argc; ++i)
  {
    std::string_view const arg(argv[i]);
    if (arg == "--help" || arg == "-h")
    {
      print_usage();
      throw NoError{};
    }

    if (arg == "--version")
    {
      print_version();
      throw NoError{};
    }

    if (parse_command_line_parameter(arg, argc, argv, &i))
      continue;

    THROW_ALERT("Unknown argument: [ARG]", AIArgs("[ARG]", arg));
  }
}

//virtual
std::u8string Application::application_name() const
{
  return application_info_.application_name();
}

//virtual
uint32_t Application::application_version() const
{
  return application_info_.encoded_version();
}

//virtual
int Application::thread_pool_number_of_worker_threads() const
{
  return default_number_of_threads;
}

//virtual
int Application::thread_pool_queue_capacity(QueuePriority UNUSED_ARG(priority)) const
{
  // By default, make the size of each thread pool queue equal to the number of worker threads.
  return thread_pool_->number_of_workers();
}

//virtual
int Application::thread_pool_reserved_threads(QueuePriority UNUSED_ARG(priority)) const
{
  return default_reserved_threads;
}

void Application::print_usage() const
{
  std::cerr << "Usage: " << utf8_to_string(application_name()) << " [--help] [--version]";
  print_usage_extra(std::cerr);
  std::cerr << "\n";
}

void Application::print_version() const
{
  auto [major, minor] = application_info_.version();
  std::cout << utf8_to_string(application_name()) << " v" << major << '.' << minor << "\n";
}

//virtual
bool Application::parse_command_line_parameter(std::string_view /*arg*/, int /*argc*/, char*[] /*argv*/, int* /*index*/)
{
  return false;
}

//virtual
void Application::print_usage_extra(std::ostream& /*os*/) const
{
}

// Finish initialization of a default constructed Application.
void Application::initialize(int argc, char** argv)
{
  DoutEntering(dc::notice, "Application::initialize(" << argc << ", " << NAMESPACE_DEBUG::print_argv(argv) << ")");

  // Only call initialize once. Calling it twice leads to a nasty dead-lock that was hard to debug ;).
  ASSERT(!event_loop_);

  // From version.h.
  application_info_.set_application_name(application_name_c);
  application_info_.set_application_version(application_version_c);

  try
  {
    // Parse command line parameters before doing any initialization, so the command line arguments can influence the initialization too.
    // Allow the user to override stuff.
    if (argc > 0)
      parse_command_line_parameters(argc, argv);

    // Notify the derived class that all command line parameters have been parsed.
    command_line_parameters_parsed();

    // Now that we (potentially) have switched to the background we can create the thread pool.
    thread_pool_ = std::make_unique<AIThreadPool>(thread_pool_number_of_worker_threads());

    // Initialize the first thread pool queue.
    low_priority_queue_ = thread_pool_->new_queue(thread_pool_queue_capacity(QueuePriority::low));

    // Initialize the thread pool.
    Debug(thread_pool_->set_color_functions([](int color){
      static std::array<std::string, 32> color_on_escape_codes = {
        "\e[38;5;1m",
        "\e[38;5;190m",
        "\e[38;5;2m",
        "\e[38;5;33m",
        "\e[38;5;206m",
        "\e[38;5;3m",
        "\e[38;5;130m",
        "\e[38;5;15m",
        "\e[38;5;84m",
        "\e[38;5;63m",
        "\e[38;5;200m",
        "\e[38;5;202m",
        "\e[38;5;9m",
        "\e[38;5;8m",
        "\e[38;5;160m",
        "\e[38;5;222m",
        "\e[38;5;44m",
        "\e[38;5;5m",
        "\e[38;5;210m",
        "\e[38;5;28m",
        "\e[38;5;11m",
        "\e[38;5;225m",
        "\e[38;5;124m",
        "\e[38;5;10m",
        "\e[38;5;4m",
        "\e[38;5;140m",
        "\e[38;5;136m",
        "\e[38;5;250m",
        "\e[38;5;6m",
        "\e[38;5;27m",
        "\e[38;5;123m",
        "\e[38;5;220m"
      };
      ASSERT(0 <= color && color < color_on_escape_codes.size());
      return color_on_escape_codes[color];
    }));
  }
  catch (AIAlert::Error const& error)
  {
    // If an exception is thrown before we have at least one thread pool queue, then that
    // exception is never caught by main(), because leaving initialize() before that point
    // will cause the application to terminate in AIThreadPool::Worker::tmain(int) with
    // FATAL         : No thread pool queue found after 100 ms. [...]
    // while the main thread is blocked in the destructor of AIThreadPool, waiting to join
    // with the one thread that was already created.
    Dout(dc::warning, "\e[31m" << error << ", caught in src/Application.cxx\e[0m");
    return;
  }

  // Initialize the remaining thread pool queues.
  //m_medium_priority_queue = m_thread_pool.new_queue(thread_pool_queue_capacity(QueuePriority::medium), thread_pool_reserved_threads(QueuePriority::medium));
  //m_low_priority_queue    = m_thread_pool.new_queue(thread_pool_queue_capacity(QueuePriority::low));

  // Set up the I/O event loop.
  event_loop_ = std::make_unique<evio::EventLoop>(low_priority_queue_ COMMA_CWDEBUG_ONLY("\e[36m", "\e[0m"));
}

void Application::quit()
{
  DoutEntering(dc::notice, "Application::quit()");
  until_codex_disconnects_.open();
}

// Run the application.
// This function does not return until the program terminated.
void Application::run()
{
  // The main thread goes to sleep for the entirety of the application.
  until_codex_disconnects_.wait();

  Dout(dc::notice, "======= Program terminating ======");

  // Application terminated cleanly.
  event_loop_->join();
}
