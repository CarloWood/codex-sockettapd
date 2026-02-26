#pragma once

#include "Application.h"

// Sockettapd
//
// Daemon application entrypoint.
class Sockettapd final : public Application
{
 public:
  // Construct and initialize base application state.
  Sockettapd(int argc, char* argv[]);

  // Destroy sockettapd object.
  ~Sockettapd();

 protected:
  // Parse remountd-specific command line parameters.
  bool parse_command_line_parameter(std::string_view arg, int argc, char* argv[], int* index) override;

  // Print sockettapd-specific usage suffix.
  void print_usage_extra(std::ostream& os) const override;

  // Return the application display name.
  std::u8string application_name() const override;
};
