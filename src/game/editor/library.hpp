#pragma once

#include "game/level/level.hpp"

struct LibraryEntry {
    const char* m_name;
    const char* m_icon;

    Entity& (*m_setup)(DashLevel& level);
};

inline constexpr LibraryEntry EDITOR_LIBRARY[] = {
    {"platform", "platform.svg", [](DashLevel& level) -> Entity& { return level.instantiate_object(EntityType::PLATFORM); }},
    {"solid", "solid.svg", [](DashLevel& level) -> Entity& { return level.instantiate_object(EntityType::SOLID); }},
    {"spike", "spike.svg", [](DashLevel& level) -> Entity& { return level.instantiate_object(EntityType::SPIKE); }},
    {"image", "image.svg", [](DashLevel& level) -> Entity& { return level.instantiate_object(EntityType::STATIC_TEXTURE); }},
    {"trigger", "trigger.svg", [](DashLevel& level) -> Entity& { return level.instantiate_object(EntityType::TRIGGER); }},
    {"start", "start.svg", [](DashLevel& level) -> Entity& { return level.instantiate_object(EntityType::START); }},
    {"finish", "finish.svg", [](DashLevel& level) -> Entity& { return level.instantiate_object(EntityType::FINISH); }},
    {"particles", "particles.svg", [](DashLevel& level) -> Entity& { return level.instantiate_object(EntityType::PARTICLES); }},
    {"text", "text.svg", [](DashLevel& level) -> Entity& { return level.instantiate_object(EntityType::TEXT); }},
};
