#pragma once

#include "UUID.h"
#include "evio/protocol/Decoder.h"
#include "evio/StreamBuf.h"

class ConfigSessionDecoder : public evio::protocol::Decoder
{
 private:
  UUID thread_id_;
  evio::Sink* return_decoder_ = nullptr;
  bool have_thread_id_ = false;

 public:
  void begin(evio::Sink& return_decoder)
  {
    return_decoder_ = &return_decoder;
    have_thread_id_ = false;
  }

 protected:
  void decode(int& allow_deletion_count, evio::MsgBlock&& msg) override;
};
