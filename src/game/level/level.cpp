#include "level.hpp"
#include "game/dash.hpp"
#include "game/objects/entities/player.hpp"
#include "game/objects/world/platform.hpp"
#include "game/objects/world/solid.hpp"
#include "game/objects/world/spike.hpp"
#include "game/objects/world/static.hpp"
#include "game/objects/world/finish.hpp"
#include "game/objects/world/start.hpp"
#include "game/objects/world/particle-emitter.hpp"
#include "game/objects/world/text.hpp"
#include "core/components/particles.hpp"
#include "game/objects/world/image-sprite.hpp"
#include "game/objects/world/invisible-sprite.hpp"
#include "game/objects/world/trigger.hpp"
#include "core/math.hpp"
#include "core/physics/shape.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

constexpr float DEATH_ANIMATION_DURATION = 1.0f; // in seconds
constexpr float DEATH_ZOOM_MULTIPLIER = 1.12f;
constexpr float DEATH_ROTATION_OFFSET = 5.0f;
constexpr float FINISH_DISTANCE = 2.0F * PHYSICS_PIXELS_PER_METER;
constexpr float PLATFORM_FINISH_DISTANCE = 8.0F * PHYSICS_PIXELS_PER_METER;

DashLevel::DashLevel(Dash& game) : Entity(EntityType::LEVEL), m_game(game), m_camera(*this) {}

Entity* DashLevel::find_object(int id) const {
    for (auto* object : m_objects) {
        if (object->id() == id) return object;
    }
    return nullptr;
}

DashLevel::~DashLevel() {
    unload();
}

void DashLevel::set_best_progress(int progress) {
    m_best_progress = std::max(m_best_progress, std::clamp(progress, 0, 100));
}

void DashLevel::set_attempts(int attempts) {
    m_attempts = std::max(attempts, 0);
}

void DashLevel::record_attempt() {
    if (m_attempts < std::numeric_limits<int>::max()) {
        ++m_attempts;
    }
}

void DashLevel::set_state(LevelState state) {
    m_state = state;

    if (m_automatic_finish != nullptr) {
        m_automatic_finish->set_visible(state != LevelState::EDITING && m_automatic_finish_active);
    }

    if (!m_music_loaded) return;

    if (state == LevelState::PLAYING || state == LevelState::DEATH) {
        ResumeMusicStream(m_music);
    } else {
        PauseMusicStream(m_music);
    }
}

void DashLevel::update() {
    if (m_state == LevelState::EDITING) {
        return;
    }

    if (m_player == nullptr || m_finished) {
        return;
    }

    if (m_state == LevelState::DEATH) {
        update_death_animation();
        return;
    }

    if (m_state != LevelState::PLAYING) {
        return;
    }

    if (m_music_loaded) {
        UpdateMusicStream(m_music);
    }

    const Vector2 player_position = m_player->body().get_position();
    if (m_finish_requested || (m_automatic_finish_active && player_position.x >= m_level_end.x)) {
        m_game.finish_level();
        return;
    }

    const float distance = m_level_end.x - m_level_start.x;
    m_current_progress =
        distance > 0.0F ? std::clamp((player_position.x - m_level_start.x) / distance * 100.0F, 0.0F, 99.0F) : 0.0F;
}

void DashLevel::request_finish(GameObject& visitor) {
    if (&visitor == m_player && m_state == LevelState::PLAYING) {
        m_finish_requested = true;
    }
}

void DashLevel::prepare_start() {
    m_level_start = {};
    float first_x = std::numeric_limits<float>::infinity();

    for (const auto* object : m_objects) {
        const auto type = object->type();
        if (type != EntityType::START && type != EntityType::PLATFORM && type != EntityType::SOLID) {
            continue;
        }

        const auto& body = *object->get_component<RigidBody>();
        if (type == EntityType::START) {
            m_level_start = body.get_position();
            return;
        }

        if (!body.collision_enabled()) {
            continue;
        }

        const Rectangle bounds = body.get_world_bounding_box();
        if (bounds.x < first_x) {
            first_x = bounds.x;
            m_level_start = {bounds.x + Player::SIZE.x * 0.5F, bounds.y - Player::SIZE.y};
        }
    }
}

