#pragma once

#include <cstdint>
#include <cstring>

struct TextLobLocator
{
  int64_t  offset = 0;
  int32_t  length = 0;
  uint32_t magic  = 0;
};

static constexpr uint32_t TEXT_LOB_MAGIC = 0x54455854U;  // "TEXT"

inline TextLobLocator decode_text_lob_locator(const char *data, int len)
{
  TextLobLocator locator;
  const int      copy_len = len < static_cast<int>(sizeof(locator)) ? len : static_cast<int>(sizeof(locator));
  if (copy_len > 0 && data != nullptr) {
    memcpy(&locator, data, copy_len);
  }
  return locator;
}
