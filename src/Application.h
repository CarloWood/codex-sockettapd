#pragma once

#include "ApplicationInfo.h"
#include "UUID.h"
#include "utils/threading/Gate.h"
#include "threadpool/AIThreadPool.h"
#include <string>
#include <memory>
#ifdef CWDEBUG
#include "cwds/debug_ostream_operators.h"
#endif
#include "debug.h"

namespace evio {
class EventLoop;
} // namespace evio

class Application
{
 public:
  // Set up the thread pool for the application.
  static constexpr int default_number_of_threads = 2;                           // Use a thread pool of 8 threads.
  static constexpr int default_reserved_threads = 1;                            // Reserve 1 thread for each priority.

  enum class QueuePriority {
    high,
    medium,
    low
  };

  static Application& instance() { return *s_instance; }

 protected:
   // Create the thread pool.
  AIThreadPool thread_pool_;

  // And the thread pool queues.
//  AIQueueHandle m_high_priority_queue;
//  AIQueueHandle m_medium_priority_queue;
  AIQueueHandle low_priority_queue_;

  // Set up the I/O event loop.
  std::unique_ptr<evio::EventLoop> event_loop_;

  // To stop the main thread from exiting until the codex closed the connection.
  mutable utils::threading::Gate until_codex_disconnects_;      // This Gate is opened when Codex disconnects.

 private:
  static Application* s_instance;                               // There can only be one instance of Application. Allow global access.
  ApplicationInfo application_info_;                            // Metadata captured during initialize().

  UUID thread_id_;                                              // The Thread ID of the Codex chat session.

 public:
  Application();
  ~Application();

 public:
  void initialize(int argc = 0, char** argv = nullptr);

  // Causes run() to return.
  void quit();

  AIQueueHandle low_priority_queue() const { return low_priority_queue_; }

  void run();

  void set_thread_id(UUID const& thread_id)
  {
    DoutEntering(dc::notice, "Application::set_thread_id(" << thread_id << ")");
    thread_id_ = thread_id;
  }

 protected:
  virtual void parse_command_line_parameters(int argc, char* argv[]);

  // Override this function to change the number of worker threads.
  virtual int thread_pool_number_of_worker_threads() const;

  // Override this function to change the size of the thread pool queues.
  virtual int thread_pool_queue_capacity(QueuePriority UNUSED_ARG(priority)) const;

  // Override this function to change the number of reserved threads for each queue (except the last, of course).
  virtual int thread_pool_reserved_threads(QueuePriority UNUSED_ARG(priority)) const;

 public:
  // Override this function to change the default ApplicationInfo values.
  virtual std::u8string application_name() const;

  // Override this function to change the default application version. The result should be a value returned by vk_utils::encode_version.
  virtual uint32_t application_version() const;
};
