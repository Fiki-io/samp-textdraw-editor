#pragma once

#include "TextDraw.h"
#include <vector>
#include <string>

class PawnExporter {
public:
    static std::string export_pawn(const std::vector<TextDraw>& textdraws, bool wrap_in_functions = true);
    static std::string color_to_samp_hex(uint32_t rgba);
    static int color_to_samp_int(uint32_t rgba);
};
