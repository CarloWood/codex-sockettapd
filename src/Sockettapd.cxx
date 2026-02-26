#include "sys.h"
#include "Sockettapd.h"

Sockettapd::Sockettapd(int argc, char* argv[])
{
  Application::initialize(argc, argv);
}

Sockettapd::~Sockettapd() = default;

//virtual
bool Sockettapd::parse_command_line_parameter(std::string_view arg, int argc, char* argv[], int* index)
{
  DoutEntering(dc::notice, "Sockettapd::parse_command_line_parameter(\"" << arg << "\", " << argc << ", " << debug::print_argv(argv) << ", &" << index << ")");
  return false;
}

//virtual
void Sockettapd::print_usage_extra(std::ostream& os) const
{
}

//virtual
std::u8string Sockettapd::application_name() const
{
  return u8"sockettapd";
}
