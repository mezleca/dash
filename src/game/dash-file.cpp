#include "dash-file.hpp"

#include <iostream>

DashFile::DashFile(const fs::path& original) {
    if (!original.empty()) {
        m_path = fs::absolute(original);
    }
}

bool DashFile::update(const fs::path& source) {
    if (m_path.empty() || source.empty()) {
        return false;
    }

    try {
        if (!fs::is_regular_file(source)) {
            return false;
        }

        if (fs::exists(m_path) && fs::equivalent(source, m_path)) {
            return true;
        }

        fs::copy_file(source, m_path, fs::copy_options::overwrite_existing);
        return true;
    } catch (const fs::filesystem_error& error) {
        std::cout << "[file] failed to copy " << source << " to " << m_path << ": " << error.what() << '\n';
        return false;
    }
}
