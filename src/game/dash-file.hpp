#pragma once

#include "paths.hpp"

class DashFile {
public:
    DashFile() = default;
    explicit DashFile(const fs::path& original);

    bool update(const fs::path& source);

    const fs::path& path() const {
        return m_path;
    }

private:
    fs::path m_path;
};
