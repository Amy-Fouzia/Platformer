#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

unsigned int LEVELC_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0,
    3, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 3, 3,
    3, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 3, 3, 0, 3, 0, 3, 0, 3, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 3, 3, 3, 3, 0, 3, 0, 3, 0, 3, 0, 3, 3,
};

LevelC::~LevelC()
{
    delete[] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelC::initialise()
{
    m_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("assets/images/tiles.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 4, 1);

    //door
    m_state.door = new Entity();
    m_state.door->set_position(glm::vec3(13.0f, -1.0f, 0.0f));
    m_state.door->m_texture_id = Utility::load_texture("assets/images/door.png");

    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
     // Existing
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_position(glm::vec3(1.0f, 0.0f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->set_speed(2.5f);
    m_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("assets/images/stoopid.png");

    // Walking
    m_state.player->m_walking[m_state.player->LEFT] = new int[4] { 1, 5, 9, 13 };
    m_state.player->m_walking[m_state.player->RIGHT] = new int[4] { 3, 7, 11, 15 };
    m_state.player->m_walking[m_state.player->UP] = new int[4] { 2, 6, 10, 14 };
    m_state.player->m_walking[m_state.player->DOWN] = new int[4] { 0, 4, 8, 12 };

    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->RIGHT];  // start George looking left
    m_state.player->m_animation_frames = 4;
    m_state.player->m_animation_index = 0;
    m_state.player->m_animation_time = 0.0f;
    m_state.player->m_animation_cols = 4;
    m_state.player->m_animation_rows = 4;
    m_state.player->set_height(0.8f);
    m_state.player->set_width(0.8f);

    // Jumping
    m_state.player->m_jumping_power = 5.0f;

    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture("assets/images/uglee.png");

    m_state.enemies = new Entity[ENEMY_COUNT];
    m_state.enemies[0].set_entity_type(ENEMY);
    m_state.enemies[0].set_ai_type(GUARD);
    m_state.enemies[0].set_ai_state(IDLE);
    m_state.enemies[0].m_texture_id = enemy_texture_id;
    m_state.enemies[0].set_position(glm::vec3(4.0f, 0.0f, 0.0f));
    m_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_state.enemies[0].set_speed(1.0f);
    m_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));


    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_state.bgm = Mix_LoadMUS("assets/audio/dooblydoo.mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 16.0f);

    m_state.jump_sfx = Mix_LoadWAV("assets/audio/bounce.wav");
}

void LevelC::update(float delta_time)
{
    m_state.door->update(delta_time, m_state.player, m_state.player, 1, m_state.map);
    m_state.enemies[0].update(delta_time, m_state.player, NULL, 0, m_state.map);
    m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);

    // move patrolling enemy
    if (m_state.enemies[0].get_position().x <= 1) {
        m_state.enemies[0].move_right();
    }
    if (m_state.enemies[0].get_position().x >= 4) {
        m_state.enemies[0].move_left();
    }

    // player fell
    if (m_state.player->get_position().y < -10.0f) m_state.fell = true;

    //player ran into enemy
    if (m_state.player->m_collided_left_entity || m_state.player->m_collided_right_entity || m_state.player->m_collided_top_entity) {
        m_state.fell = true;
    }

    //player jumped on enemy
    if (m_state.player->m_collided_bottom_entity && m_state.player->get_position().y > m_state.enemies[0].get_position().y) {
        m_state.enemies[0].set_position(glm::vec3(-15.0, -5.0, 0.0));
    }

    // player reached door
    if (m_state.player->get_position().y == -1.1f && (m_state.player->get_position().x > 13.0f && m_state.player->get_position().x < 13.4f)) m_state.finished = true;
}

void LevelC::render(ShaderProgram* program)
{
    m_state.map->render(program);
    m_state.door->render(program);
    m_state.enemies[0].render(program);
    m_state.player->render(program);
}