void DashLevel::prepare_finish() {
    m_level_end = m_level_start;
    m_automatic_finish_active = false;

    if (m_automatic_finish != nullptr) {
        m_automatic_finish->set_visible(false);
    }

    bool last_is_platform = false;
    float previous_end = m_level_end.x;
    for (const auto* object : m_objects) {
        if (object->type() == EntityType::FINISH) {
            const Rectangle bounds = object->get_component<RigidBody>()->get_world_bounding_box();
            m_level_end = {bounds.x, bounds.y + bounds.height * 0.5F};
            return;
        }

        const auto* image = object->get_component<ImageSprite>();
        const auto* body = object->get_component<RigidBody>();

        if (body == nullptr || (image != nullptr && image->space() == ImageSpace::Static) ||
            object->get_component<InvisibleSprite>() != nullptr)
            continue;

        const Rectangle bounds = body->get_world_bounding_box();
        const float edge = bounds.x + bounds.width;

        // track the two rightmost edges.
        previous_end = std::max(previous_end, std::min(m_level_end.x, edge));
        if (edge >= m_level_end.x) {
            m_level_end = {edge, bounds.y + bounds.height * 0.5F};
            last_is_platform = object->type() == EntityType::PLATFORM;
        }
    }

    m_level_end.x =
        last_is_platform ? std::min(m_level_end.x, previous_end + PLATFORM_FINISH_DISTANCE) : m_level_end.x + FINISH_DISTANCE;

    // automatic finishes only check x.
    if (m_automatic_finish == nullptr) {
        m_automatic_finish = &m_game.add_object<Finish>(m_game.world(), *this);
        m_automatic_finish->get_component<RigidBody>()->set_collision_enabled(false);
    }

    auto& body = *m_automatic_finish->get_component<RigidBody>();
    body.set_position(m_level_end.x, m_level_end.y - body.get_size().y * 0.5F);
    m_automatic_finish_active = true;
    m_automatic_finish->set_visible(m_state != LevelState::EDITING);
}

float DashLevel::death_animation_progress() const {
    return d_easing::out_quad(m_death_elapsed / DEATH_ANIMATION_DURATION);
}

void DashLevel::update_behaviours(float frametime) {
    if (m_state != LevelState::PLAYING) {
        return;
    }

    for (auto* object : m_objects) {
        object->update_behaviours(frametime);
    }

    if (m_automatic_finish_active && m_automatic_finish != nullptr) {
        m_automatic_finish->update_behaviours(frametime);
    }
}

void DashLevel::clear_objects() {
    m_game.remove_object(m_player);
    m_player = nullptr;
    m_game.remove_object(m_automatic_finish);
    m_automatic_finish = nullptr;
    m_automatic_finish_active = false;

    for (auto* object : m_objects) {
        m_game.remove_object(object);
    }

    m_objects.clear();
}

void DashLevel::unload() {
    m_camera.clear();
    unload_music();
    clear_objects();
    m_level_start = {};
    m_level_end = {0, 0};
    m_current_progress = 0.0f;
    m_finished = false;
    m_finish_requested = false;
    m_state = LevelState::INVALID;
}

void DashLevel::reset() {
    m_camera.clear();
    m_current_progress = 0.0f;
    m_finished = false;
    m_finish_requested = false;
    m_finished_death_animation = false;
    m_death_elapsed = 0.0f;
    set_music_pitch(1.0f);
    prepare_start();

    if (m_state != LevelState::EDITING) {
        prepare_finish();
        for (auto* object : m_objects) {
            object->reset_behaviours();
            if (auto* particles = object->get_component<Particles>()) particles->clear();
        }
        if (m_automatic_finish != nullptr) {
            m_automatic_finish->reset_behaviours();
        }
    }
}

bool DashLevel::create(const LevelMetadata& metadata) {
    if (!m_file.empty() || metadata.name.find_first_not_of(" \t\r\n") == std::string::npos) return false;

    const std::string id = metadata.name + "_" + metadata.creator;
    if (!fs::is_regular_file(metadata.music.path())) {
        return false;
    }

    fs::create_directories(LEVELS_LOCATION);

    const fs::path directory = LEVELS_LOCATION / id;
    if (!fs::create_directory(directory)) {
        std::cout << "[level] cannot create level\nreason: unknown\n";
        return false;
    }

    m_file = directory / "level.json";
    m_metadata = metadata;
    m_metadata.music = DashFile(directory / ("audio" + metadata.music.path().extension().string()));

    auto& platform = create_object(EntityType::PLATFORM);
    auto* body = platform.get_component<RigidBody>();
    body->set_shape(make_box_shape(1024.0F, 64.0F));
    body->set_position(0.0F, 100.0F);

    const bool created = m_metadata.music.update(metadata.music.path()) && save();
    unload();

    if (!created) {
        fs::remove_all(directory);
        m_file.clear();
    }

    return created;
}

