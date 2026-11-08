// The fan translations a player has, each a folder in mods beside the game (translation.cpp has what is in one): the
// launcher lists them and the game plays the one chosen (translation in castlevania.toml, the folder's name).
#pragma once

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace cv {

// a pack's texts by the game's text IDs (translation.cpp)
inline constexpr const char* kCatalogFile = "ptbr_strings.bin";

// the folders a translation is known by, and what the menus call it: in its own language (the launcher, which writes
// every language) and in English (the in-game menu)
struct KnownTranslation {
    const char* folder;
    const char* name;
    const char* english;
};
inline constexpr KnownTranslation kKnownTranslations[] = {
    {"PTBR-PS2", "Português (Brasil)", "Brazilian Portuguese"},  // Neguchads' port's pack, the PS2's fan translation
};

struct TranslationPack {
    std::string folder;   // its folder's name in mods, what castlevania.toml keeps
    std::string name;     // in its own language, else the folder's name
    std::string english;  // in English, else the folder's name
    bool texts = false;   // a catalog of texts (kCatalogFile)
    bool files = false;   // a files folder, the disc's files in their place
};

// mods beside the game; CV_MODS another (tools/cvtest)
inline std::filesystem::path ModsFolder(const std::filesystem::path& exe_dir) {
    if (const char* env = std::getenv("CV_MODS"); env && *env) return env;
    return exe_dir / "mods";
}

// each folder in mods with texts or files, by name
inline std::vector<TranslationPack> TranslationPacks(const std::filesystem::path& mods) {
    namespace fs = std::filesystem;
    std::vector<TranslationPack> out;
    std::error_code ec;
    for (const fs::directory_entry& e : fs::directory_iterator(mods, ec)) {
        if (!e.is_directory(ec)) continue;
        TranslationPack p;
        p.folder = e.path().filename().string();
        p.name = p.english = p.folder;
        for (const KnownTranslation& k : kKnownTranslations)
            if (p.folder == k.folder) {
                p.name = k.name;
                p.english = k.english;
            }
        p.texts = fs::is_regular_file(e.path() / kCatalogFile, ec);
        p.files = fs::is_directory(e.path() / "files", ec);
        if (p.texts || p.files) out.push_back(std::move(p));
    }
    std::sort(out.begin(), out.end(), [](const TranslationPack& a, const TranslationPack& b) { return a.folder < b.folder; });
    return out;
}

}  // namespace cv
