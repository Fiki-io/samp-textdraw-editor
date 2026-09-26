#include "SampColorParser.h"
#include <cctype>

std::vector<TextSpan> SampColorParser::parse(const std::string& input, uint32_t default_color) {
    std::vector<TextSpan> spans;
    uint32_t current_color = default_color;
    std::string current_text = "";
    
    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == '~' && i + 2 < input.size() && input[i + 2] == '~') {
            char tag = std::tolower(input[i + 1]);
            
            // If we have accumulated text before this tag, push it
            if (!current_text.empty()) {
                spans.push_back({current_text, current_color, false});
                current_text.clear();
            }
            
            if (tag == 'r') {
                current_color = 0xE83B3BFF; // GTA SA Red
            } else if (tag == 'g') {
                current_color = 0x48D848FF; // GTA SA Green
            } else if (tag == 'b') {
                current_color = 0x4D88FFFF; // GTA SA Blue
            } else if (tag == 'w') {
                current_color = 0xFFFFFFFF; // White
            } else if (tag == 'y') {
                current_color = 0xF8E838FF; // GTA SA Yellow
            } else if (tag == 'p') {
                current_color = 0xBA4DFFFF; // GTA SA Purple
            } else if (tag == 'l') {
                current_color = 0x1A1A1AFF; // Black / Dark
            } else if (tag == 's') {
                current_color = default_color; // Standard / Reset to default
            } else if (tag == 'h') {
                // Highlight / brighter shade
                int r = std::min(255, (int)(((current_color >> 24) & 0xFF) * 1.35f + 35));
                int g = std::min(255, (int)(((current_color >> 16) & 0xFF) * 1.35f + 35));
                int b = std::min(255, (int)(((current_color >> 8) & 0xFF) * 1.35f + 35));
                uint32_t a = current_color & 0xFF;
                current_color = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | a;
            } else if (tag == 'n') {
                spans.push_back({"", current_color, true}); // Newline span
            } else if (tag == '<') {
                current_text += "<";
            } else if (tag == '>') {
                current_text += ">";
            } else if (tag == 'u') {
                current_text += "^";
            } else if (tag == 'd') {
                current_text += "v";
            } else {
                // Unknown tag, treat as literal text
                current_text += input.substr(i, 3);
            }
            
            i += 3;
        } else {
            current_text += input[i];
            i++;
        }
    }
    
    if (!current_text.empty()) {
        spans.push_back({current_text, current_color, false});
    }
    
    if (spans.empty()) {
        spans.push_back({"", default_color, false});
    }
    
    return spans;
}

std::string SampColorParser::strip_tags(const std::string& input) {
    std::string result = "";
    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == '~' && i + 2 < input.size() && input[i + 2] == '~') {
            char tag = std::tolower(input[i + 1]);
            if (tag == 'n') {
                result += "\n";
            } else if (tag == '<') {
                result += "<";
            } else if (tag == '>') {
                result += ">";
            } else if (tag == 'u') {
                result += "^";
            } else if (tag == 'd') {
                result += "v";
            }
            i += 3;
        } else {
            result += input[i];
            i++;
        }
    }
    return result;
}

const std::vector<SampTagDef>& SampColorParser::get_available_tags() {
    static const std::vector<SampTagDef> tags = {
        {"~r~", "Red", 0xE83B3BFF},
        {"~g~", "Green", 0x48D848FF},
        {"~b~", "Blue", 0x4D88FFFF},
        {"~y~", "Yellow", 0xF8E838FF},
        {"~w~", "White", 0xFFFFFFFF},
        {"~p~", "Purple", 0xBA4DFFFF},
        {"~l~", "Black", 0x222222FF},
        {"~s~", "Standard", 0xCCCCCCFF},
        {"~h~", "Highlight", 0xFFFFAAFF},
        {"~n~", "New Line", 0x888888FF},
        {"~<~", "Left Arrow", 0x00E5FFFF},
        {"~>~", "Right Arrow", 0x00E5FFFF},
        {"~u~", "Up Arrow", 0x00E5FFFF},
        {"~d~", "Down Arrow", 0x00E5FFFF}
    };
    return tags;
}