bool DashLevel::remove() {
    if (m_file.empty()) return false;

    std::error_code error;
    fs::remove_all(m_file.parent_path(), error);
    if (error) {
        std::cout << "[level] failed to remove " << m_file << ": " << error.message() << '\n';
        return false;
    }

    unload();
    m_file.clear();
    return true;
}

bool DashLevel::load(const fs::path& location) {
    std::cout << "[level] loading level from " << location << "\n";

    try {
        const auto absolute_location = fs::absolute(location);
        load_from_file(absolute_location);
        m_file = absolute_location;
    } catch (const std::exception& error) {
        std::cout << "[level] invalid level metadata in " << location << ": " << error.what() << "\n";
        return false;
    }

    return true;
}

bool DashLevel::load_objects() {
    clear_objects();
    m_level_end = {};
    m_loading_objects = true;

    try {
        load_from_file(m_file);
    } catch (const std::exception& error) {
        m_loading_objects = false;
        std::cout << "[level] invalid object: " << error.what() << "\n";
        unload();
        return false;
    }

    m_loading_objects = false;
    return true;
}

void DashLevel::serialize(Serializer& serializer) const {
    serializer.field("name", m_metadata.name);
    serializer.field("creator", m_metadata.creator);
    serializer.field("description", m_metadata.description);
    serializer.field("music_file", m_metadata.music.path().filename().string());
    serializer.field("initial_type", static_cast<int>(m_metadata.initial_type));
    serializer.field("solid_background", m_solid_background);

    serializer.begin_array("objects");

    for (const auto* object : m_objects) {
        serializer.begin_element();
        serializer.field("type", static_cast<int>(object->type()));
        object->serialize(serializer);
        serializer.end_element();
    }

    serializer.end_array();
}

void DashLevel::deserialize(Deserializer& deserializer, const fs::path& directory) {
    std::string music_file;
    if (!deserializer.field("name", m_metadata.name) || !deserializer.field("music_file", music_file)) {
        throw std::invalid_argument("level metadata is incomplete");
    }

    m_metadata.music = DashFile(directory / fs::path(music_file).filename());

    m_metadata.creator.clear();
    deserializer.field("creator", m_metadata.creator);

    m_metadata.description.clear();
    deserializer.field("description", m_metadata.description);

    int initial_type = static_cast<int>(PlayerType::BOX);
    deserializer.field("initial_type", initial_type);
    m_metadata.initial_type = static_cast<PlayerType>(initial_type);
    if (m_metadata.initial_type != PlayerType::BOX && m_metadata.initial_type != PlayerType::BIRD) {
        throw std::invalid_argument("invalid initial player type");
    }

    m_solid_background = BLACK;
    deserializer.field("solid_background", m_solid_background);

    if (!m_loading_objects) {
        return;
    }

    const std::size_t count = deserializer.array_size("objects");

    m_objects.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        deserializer.begin_element("objects", index);

        int raw_type = 0;
        if (!deserializer.field("type", raw_type)) {
            throw std::invalid_argument("level object type is missing");
        }

        const auto type = static_cast<EntityType>(raw_type);
        if (type != EntityType::PLATFORM && type != EntityType::SOLID && type != EntityType::SPIKE &&
            type != EntityType::STATIC_TEXTURE && type != EntityType::TRIGGER && type != EntityType::FINISH &&
            type != EntityType::START && type != EntityType::PARTICLES && type != EntityType::TEXT) {
            std::cout << "[level] unsupported object type: " << raw_type << "\n";
            deserializer.end_element();
            continue;
        }

        Entity* object = &create_object(type);
        object->deserialize(deserializer, directory);

        deserializer.end_element();
    }

    prepare_start();
    if (m_state != LevelState::EDITING) {
        prepare_finish();
    }
}

Entity& DashLevel::instantiate_object(EntityType type) {
    Entity* object = nullptr;
    switch (type) {
        case EntityType::PLATFORM:
            object = &m_game.add_object<Platform>(m_game.world());
            break;
        case EntityType::SOLID:
            object = &m_game.add_object<Solid>(m_game.world());
            break;
        case EntityType::SPIKE:
            object = &m_game.add_object<Spike>(m_game.world());
            break;
        case EntityType::STATIC_TEXTURE:
            object = &m_game.add_object<StaticTexture>(m_game.world());
            break;
        case EntityType::TRIGGER:
            object = &m_game.add_object<Trigger>(m_game.world(), *this);
            break;
        case EntityType::FINISH:
            object = &m_game.add_object<Finish>(m_game.world(), *this);
            break;
        case EntityType::START:
            object = &m_game.add_object<Start>(m_game.world(), *this);
            break;
        case EntityType::PARTICLES:
            object = &m_game.add_object<ParticleEmitter>(m_game.world(), *this);
            break;
        case EntityType::TEXT:
            object = &m_game.add_object<Text>(m_game.world());
            break;
        default:
            throw std::invalid_argument("unsupported level object type");
    }

    object->set_level(*this);
    return *object;
}

