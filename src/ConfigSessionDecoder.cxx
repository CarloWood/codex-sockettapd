#include "sys.h"
#include "STDecoder.h"
#include "ConfigSessionDecoder.h"
#include "Application.h"
#include "utils/AIAlert.h"
#include "debug.h"

void ConfigSessionDecoder::decode(int& allow_deletion_count, evio::MsgBlock&& msg)
{
  DoutEntering(dc::notice, "ConfigSessionDecoder::decode({" << allow_deletion_count <<
      "}, \"" << buf2str(msg.get_start(), msg.get_size()) << "\") [" << this << ']');

  constexpr std::string_view session_id_open = "<session-id>";
  constexpr std::string_view session_id_close = "</session-id>";

  std::string_view const line = msg.view();

  if (!have_thread_id_)
  {
    auto const open_pos = line.find(session_id_open);
    if (open_pos != std::string_view::npos)
    {
      auto const value_pos = open_pos + session_id_open.size();
      auto const close_pos = line.find(session_id_close, value_pos);
      if (close_pos != std::string_view::npos && close_pos > value_pos)
      {
        std::string_view const uuid_sv = line.substr(value_pos, close_pos - value_pos);
        thread_id_.assign_from_json_string(uuid_sv);
        have_thread_id_ = true;
      }
      return;
    }
  }

  if (line == "</config-session>\n")
  {
    if (!have_thread_id_)
      THROW_LALERT("Received </config-session> without <session-id> block!");

    // Call begin(return_decoder) before passing ConfigSessionDecoder the to switch_protocol_decoder.
    ASSERT(return_decoder_);
    switch_protocol_decoder(*return_decoder_);

    // Pass the decoded Thread ID back to STDecoder.
    return_decoder_->thread_id_received(thread_id_);
  }
}
