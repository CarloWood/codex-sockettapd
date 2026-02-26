#pragma once

#include "Application.h"
#include <filesystem>

// Sockettapd
//
// Daemon application entrypoint.
class Sockettapd final : public Application
{
 private:
  bool opt_foreground_{false};          // Set if --foreground.
  bool opt_restart_{false};             // Set if --restart.
  std::filesystem::path project_dir_;   // Argument passed to --projectdir <dir>.

 public:
  // Construct and initialize base application state.
  Sockettapd(int argc, char* argv[]);

  // Destroy sockettapd object.
  ~Sockettapd();

  // Run as daemon.
  static void goto_background();

  // Option accessors.
  bool restart() const { return opt_restart_; }
  bool foreground() const { return opt_foreground_; }

 protected:
  // Parse remountd-specific command line parameters.
  bool parse_command_line_parameter(std::string_view arg, int argc, char* argv[], int* index) override;

  // Print sockettapd-specific usage suffix.
  void print_usage_extra(std::ostream& os) const override;

  // Return the application display name.
  std::u8string application_name() const override;
};
