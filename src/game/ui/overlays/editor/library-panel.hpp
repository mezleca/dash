#pragma once

#include <imgui-ui/layout/container.hpp>

class Dash;

class LibraryPanel : public ui::Container {
public:
    explicit LibraryPanel(Dash& game);
};
