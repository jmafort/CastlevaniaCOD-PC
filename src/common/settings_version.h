// castlevania.toml's settings_version, which the launcher and the game both write: what a file saved by an older release
// holds that its next start changes, once. The launcher and the game write every setting they know into the file, the
// defaults too, so a default changed in a release reaches only a new install unless the file says how old it is.
//   1 (1.9.3): ray_tracing_quality back to 3, path traced. 1.8.10 to 1.9.1 offered it as a choice, 2 (High) the default,
//     and saved it; 1.9.2 made Ray tracing the path traced lighting and took the choice from the menus, the key in the
//     file alone (a handheld's 0 by hand), so a player's old choice or old default held them at the lighting before it.
// A file older than kSettingsVersion (none: a release before 1.9.3) is migrated and saved at once by whichever starts
// first, the launcher or the game, so a value set by hand after it is kept. No file, no migration: a new install's
// defaults are the newest.
#pragma once

#include <cstdint>

namespace cv {

constexpr uint32_t kSettingsVersion = 1;
constexpr uint32_t kPathTraced = 3;  // ray_tracing_quality's default

// the values read from a file saved at version (0 for none) brought to kSettingsVersion's
inline void MigrateSettings(uint32_t version, uint32_t& rt_quality) {
    if (version < 1) rt_quality = kPathTraced;
}

}  // namespace cv
