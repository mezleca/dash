#pragma once

#include <filesystem>
#include <raylib.h>

namespace fs = std::filesystem;

inline const fs::path RESOURCES_LOCATION = fs::path(GetApplicationDirectory()) / "resources";
inline const fs::path LEVELS_LOCATION = RESOURCES_LOCATION / "levels";
