#pragma once

#include "game/objects/entities/entity.hpp"
#include "game/objects/entities/player-type.hpp"
#include "game/paths.hpp"
#include "game/dash-file.hpp"
#include "camera.hpp"

#include <string>
#include <vector>

enum class LevelState : int8_t {
    INVALID = -1,
    PLAYING,
    EDITING,
    PAUSED,
    DEATH
};

class Player;
class Dash;
class Finish;

struct LevelMetadata {
    std::string name;
    std::string creator;
    std::string description;
    DashFile music;
    PlayerType initial_type = PlayerType::BOX;
};

class DashLevel : public Entity {
public:
    explicit DashLevel(Dash& game);
    ~DashLevel();

    const std::string& name() const {
        return m_metadata.name;
    }

    LevelMetadata& metadata() {
        return m_metadata;
    }

    const LevelMetadata& metadata() const {
        return m_metadata;
    }

    Color solid_background() const {
        return m_solid_background;
    }

    void set_solid_background(Color color) {
        m_solid_background = color;
    }

    const fs::path& file() const {
        return m_file;
    }

    int best_progress() const {
        return m_best_progress;
    }

    int attempts() const {
        return m_attempts;
    }

    void set_best_progress(int progress);
    void set_attempts(int attempts);
    void record_attempt();
    void set_state(LevelState state);

    LevelState state() const {
        return m_state;
    }

    bool loaded() const {
        return !m_objects.empty();
    }

    bool finished() const {
        return m_finished;
    }

    void set_finished(bool value) {
        m_finished = value;
        if (value) {
            m_current_progress = 100.0F;
        }
    }

    Vector2 player_start() const {
        return m_level_start;
    }

    Vector2 end_position() const {
        return m_level_end;
    }

    void request_finish(GameObject& visitor);

    Player* player() const {
        return m_player;
    }

    float progress() const {
        return m_current_progress;
    }

    const std::vector<Entity*>& objects() const {
        return m_objects;
    }
    Entity* find_object(int id) const;
    LevelCamera& camera_control() {
        return m_camera;
    }

    bool save();
    bool create(const LevelMetadata& metadata);
    bool remove();
    bool load(const fs::path& location);

    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const fs::path& directory) override;

    bool load_objects();
    Entity& instantiate_object(EntityType type);
    void add_object(Entity& object);
    Entity& create_object(EntityType type);
    void remove_object(Entity* object);
    void unload();
    void reset();
    void begin_death();

    void spawn_player(bool god_mode, bool free_mode);

    bool load_music();
    void play_music();
    void unload_music();
    void set_music_pitch(float pitch);
    void set_music_volume(float volume);

    using Entity::update;
    void update();
    float death_animation_progress() const;
    void update_behaviours(float frametime);

private:
    void prepare_start();
    void prepare_finish();
    void update_death_animation();
    void clear_objects();

    Dash& m_game;
    LevelCamera m_camera;
    std::vector<Entity*> m_objects;
    Player* m_player = nullptr;
    Finish* m_automatic_finish = nullptr;
    bool m_automatic_finish_active = false;
    bool m_loading_objects = false;

    fs::path m_file;
    LevelMetadata m_metadata;
    Color m_solid_background = BLACK;

    Vector2 m_level_start = {};
    Vector2 m_level_end = {};

    float m_current_progress = 0.0f;
    int m_best_progress = 0;
    int m_attempts = 0;

    Music m_music = {};
    float m_music_pitch = 1.0f;
    float m_music_volume = 1.0f;
    float m_death_start_volume = 1.0f;
    bool m_music_loaded = false;

    bool m_finished = false;
    bool m_finish_requested = false;

    bool m_finished_death_animation = false;
    float m_death_elapsed = 0.0f;

    LevelState m_state = LevelState::INVALID;
};
