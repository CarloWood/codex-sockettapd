#include "sys.h"
#include "Sockettapd.h"
#include "utils/AIAlert.h"

Sockettapd::Sockettapd(int argc, char* argv[])
{
  Application::initialize(argc, argv);
}

Sockettapd::~Sockettapd() = default;

//virtual
bool Sockettapd::parse_command_line_parameter(std::string_view arg, int argc, char* argv[], int* index)
{
  DoutEntering(dc::notice, "Sockettapd::parse_command_line_parameter(\"" << arg << "\", " << argc << ", " << debug::print_argv(argv) << ", &" << index << ")");

  if (arg == "--restart")
  {
    opt_restart_ = true;
    return true;
  }

  if (arg == "--foregound")
  {
    opt_foreground_ = true;
    return true;
  }

  if (arg == "--projectdir")
  {
    ++*index;
    if (*index >= argc)
      THROW_ALERT("Missing argument for --projectdir");
    project_dir_ = argv[*index];
    Dout(dc::notice, "project_dir_ set to " << project_dir_ << ".");
    return true;
  }

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

//static
void Sockettapd::goto_background()
{
}
