#pragma once
#include <string>

// Returns a multi-line string rendering `text` as a 5-row block-character banner.
// Supports all printable ASCII (0x20–0x7E); other bytes render as spaces.
std::string figlet(const std::string& text);
