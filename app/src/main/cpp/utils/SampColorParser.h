#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct TextSpan {
    std::string text;
    uint32_t color; // RGBA
    bool is_newline = false;
};

class SampColorParser {
public:
    static std::vector<TextSpan> parse(const std::string& input, uint32_t default_color);
    static std::string strip_tags(const std::string& input);
};
