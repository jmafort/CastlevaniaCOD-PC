// Fan translations: a folder in mods beside the game (CV_MODS another mods folder), the one chosen in the launcher or
// the in-game menu (translation in castlevania.toml, the folder's name; translation_packs.h finds them). In one:
//
//   files\...          the game disc's files read in their place, by the same path (files\data\title.dar for
//                      D:\data\title.dar): its pictures and menus, the movies' voices (movie\movie_sound_E.cv2) and the
//                      story scenes' (sound\se\sd*.xwb)
//   ptbr_strings.bin   its texts by the game's text IDs (the string table, data\strdataue.dar's), each drawn in place
//                      of the English one: "CODPTBR1", a u32 count, then for each a u32 ID, a u32 length and its UTF-8
//                      bytes (no zero), all little-endian. The game draws UTF-8 as its own tables have it ("..."), its
//                      font Windows-1252's letters
//
// as Neguchads' port (github.com/Neguchads/Castlevania-COD-Recomp) has its Brazilian Portuguese pack, mods\PTBR-PS2:
// HNNEWGAMES', STR Brasil's, MATTH dubs' and Os Aristocratas do Giz Vermelho's PS2 translation and dub made into the
// Xbox's files from the player's own discs. Its files are the USA release's, so is its catalog: the European release
// plays without it. translation_dub = false leaves the voices out (movie\, sound\): the game's own, the texts
// translated. A pack's textures folder (that port's HD pictures, not the translation's) is not read: HD textures are a
// texture pack's (texpack.cpp). Read at the game's start: another choice takes the next.
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "app/app.h"
#include "common/release.h"
#include "common/translation_packs.h"
#include "core/log.h"
#include "kernel/kernel.h"
#include "kernel/memory.h"

namespace fs = std::filesystem;

std::string g_translation;       // the pack's folder in mods, empty none (game_info.cpp keeps it)
bool g_translation_dub = true;   // its voices with its texts (movie\, sound\)

