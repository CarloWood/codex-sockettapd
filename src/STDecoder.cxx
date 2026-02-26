#include "sys.h"
#include "STDecoder.h"
#include "debug.h"

void STDecoder::decode(int& allow_deletion_count, evio::MsgBlock&& msg)
{
  // Just print what was received.
  DoutEntering(dc::notice, "STDecoder::decode({" << allow_deletion_count <<
      "}, \"" << buf2str(msg.get_start(), msg.get_size()) << "\") [" << this << ']');

  std::string_view const line = msg.view();
  if (line.starts_with("<config-session>"))
  {
    config_session_decoder_.begin(*this);
    switch_protocol_decoder(config_session_decoder_);
  }
}
