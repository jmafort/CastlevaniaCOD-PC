// The game's own messages that named the Xbox, for a PC: "Saving... Please don't turn off your Xbox console." and the
// rest of the save, load and delete messages say "your PC" instead. They are the US release's English messages
// (data\global2ue.dar, plain text) and the European release's in its five languages (global2ee, ef, eg, ei, ep: UTF-8,
// in each the same nine: Continue on its title with no save says there is none on the PC); each is changed in place as the file is read (the SDK's
// kernel::g_file_read), the new words no longer than the old with the rest of their space zeroed, and only where the
// bytes read are the ones this was made from: another release's file of the same name is left as it is.
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <string>

#include "core/log.h"
#include "kernel/kernel.h"

namespace {

struct Message {
    uint32_t at;      // where it starts in the file
    const char* was;  // the release's words
    const char* now;  // the PC's, no longer
};

// every message of data\global2ue.dar with "Xbox" in it
constexpr Message kUE[] = {
    {0x987d8, "Saving...\nPlease don't turn off your Xbox console.",
     "Saving...\nPlease don't turn off your PC."},
    {0x98918, "No game data present on your Xbox.",
     "No game data present on your PC."},
    {0x98aa8, "Failed to load data from your Xbox.",
     "Failed to load data from your PC."},
    {0x98b70, "Loading data from your Xbox.\nPlease don't turn off your Xbox console.",
     "Loading data from your PC.\nPlease don't turn off your PC."},
    {0x98bc0, "Loading...\nPlease don't turn off your Xbox console.",
     "Loading...\nPlease don't turn off your PC."},
    {0x98ca0, "Data loaded from your Xbox.",
     "Data loaded from your PC."},
    {0x98eb8, "Deleting...\nPlease don't turn off your Xbox console.",
     "Deleting...\nPlease don't turn off your PC."},
    {0x98ef8, "Creating new save data...\nPlease don't turn off your Xbox console.",
     "Creating new save data...\nPlease don't turn off your PC."},
    {0x993e0, "Your Xbox doesn't have enough \nfree blocks to save games.\nYou need to free %d more blocks.\nPress A to continue without saving\nor B to free more blocks.",
     "Your PC doesn't have enough \nfree blocks to save games.\nYou need to free %d more blocks.\nPress A to continue without saving\nor B to free more blocks."},
    {0x99b00, "There is no save data on your Xbox.\nCreate new save data?",
     "There is no save data on your PC.\nCreate new save data?"},
    {0x99c30, "Checking...\nPlease don't turn off your Xbox console.",
     "Checking...\nPlease don't turn off your PC."},
};
// the European release's, from its disc: in French "le PC", in German "den PC" ("am PC" where "auf dem PC" would be
// longer), in Italian "il PC" (where "Non spegnere il PC." would be longer, its period left out), in Spanish "el PC"
constexpr Message kEE[] = {  // data\global2ee.dar
    {0xa9478, "Saving...\nPlease don't turn off your Xbox console.",
     "Saving...\nPlease don't turn off your PC."},
    {0xa95a0, "No game data present on your Xbox.",
     "No game data present on your PC."},
    {0xa9808, "Loading data from your Xbox.\nPlease don't turn off your Xbox console.",
     "Loading data from your PC.\nPlease don't turn off your PC."},
    {0xa9858, "Loading...\nPlease don't turn off your Xbox console.",
     "Loading...\nPlease don't turn off your PC."},
    {0xa9938, "Data loaded from your Xbox.",
     "Data loaded from your PC."},
    {0xa9ba8, "Creating new save data...\nPlease don't turn off your Xbox console.",
     "Creating new save data...\nPlease don't turn off your PC."},
    {0xaa058, "Your Xbox doesn't have enough \nfree blocks to save games.\nYou need to free %d more blocks.\nPress A to continue without saving\nor B to free more blocks.",
     "Your PC doesn't have enough \nfree blocks to save games.\nYou need to free %d more blocks.\nPress A to continue without saving\nor B to free more blocks."},
    {0xaa730, "There is no save data on your Xbox.\nCreate new save data?",
     "There is no save data on your PC.\nCreate new save data?"},
    {0xaa850, "Checking...\nPlease don't turn off your Xbox console.",
     "Checking...\nPlease don't turn off your PC."},
};
constexpr Message kEF[] = {  // data\global2ef.dar
    {0xa9e50, "Sauvegarde en cours.\nNe pas \303\251teindre la console Xbox.",
     "Sauvegarde en cours.\nNe pas \303\251teindre le PC."},
    {0xaa018, "Aucun jeu pr\303\251sent sur la console Xbox.",
     "Aucun jeu pr\303\251sent sur le PC."},
    {0xaa390, "Chargement des donn\303\251es depuis la console Xbox.\nNe pas \303\251teindre la console Xbox.",
     "Chargement des donn\303\251es depuis le PC.\nNe pas \303\251teindre le PC."},
    {0xaa3e8, "Chargement en cours...\nNe pas \303\251teindre la console Xbox.",
     "Chargement en cours...\nNe pas \303\251teindre le PC."},
    {0xaa508, "Donn\303\251es charg\303\251es depuis la console Xbox.",
     "Donn\303\251es charg\303\251es depuis le PC."},
    {0xaa830, "Cr\303\251ation de nouvelles donn\303\251es de sauvegarde.\nNe pas \303\251teindre la console Xbox.",
     "Cr\303\251ation de nouvelles donn\303\251es de sauvegarde.\nNe pas \303\251teindre le PC."},
    {0xaaee0, "La Xbox ne comporte pas assez de blocs libres \npour sauvegarder les parties. Vous avez besoin \nde %d blocs libres suppl\303\251mentaires.\nAppuyer sur A pour jouer sans sauvegarder. \nAppuyer sur B pour lib\303\251rer des blocs.",
     "Le PC ne comporte pas assez de blocs libres \npour sauvegarder les parties. Vous avez besoin \nde %d blocs libres suppl\303\251mentaires.\nAppuyer sur A pour jouer sans sauvegarder. \nAppuyer sur B pour lib\303\251rer des blocs."},
    {0xab838, "Pas de donn\303\251es de sauvegarde sur la console Xbox.\nCr\303\251er des donn\303\251es ?",
     "Pas de donn\303\251es de sauvegarde sur le PC.\nCr\303\251er des donn\303\251es ?"},
    {0xab9b8, "V\303\251rification en cours.\nNe pas \303\251teindre la console Xbox.",
     "V\303\251rification en cours.\nNe pas \303\251teindre le PC."},
};
constexpr Message kEG[] = {  // data\global2eg.dar
    {0xa9ae8, "Speichern l\303\244uft \342\200\246\nBitte die Xbox nicht ausschalten.",
     "Speichern l\303\244uft \342\200\246\nBitte den PC nicht ausschalten."},
    {0xa9c30, "Keine Spieldaten auf Xbox gefunden.",
     "Keine Spieldaten am PC gefunden."},
    {0xa9ee0, "Lade Daten von Xbox.\nBitte die Xbox nicht ausschalten.",
     "Lade Daten vom PC.\nBitte den PC nicht ausschalten."},
    {0xa9f18, "Ladevorgang l\303\244uft \342\200\246 \nBitte die Xbox nicht ausschalten.",
     "Ladevorgang l\303\244uft \342\200\246 \nBitte den PC nicht ausschalten."},
    {0xaa038, "Daten von Xbox geladen.",
     "Daten vom PC geladen."},
    {0xaa2f8, "Lege neue Speicherdaten an.\nBitte die Xbox nicht ausschalten.",
     "Lege neue Speicherdaten an.\nBitte den PC nicht ausschalten."},
    {0xaa810, "Die Xbox hat nicht gen\303\274gend freie Bl\303\266cke, um Spiele \nzu speichern. Sie ben\303\266tigen %d weitere freie Bl\303\266cke.\nA dr\303\274cken, um ohne zu speichern weiterzuspielen,\nB dr\303\274cken, um f\303\274r mehr freie Bl\303\266cke zu sorgen.",
     "Der PC hat nicht gen\303\274gend freie Bl\303\266cke, um Spiele \nzu speichern. Sie ben\303\266tigen %d weitere freie Bl\303\266cke.\nA dr\303\274cken, um ohne zu speichern weiterzuspielen,\nB dr\303\274cken, um f\303\274r mehr freie Bl\303\266cke zu sorgen."},
    {0xab068, "Es liegen keine Speicherdaten auf der Xbox vor.\nNeue Speicherdaten erstellen?",
     "Es liegen keine Speicherdaten auf dem PC vor.\nNeue Speicherdaten erstellen?"},
    {0xab198, "Pr\303\274fung l\303\244uft \342\200\246\nBitte die Xbox nicht ausschalten.",
     "Pr\303\274fung l\303\244uft \342\200\246\nBitte den PC nicht ausschalten."},
};
constexpr Message kEI[] = {  // data\global2ei.dar
    {0xa99c0, "Salvataggio in corso\342\200\246 \nNon spegnere Xbox.",
     "Salvataggio in corso\342\200\246\nNon spegnere il PC."},
    {0xa9b08, "Sulla Xbox non sono presenti dati di gioco.",
     "Sul PC non sono presenti dati di gioco."},
    {0xa9da8, "Caricamento dati dalla Xbox.\nNon spegnere Xbox.",
     "Caricamento dati dal PC.\nNon spegnere il PC."},
    {0xa9dd8, "Caricamento in corso...\nNon spegnere Xbox.",
     "Caricamento in corso...\nNon spegnere il PC"},
    {0xa9ed8, "Dati caricati dalla tua Xbox.",
     "Dati caricati dal tuo PC."},
    {0xaa1c0, "Creazione di nuovi dati di salvataggio in corso... \nNon spegnere Xbox.",
     "Creazione di nuovi dati di salvataggio in corso...\nNon spegnere il PC."},
    {0xaa6f8, "Xbox non dispone di sufficienti blocchi liberi \nper salvare. Sono necessari %d ulteriori blocchi liberi.\nPremi A per continuare senza salvare \noppure B per liberare dei blocchi.",
     "Il PC non dispone di sufficienti blocchi liberi\nper salvare. Sono necessari %d ulteriori blocchi liberi.\nPremi A per continuare senza salvare \noppure B per liberare dei blocchi."},
    {0xaaec0, "Non ci sono dati di salvataggio sulla tua Xbox.\nVuoi creare dati di salvataggio nuovi?",
     "Non ci sono dati di salvataggio sul tuo PC.\nVuoi creare dati di salvataggio nuovi?"},
    {0xab010, "Verifica in corso...\nNon spegnere Xbox.",
     "Verifica in corso...\nNon spegnere il PC"},
};
constexpr Message kEP[] = {  // data\global2ep.dar
    {0xa9ef8, "Guardando... \nNo apagar la consola Xbox.",
     "Guardando... \nNo apagar el PC."},
    {0xaa0d0, "No hay datos de partida en la consola Xbox.",
     "No hay datos de partida en el PC."},
    {0xaa458, "Cargando datos de la consola Xbox.\nNo apagar la consola Xbox.",
     "Cargando datos del PC.\nNo apagar el PC."},
    {0xaa498, "Cargando... \nNo apagar la consola Xbox.",
     "Cargando... \nNo apagar el PC."},
    {0xaa598, "Datos cargados de la consola Xbox.",
     "Datos cargados del PC."},
    {0xaa8d0, "Creando nuevos datos guardados...\nNo apagar la consola Xbox.",
     "Creando nuevos datos guardados...\nNo apagar el PC."},
    {0xaaf98, "Tu Xbox no tiene suficientes bloques libres para \nguardar partidas. Se necesitan %d bloques libres m\303\241s.\nPresionar A para continuar sin guardar \n\303\263 B para liberar m\303\241s bloques.",
     "Tu PC no tiene suficientes bloques libres para \nguardar partidas. Se necesitan %d bloques libres m\303\241s.\nPresionar A para continuar sin guardar \n\303\263 B para liberar m\303\241s bloques."},
    {0xab8c0, "No hay datos guardados en la consola Xbox.\n\302\277Crear nuevos datos guardados?",
     "No hay datos guardados en el PC.\n\302\277Crear nuevos datos guardados?"},
    {0xaba58, "Comprobando... \nNo apagar la consola Xbox.",
     "Comprobando... \nNo apagar el PC."},
};

struct File {
    const char* name;
    const Message* messages;
    size_t count;
};
template <size_t N>
constexpr File Of(const char* name, const Message (&m)[N]) {
    return {name, m, N};
}
constexpr File kFiles[] = {Of("global2ue.dar", kUE), Of("global2ee.dar", kEE), Of("global2ef.dar", kEF),
                           Of("global2eg.dar", kEG), Of("global2ei.dar", kEI), Of("global2ep.dar", kEP)};
static_assert([] {
    for (const File& f : kFiles)
        for (size_t i = 0; i < f.count; i++)
            if (std::char_traits<char>::length(f.messages[i].now) > std::char_traits<char>::length(f.messages[i].was))
                return false;
    return true;
}(), "a PC message is longer than the one it replaces");

bool EndsWith(const std::string& s, const char* tail) {
    size_t n = std::strlen(tail);
    if (s.size() < n) return false;
    for (size_t i = 0; i < n; i++)
        if (std::tolower(static_cast<unsigned char>(s[s.size() - n + i])) != tail[i]) return false;
    return true;
}

// the part of each message this read holds (a message can straddle two reads): changed when it is the release's
void Read(const std::string& path, uint64_t offset, uint8_t* data, size_t size) {
    const File* file = nullptr;
    for (const File& f : kFiles)
        if (EndsWith(path, f.name)) file = &f;
    if (!file) return;
    int changed = 0;
    for (size_t k = 0; k < file->count; k++) {
        const Message& m = file->messages[k];
        size_t len = std::strlen(m.was) + 1, now = std::strlen(m.now);
        uint64_t lo = std::max<uint64_t>(offset, m.at), hi = std::min<uint64_t>(offset + size, m.at + len);
        if (lo >= hi) continue;
        size_t from = size_t(lo - m.at), n = size_t(hi - lo);
        uint8_t* p = data + (lo - offset);
        // the release's bytes there (its zero at the end too)
        if (std::memcmp(p, m.was + from, n) != 0) continue;
        for (size_t i = 0; i < n; i++) p[i] = from + i < now ? uint8_t(m.now[from + i]) : 0;
        changed++;
    }
    if (changed) LOGR("pc text: %d of the save messages say \"your PC\" (%s)", changed, path.c_str());
}

}  // namespace

void PcTextInit() { kernel::g_file_read = Read; }