namespace {

// its files by their path on the disc, lower case with backslashes: the host file
std::unordered_map<std::string, fs::path> g_files;
std::mutex g_served_lock;
std::set<std::string> g_served;  // those read so far, each logged once

// its texts, then in guest memory at the first one drawn (the system pool's: the game's own memory untouched)
std::vector<std::pair<uint32_t, std::string>> g_texts;
std::vector<uint32_t> g_guest;  // by ID, the guest address of its text, 0 none
std::once_flag g_guest_once;
std::atomic<bool> g_active{false};
std::atomic<uint32_t> g_table{0};      // the string table last seen
std::atomic<bool> g_english{false};    // that one the English (only it is translated: the USA release has Japanese too)

std::string Lower(std::string s) {
    for (char& c : s) c = c == '/' ? '\\' : char(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// "CODPTBR1", count, then id, length, bytes; false (and nothing) when it is not exactly that
bool ReadCatalog(const fs::path& path, std::vector<std::pair<uint32_t, std::string>>& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::vector<uint8_t> d{std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
    size_t at = 8;
    auto u32 = [&](uint32_t& v) {
        if (d.size() - at < 4) return false;
        v = uint32_t(d[at]) | uint32_t(d[at + 1]) << 8 | uint32_t(d[at + 2]) << 16 | uint32_t(d[at + 3]) << 24;
        at += 4;
        return true;
    };
    uint32_t count = 0;
    if (d.size() < 12 || std::memcmp(d.data(), "CODPTBR1", 8) != 0 || !u32(count) || count > (d.size() - at) / 8)
        return false;
    std::vector<std::pair<uint32_t, std::string>> texts;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t id = 0, n = 0;
        if (!u32(id) || !u32(n) || n > d.size() - at || id > 0xFFFF) return false;
        std::string s(reinterpret_cast<const char*>(d.data() + at), n);
        if (s.find('\0') != std::string::npos) return false;
        at += n;
        texts.emplace_back(id, std::move(s));
    }
    if (at != d.size()) return false;
    out = std::move(texts);
    return true;
}

// the disc's path ("data\title.dar"): the pack's file there, else empty for the disc's own (kernel::g_disc_file)
std::string DiscFile(const std::string& disc_path) {
    std::string key = Lower(disc_path);
    auto it = g_files.find(key);
    if (it == g_files.end()) return {};
    {
        std::lock_guard<std::mutex> lock(g_served_lock);
        if (g_served.insert(key).second) LOGR("translation: %s from the pack", key.c_str());
    }
    return it->second.string();
}

// the texts copied into guest memory, the first time one is drawn (the kernel is up by then)
void PlaceTexts() {
    uint32_t total = 0, top = 0;
    for (const auto& [id, s] : g_texts) {
        total += uint32_t(s.size()) + 1;
        top = std::max(top, id);
    }
    uint32_t va = total ? mem::AllocSystem(total) : 0;
    if (!va) {
        LOGE("translation: no memory for its %zu texts (%u bytes): drawn in English", g_texts.size(), total);
        g_active = false;
        return;
    }
    g_guest.assign(top + 1, 0);
    uint32_t at = va;
    for (const auto& [id, s] : g_texts) {
        std::memcpy(mem::Base() + at, s.c_str(), s.size() + 1);
        g_guest[id] = at;
        at += uint32_t(s.size()) + 1;
    }
    LOGR("translation: %zu texts placed at 0x%08X (%u bytes)", g_texts.size(), va, total);
}

uint32_t R32(const uint8_t* base, uint32_t va) {
    uint32_t v;
    std::memcpy(&v, base + va, 4);
    return v;
}

}  // namespace

// the game's start, before its files are opened (setup.cpp)
void TranslationInit() {
    if (g_translation.empty()) return;
    fs::path pack = cv::ModsFolder(app::ExeDir()) / g_translation;
    std::error_code ec;
    if (!fs::is_directory(pack, ec)) {
        LOGR("translation: %s is not in %s: the game as it is", g_translation.c_str(), pack.parent_path().string().c_str());
        return;
    }
    if (cv::kEurope) {
        LOGR("translation: %s is made for the USA release: the European one plays as it is", g_translation.c_str());
        return;
    }
    // its files, the voices only with translation_dub
    size_t voices = 0;
    fs::path files = pack / "files";
    for (auto it = fs::recursive_directory_iterator(files, ec); !ec && it != fs::recursive_directory_iterator();
         it.increment(ec)) {
        if (!it->is_regular_file(ec)) continue;
        std::string rel = Lower(fs::relative(it->path(), files, ec).string());
        if (rel.rfind("movie\\", 0) == 0 || rel.rfind("sound\\", 0) == 0) {
            voices++;
            if (!g_translation_dub) continue;
        }
        g_files[rel] = it->path();
    }
    if (!g_files.empty()) kernel::g_disc_file = DiscFile;
    // its texts
    fs::path catalog = pack / cv::kCatalogFile;
    bool texts = false;
    if (fs::is_regular_file(catalog, ec)) {
        texts = ReadCatalog(catalog, g_texts);
        if (!texts) LOGE("translation: %s is not a catalog of texts (CODPTBR1): drawn in English", catalog.string().c_str());
    }
    g_active = texts && !g_texts.empty();
    LOGR("translation: %s, %zu files in place of the disc's (voices %s, %zu of them), %zu texts", g_translation.c_str(),
         g_files.size(), g_translation_dub ? "its own" : "the game's", voices, g_texts.size());
}

// the text the game looked up by its ID (hooks.cpp, CV_TEXT), its address in guest memory: the pack's in place of the
// English one, the table's own otherwise
uint32_t TranslatedText(uint8_t* base, uint32_t id, uint32_t original) {
    if (!g_active.load(std::memory_order_relaxed)) return original;
    // the table the game has now: English when its first text is (a language changed in the options loads another)
    uint32_t table = R32(base, cv::kStringTable);
    if (table != g_table.load(std::memory_order_relaxed)) {
        uint32_t first = table ? R32(base, table + 0x10) : 0;
        static const char kFirst[] = "Garibaldi Temple";
        g_english = first && std::memcmp(base + first, kFirst, sizeof(kFirst)) == 0;
        g_table = table;
        LOGR("translation: the string table at 0x%08X, %s", table, g_english.load() ? "English: translated" : "not English: as it is");
    }
    if (!g_english.load(std::memory_order_relaxed)) return original;
    std::call_once(g_guest_once, PlaceTexts);
    if (!g_active.load(std::memory_order_relaxed) || id >= g_guest.size() || !g_guest[id]) return original;
    return g_guest[id];
}
