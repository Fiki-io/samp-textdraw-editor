#include "PawnExporter.h"
#include <sstream>
#include <iomanip>

std::string PawnExporter::color_to_samp_hex(uint32_t rgba) {
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << rgba;
    return ss.str();
}

int PawnExporter::color_to_samp_int(uint32_t rgba) {
    return static_cast<int>(rgba);
}

std::string PawnExporter::escape_pawn_string(const std::string& input) {
    std::string out = "";
    for (char c : input) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

std::string PawnExporter::export_pawn(const std::vector<TextDraw>& textdraws, bool wrap_in_functions, bool only_selected) {
    std::stringstream ss;
    
    // Separate into Global and Player Textdraws
    std::vector<const TextDraw*> globals;
    std::vector<const TextDraw*> players;
    
    for (const auto& td : textdraws) {
        if (only_selected && !td.is_selected) continue;
        if (td.is_player) {
            players.push_back(&td);
        } else {
            globals.push_back(&td);
        }
    }
    
    size_t total_exported = globals.size() + players.size();
    
    ss << "// ==========================================================================\n";
    ss << "// SA-MP TextDraw Code Export (Total: " << total_exported << " TextDraws)\n";
    if (only_selected) {
        ss << "// Filter: Selected / Group TextDraws Only\n";
    }
    ss << "// ==========================================================================\n\n";
    
    // Variable Declarations
    if (!globals.empty()) {
        ss << "// Global TextDraws\n";
        for (const auto* td : globals) {
            ss << "new Text:" << td->variable_name << ";\n";
        }
        ss << "\n";
    }
    
    if (!players.empty()) {
        ss << "// Player TextDraws\n";
        for (const auto* td : players) {
            ss << "new PlayerText:" << td->variable_name << "[MAX_PLAYERS];\n";
        }
        ss << "\n";
    }
    
    // Loader Function
    if (wrap_in_functions && !globals.empty()) {
        ss << "public OnGameModeInit()\n{\n";
    }
    
    for (const auto* td : globals) {
        std::string var = td->variable_name;
        ss << "    " << var << " = TextDrawCreate(" << std::fixed << std::setprecision(6)
           << td->x << ", " << td->y << ", \"" << escape_pawn_string(td->text) << "\");\n";
        ss << "    TextDrawLetterSize(" << var << ", " << td->letter_width << ", " << td->letter_height << ");\n";
        ss << "    TextDrawTextSize(" << var << ", " << td->text_width << ", " << td->text_height << ");\n";
        ss << "    TextDrawAlignment(" << var << ", " << static_cast<int>(td->alignment) << ");\n";
        ss << "    TextDrawColor(" << var << ", " << color_to_samp_hex(td->color) << ");\n";
        ss << "    TextDrawUseBox(" << var << ", " << (td->use_box ? 1 : 0) << ");\n";
        ss << "    TextDrawBoxColor(" << var << ", " << color_to_samp_hex(td->box_color) << ");\n";
        ss << "    TextDrawSetShadow(" << var << ", " << td->shadow << ");\n";
        ss << "    TextDrawSetOutline(" << var << ", " << td->outline << ");\n";
        ss << "    TextDrawBackgroundColor(" << var << ", " << color_to_samp_hex(td->background_color) << ");\n";
        ss << "    TextDrawFont(" << var << ", " << td->font << ");\n";
        ss << "    TextDrawSetProportional(" << var << ", " << (td->proportional ? 1 : 0) << ");\n";
        ss << "    TextDrawSetSelectable(" << var << ", " << (td->selectable ? 1 : 0) << ");\n";
        
        if (td->font == 5) {
            ss << "    TextDrawSetPreviewModel(" << var << ", " << td->preview_model << ");\n";
            ss << "    TextDrawSetPreviewRot(" << var << ", " << td->rot_x << ", " << td->rot_y << ", " << td->rot_z << ", " << td->zoom << ");\n";
            ss << "    TextDrawSetPreviewVehCol(" << var << ", " << td->veh_color1 << ", " << td->veh_color2 << ");\n";
        }
        ss << "\n";
    }
    
    if (wrap_in_functions && !globals.empty()) {
        ss << "    return 1;\n}\n\n";
    }
    
    // Player Textdraws Creation Function
    if (!players.empty()) {
        if (wrap_in_functions) {
            ss << "public OnPlayerConnect(playerid)\n{\n";
        }
        for (const auto* td : players) {
            std::string var = td->variable_name + (wrap_in_functions ? "[playerid]" : "");
            ss << "    " << var << " = CreatePlayerTextDraw(playerid, " << std::fixed << std::setprecision(6)
               << td->x << ", " << td->y << ", \"" << escape_pawn_string(td->text) << "\");\n";
            ss << "    PlayerTextDrawLetterSize(playerid, " << var << ", " << td->letter_width << ", " << td->letter_height << ");\n";
            ss << "    PlayerTextDrawTextSize(playerid, " << var << ", " << td->text_width << ", " << td->text_height << ");\n";
            ss << "    PlayerTextDrawAlignment(playerid, " << var << ", " << static_cast<int>(td->alignment) << ");\n";
            ss << "    PlayerTextDrawColor(playerid, " << var << ", " << color_to_samp_hex(td->color) << ");\n";
            ss << "    PlayerTextDrawUseBox(playerid, " << var << ", " << (td->use_box ? 1 : 0) << ");\n";
            ss << "    PlayerTextDrawBoxColor(playerid, " << var << ", " << color_to_samp_hex(td->box_color) << ");\n";
            ss << "    PlayerTextDrawSetShadow(playerid, " << var << ", " << td->shadow << ");\n";
            ss << "    PlayerTextDrawSetOutline(playerid, " << var << ", " << td->outline << ");\n";
            ss << "    PlayerTextDrawBackgroundColor(playerid, " << var << ", " << color_to_samp_hex(td->background_color) << ");\n";
            ss << "    PlayerTextDrawFont(playerid, " << var << ", " << td->font << ");\n";
            ss << "    PlayerTextDrawSetProportional(playerid, " << var << ", " << (td->proportional ? 1 : 0) << ");\n";
            ss << "    PlayerTextDrawSetSelectable(playerid, " << var << ", " << (td->selectable ? 1 : 0) << ");\n";
            
            if (td->font == 5) {
                ss << "    PlayerTextDrawSetPreviewModel(playerid, " << var << ", " << td->preview_model << ");\n";
                ss << "    PlayerTextDrawSetPreviewRot(playerid, " << var << ", " << td->rot_x << ", " << td->rot_y << ", " << td->rot_z << ", " << td->zoom << ");\n";
                ss << "    PlayerTextDrawSetPreviewVehCol(playerid, " << var << ", " << td->veh_color1 << ", " << td->veh_color2 << ");\n";
            }
            ss << "\n";
        }
        ss << "    return 1;\n}\n\n";
    }
    
    // Show / Hide helper functions
    if (wrap_in_functions && (!globals.empty() || !players.empty())) {
        ss << "// Helper function to show all textdraws to player\n";
        ss << "stock ShowTextdrawsForPlayer(playerid)\n{\n";
        for (const auto* td : globals) {
            ss << "    TextDrawShowForPlayer(playerid, " << td->variable_name << ");\n";
        }
        for (const auto* td : players) {
            ss << "    PlayerTextDrawShow(playerid, " << td->variable_name << "[playerid]);\n";
        }
        ss << "}\n\n";
        
        ss << "// Helper function to hide all textdraws from player\n";
        ss << "stock HideTextdrawsForPlayer(playerid)\n{\n";
        for (const auto* td : globals) {
            ss << "    TextDrawHideForPlayer(playerid, " << td->variable_name << ");\n";
        }
        for (const auto* td : players) {
            ss << "    PlayerTextDrawHide(playerid, " << td->variable_name << "[playerid]);\n";
        }
        ss << "}\n";
    }
    
    return ss.str();
}
