#include "sys.h"
#include "Sockettapd.h"
#include "utils/AIAlert.h"
#include <unistd.h>

Sockettapd::Sockettapd(int argc, char* argv[])
{
  Application::initialize(argc, argv);
}

Sockettapd::~Sockettapd()
{
#ifdef CWDEBUG
  // Make sure we don't write debug output to a closed file.
  if (!opt_foreground_ && !logfile_name_.empty())
  {
    Dout(dc::notice, "Turning all debug output off!");
    Debug(libcw_do.off());
  }
#endif
}

//virtual
void Sockettapd::command_line_parameters_parsed()
{
  DoutEntering(dc::notice, "Sockettapd::command_line_parameters_parsed()");

  // Make logfile_name_ absolute.
  if (logfile_name_.is_relative())
    logfile_name_ = project_dir_ / logfile_name_;

  Dout(dc::notice, "logfile_name_ is now " << logfile_name_);

  // Switch to the background BEFORE creating any threads!
  if (!opt_foreground_)
    goto_background();
}

//virtual
bool Sockettapd::parse_command_line_parameter(std::string_view arg, int argc, char* argv[], int* index)
{
  DoutEntering(dc::notice, "Sockettapd::parse_command_line_parameter(\"" << arg << "\", " << argc << ", " << debug::print_argv(argv) << ", &" << index << ")");

  if (arg == "--one-shot")
  {
    opt_one_shot_ = true;
    return true;
  }

  if (arg == "--foreground")
  {
    opt_foreground_ = true;
    return true;
  }

  if (arg == "--projectdir" || arg == "--log")
  {
    ++*index;
    if (*index >= argc)
      THROW_ALERT("Missing argument for [ARG]", AIArgs("[ARG]", arg));
    if (arg == "--projectdir")
    {
      project_dir_ = argv[*index];
      Dout(dc::notice, "project_dir_ set to " << project_dir_ << ".");
    }
#ifdef CWDEBUG
    else
    {
      logfile_name_ = argv[*index];
      Dout(dc::notice, "logfile_name_ set to " << logfile_name_ << ".");
    }
#endif
    return true;
  }

  return false;
}

//virtual
void Sockettapd::print_usage_extra(std::ostream& os) const
{
  os << "[--one-shot][--foreground][--projectdir <dirname>][--log <logfile>]";
}

//virtual
std::u8string Sockettapd::application_name() const
{
  return u8"sockettapd";
}

void Sockettapd::goto_background()
{
  DoutEntering(dc::notice, "Sockettapd::goto_background()");

  // Make sure we don't write to a closed ostream.
  Dout(dc::notice, "Turning all debug output off!");
  Debug(libcw_do.off());

  // Closes stdin/stdout/stderr and changes cwd to "/".
  // First argument must be 1 (do not change cwd) for `attach_gdb()` to work).
  if (::daemon(1, 0) == -1)
    THROW_ALERTE("daemon");

#ifdef CWDEBUG
  // We are now running in the background. Either keep debugging off or start writing to a log file.
  if (!logfile_name_.empty())
  {
    logfile_.open(logfile_name_);
    Debug(libcw_do.set_ostream(&logfile_, &logfile_mutex_));
    Debug(libcw_do.on());
    Dout(dc::notice, "Turned all debug output on again!");
  }
#endif
}
