#include "sys.h"
#include "utils.h"

std::string utf8_to_string(std::u8string const& text)
{
  return std::string(reinterpret_cast<char const*>(text.data()), text.size());
}