void DashLevel::add_object(Entity& object) {
    m_objects.push_back(&object);
}

Entity& DashLevel::create_object(EntityType type) {
    auto& object = instantiate_object(type);
    add_object(object);
    return object;
}

void DashLevel::remove_object(Entity* object) {
    std::erase(m_objects, object);
    m_game.remove_object(object);
}

bool DashLevel::save() {
    std::cout << "[level] saving level at " << m_file << "\n";

    try {
        return save_to_file(m_file);
    } catch (const std::exception& error) {
        std::cout << "[level] failed to save " << m_file << ": " << error.what() << "\n";
        return false;
    }
}

void DashLevel::begin_death() {
    m_death_elapsed = 0.0f;
    m_death_start_volume = m_music_volume;
    m_finished_death_animation = false;
    set_state(LevelState::DEATH);
}

void DashLevel::spawn_player(bool god_mode, bool free_mode) {
    if (m_state == LevelState::EDITING) return;

    if (m_player == nullptr) {
        m_player = &m_game.add_object<Player>(m_game, m_metadata.initial_type, god_mode);
        m_player->set_level(*this);
    } else {
        m_player->reset();
        m_player->set_god_mode(god_mode);
        m_player->update_player_type(m_metadata.initial_type);
    }

    m_player->set_free_mode(free_mode);
    m_player->body().set_position(m_level_start.x, m_level_start.y);
}

void DashLevel::update_death_animation() {
    if (m_finished_death_animation) {
        return;
    }

    m_death_elapsed = std::min(m_death_elapsed + GetFrameTime(), DEATH_ANIMATION_DURATION);
    const float progress = death_animation_progress();

    if (m_music_loaded) {
        UpdateMusicStream(m_music);
        set_music_pitch(d_math::lerp(1.0f, 0.0f, progress));
        set_music_volume(std::min(m_music_volume, d_math::lerp(m_death_start_volume, 0.0f, progress)));
    }

    CameraView* camera = m_game.camera_for(*this);
    if (camera != nullptr) {
        camera->view().zoom = d_math::lerp(DEFAULT_CAMERA_ZOOM, DEFAULT_CAMERA_ZOOM * DEATH_ZOOM_MULTIPLIER, progress);
        camera->view().rotation =
            d_math::lerp(DEFAULT_CAMERA_ROTATION, DEFAULT_CAMERA_ROTATION + DEATH_ROTATION_OFFSET, progress);
    }

    if (m_death_elapsed >= DEATH_ANIMATION_DURATION) {
        m_finished_death_animation = true;
        m_player->kill();
        if (m_music_loaded) {
            StopMusicStream(m_music);
        }
    }
}

bool DashLevel::load_music() {
    unload_music();

    const auto location = m_metadata.music.path();
    m_music = LoadMusicStream(location.c_str());
    m_music_loaded = IsMusicValid(m_music);
    if (!m_music_loaded) {
        std::cout << "[level] failed to load music from " << location << '\n';
        return false;
    }

    SetMusicPitch(m_music, m_music_pitch);
    SetMusicVolume(m_music, m_music_volume);
    return true;
}

void DashLevel::play_music() {
    if (!m_music_loaded) return;

    StopMusicStream(m_music);
    PlayMusicStream(m_music);
}

void DashLevel::unload_music() {
    if (!m_music_loaded) return;

    StopMusicStream(m_music);
    UnloadMusicStream(m_music);
    m_music = {};
    m_music_loaded = false;
}

void DashLevel::set_music_pitch(float pitch) {
    if (pitch <= 0.0f) return;

    m_music_pitch = pitch;
    if (m_music_loaded) SetMusicPitch(m_music, pitch);
}

void DashLevel::set_music_volume(float volume) {
    m_music_volume = std::clamp(volume, 0.0f, 1.0f);
    if (m_music_loaded) SetMusicVolume(m_music, m_music_volume);
}
