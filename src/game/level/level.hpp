#pragma once

#include "core/components/rigid-body.hpp"
#include "core/object.hpp"
#include "game/paths.hpp"
#include "behaviour.hpp"
#include "object-type.hpp"
#include "game/dash-file.hpp"

#include <string>
#include <memory>
#include <vector>

enum class LevelState : int8_t {
    INVALID = -1, // corrupted, not loaded, whatever
    PLAYING,
    EDITING,
    PAUSED,
    DEATH
};

class Player;
class Dash;

struct LevelObject {
    ObjectType type;
    GameObject* object;
};

struct LevelMetadata {
    std::string name;
    std::string creator;
    std::string description;
    DashFile music;
    Vector2 player_start = {};
};

class DashLevel : public GameObject {
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
    }

    Vector2 player_start() const {
        return m_metadata.player_start;
    }

    Player* player() const {
        return m_player;
    }

    float progress() const {
        return m_current_progress;
    }

    const std::vector<LevelObject>& objects() const {
        return m_objects;
    }

    bool save();
    bool load(const fs::path& location);

    void serialize(Serializer& serializer) const override;
    void deserialize(Deserializer& deserializer, const fs::path& directory) override;

    bool load_objects();
    void unload();
    void reset();
    void begin_death();

    void spawn_player(bool god_mode, bool free_mode);

    bool load_music();
    void play_music();
    void unload_music();
    void set_music_pitch(float pitch);
    void set_music_volume(float volume);

    bool update();
    float death_animation_progress() const;
    void update_behaviours(float frametime);
    void add_behaviour(std::unique_ptr<Behaviour> behaviour);

private:
    void update_death_animation();
    void clear_objects();

    Dash& m_game;
    std::vector<LevelObject> m_objects;
    std::vector<std::unique_ptr<Behaviour>> m_behaviours;
    Player* m_player = nullptr;
    bool m_loading_objects = false;

    fs::path m_file;  // runtime
    LevelMetadata m_metadata;

    // level start/end
    Vector2 m_level_end = {};    // runtime

    // progress
    float m_current_progress = 0.0f;
    int m_best_progress = 0;
    int m_attempts = 0;

    Music m_music = {};
    float m_music_pitch = 1.0f;
    float m_music_volume = 1.0f;
    float m_death_start_volume = 1.0f;
    bool m_music_loaded = false;

    bool m_finished = false;

    bool m_finished_death_animation = false;
    float m_death_elapsed = 0.0f;

    LevelState m_state = LevelState::INVALID;
};
