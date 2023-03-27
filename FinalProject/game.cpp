#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <time.h>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp> 
#include <SOIL/SOIL.h>
#include <iostream>
#include <math.h>

#include <path_config.h>

#include "sprite.h"
#include "shader.h"
#include "player_game_object.h"
#include "enemy_game_object.h"
#include "collectible_game_object.h"
#include "game.h"

namespace game {

// Some configuration constants
// They are written here as global variables, but ideally they should be loaded from a configuration file

// Globals that define the OpenGL window and viewport
const char *window_title_g = "Assignment 2";
const unsigned int window_width_g = 800;
const unsigned int window_height_g = 600;
const glm::vec3 viewport_background_color_g(0.0, 0.0, 1.0);

// Directory with game resources such as textures
const std::string resources_directory_g = RESOURCES_DIRECTORY;


Game::Game(void)
{
    // Don't do work in the constructor, leave it for the Init() function
}


void Game::Init(void)
{

    // Initialize the window management library (GLFW)
    if (!glfwInit()) {
        throw(std::runtime_error(std::string("Could not initialize the GLFW library")));
    }

    // Set window to not resizable
    // Required or else the calculation to get cursor pos to screenspace will be incorrect
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); 

    // Create a window and its OpenGL context
    window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g, NULL, NULL);
    if (!window_) {
        glfwTerminate();
        throw(std::runtime_error(std::string("Could not create window")));
    }

    // Make the window's OpenGL context the current one
    glfwMakeContextCurrent(window_);

    // Initialize the GLEW library to access OpenGL extensions
    // Need to do it after initializing an OpenGL context
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        throw(std::runtime_error(std::string("Could not initialize the GLEW library: ") + std::string((const char *)glewGetErrorString(err))));
    }

    // Set event callbacks
    glfwSetFramebufferSizeCallback(window_, ResizeCallback);

    // Initialize sprite geometry
    sprite_ = new Sprite();
    sprite_->CreateGeometry();

    // Initialize sprite shader
    sprite_shader_.Init((resources_directory_g+std::string("/sprite_vertex_shader.glsl")).c_str(), (resources_directory_g+std::string("/sprite_fragment_shader.glsl")).c_str());

    // Initialize time
    current_time_ = 0.0;
}


Game::~Game()
{
    // Free memory for all objects
    // Only need to delete objects that are not automatically freed
    delete sprite_;
    for (int i = 0; i < game_objects_.size(); i++){
        delete game_objects_[i];
    }
    for (int i = 0; i < enemies_.size(); i++) {
        delete enemies_[i];
    }

    // Close window
    glfwDestroyWindow(window_);
    glfwTerminate();
}


void Game::Setup(void)
{

    // Setup the game world

    // Load textures
    SetAllTextures();

    // Setting the number of lives
    lives_ = 2;

    // Setting the loop break condition
    breakout_ = false;

    // Setting the number of items the player has picked up
    items_ = 0;

    // Setting that the user is not invulnerable to start
    invulnerable_ = false;

    // Setting the time for invulnerability
    invTime_ = 0;

    // Determining if the player is dead
    dead = false;

    // Setting up time for new enemy to spawn
    spawn = 7;

    // Setting up random number seed
    srand(time(NULL));

    // Setup the player object (position, texture, vertex count)
    // Note that, in this specific implementation, the player object should always be the first object in the game object vector 
    game_objects_.push_back(new PlayerGameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[0]));

    // Setup other objects
    enemies_.push_back(new EnemyGameObject(glm::vec3(-2.2f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[2]));
    enemies_.push_back(new EnemyGameObject(glm::vec3(2.8f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[2]));
    game_objects_.push_back(new CollectibleGameObject(glm::vec3(-3.5f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[6]));
    game_objects_.push_back(new CollectibleGameObject(glm::vec3(3.5f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[6]));
    game_objects_.push_back(new CollectibleGameObject(glm::vec3(0.0f, 3.5f, 0.0f), sprite_, &sprite_shader_, tex_[6]));
    game_objects_.push_back(new CollectibleGameObject(glm::vec3(-3.0f, -3.5f, 0.0f), sprite_, &sprite_shader_, tex_[6]));
    game_objects_.push_back(new CollectibleGameObject(glm::vec3(3.5f, -3.5f, 0.0f), sprite_, &sprite_shader_, tex_[6]));

    // Setting up the explosion objects
    GameObject* explosion = new GameObject(glm::vec3(100.0f, 100.0f, 100.0f), sprite_, &sprite_shader_, tex_[5]);
    explosion->SetScale(5.0f);
    game_objects_.push_back(explosion);
    GameObject* death = new GameObject(glm::vec3(100.0f, 100.0f, 100.0f), sprite_, &sprite_shader_, tex_[5]);
    death->SetScale(5.0f);
    game_objects_.push_back(death);

    // Setup background
    // In this specific implementation, the background is always the
    // last object
    GameObject *background = new GameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[3]);
    background->SetScale(10.0);
    game_objects_.push_back(background);
}


void Game::ResizeCallback(GLFWwindow* window, int width, int height)
{

    // Set OpenGL viewport based on framebuffer width and height
    glViewport(0, 0, width, height);
}


void Game::SetTexture(GLuint w, const char *fname)
{
    // Bind texture buffer
    glBindTexture(GL_TEXTURE_2D, w);

    // Load texture from a file to the buffer
    int width, height;
    unsigned char* image = SOIL_load_image(fname, &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);

    // Texture Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Texture Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void Game::SetAllTextures(void)
{
    // Load all textures that we will need
    glGenTextures(NUM_TEXTURES, tex_);
    SetTexture(tex_[0], (resources_directory_g+std::string("/textures/body_01.png")).c_str());
    SetTexture(tex_[1], (resources_directory_g+std::string("/textures/body_02.png")).c_str());
    SetTexture(tex_[2], (resources_directory_g+std::string("/textures/body_03.png")).c_str());
    SetTexture(tex_[3], (resources_directory_g+std::string("/textures/stars.png")).c_str());
    SetTexture(tex_[4], (resources_directory_g+std::string("/textures/orb.png")).c_str());
    SetTexture(tex_[5], (resources_directory_g+std::string("/textures/explosion.png")).c_str());
    SetTexture(tex_[6], (resources_directory_g+std::string("/textures/item.png")).c_str());
    glBindTexture(GL_TEXTURE_2D, tex_[0]);
}


void Game::MainLoop(void)
{
    // Loop while the user did not close the window
    double last_time = glfwGetTime();
    while (!glfwWindowShouldClose(window_)){

        // Clear background
        glClearColor(viewport_background_color_g.r,
                     viewport_background_color_g.g,
                     viewport_background_color_g.b, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set view to zoom out, centered by default at 0,0
        float camera_zoom = 0.25f;
        glm::mat4 view_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(camera_zoom, camera_zoom, camera_zoom));

        // Calculate delta time
        double current_time = glfwGetTime();
        double delta_time = current_time - last_time;
        last_time = current_time;

        // Update other events like input handling
        glfwPollEvents();

        // Update the game
        Update(view_matrix, delta_time);

        // Push buffer drawn in the background onto the display
        glfwSwapBuffers(window_);

        // Condition to end the game
        if (breakout_) {
            break;
        }
    }
}


void Game::Update(glm::mat4 view_matrix, double delta_time)
{

    // Update time
    current_time_ += delta_time;

    // Handle user input
    if (lives_ >= 0) {
        Controls(delta_time);
    }

    // Checking to see if new enemy should spawn
    if (current_time_ > spawn) {
        spawn += 7;
        int subFac = rand() % 4;
        float xCoord = (rand() % 3 - subFac);
        float yCoord = (rand() % 3 - subFac);
        enemies_.push_back(new EnemyGameObject(glm::vec3(xCoord, yCoord, 0.0f), sprite_, &sprite_shader_, tex_[2]));
    }

    // Update and render all game objects
    for (int i = 0; i < game_objects_.size(); i++) {
        // Get the current game object
        GameObject* current_game_object = game_objects_[i];

        // Updating the player's current position in other objects
        current_game_object->player = game_objects_[0]->GetPosition();

        // Update the current game object
        current_game_object->Update(delta_time);

        for (int k = 0; k < enemies_.size(); k++) {

            // Grabbing enemy from vector
            GameObject* enObj = enemies_[k];

            // Updating the player's position
            enObj->player = game_objects_[0]->GetPosition();

            // Handling the movement of the enemies
            if (enObj->state == false && dead == false) {
                // Patrolling (rotating) movement
                glm::vec3 tempPos = enObj->GetPosition();
                double xRot = (enObj->GetRotation()[0] + (tempPos[0] - enObj->GetRotation()[0]) * cos(0.5 * delta_time) - (tempPos[1] - enObj->GetRotation()[1]) * sin(0.5 * delta_time));
                double yRot = (enObj->GetRotation()[1] + (tempPos[1] - enObj->GetRotation()[1]) * cos(0.5 * delta_time) + (tempPos[0] - enObj->GetRotation()[0]) * sin(0.5 * delta_time));
                enObj->SetPosition(glm::vec3(xRot, yRot, 0.0));
                //std::cout << "x: " << xRot << " y: " << yRot << std::endl;
            } else if (dead == false) {
                // Moving (vector) movement
                glm::vec3 dirVec = enObj->player - enObj->GetPosition();
                //std::cout << "X: " << dirVec[0] << "Y: " << dirVec[1] << std::endl;
                enObj->SetVelocity(0.1f * dirVec);
                //enObj->SetPosition(enObj->GetPosition() + 0.000001f * enObj->GetVelocity());
            }

            // Update the current game object
            enObj->Update(delta_time);

            // Compute distance between the player and the enemy
            float distance = glm::length(enObj->GetPosition() - game_objects_[0]->GetPosition());

            // If distance reaches an upper threshold, the enemy begins to follow the player
            if (distance < 1.5 * game_objects_[0]->GetScale() && i == 0 && enObj->state == false) {
                enObj->state = true;
            }

            // If distance is below a lower threshold, we have a collision
            if (distance < game_objects_[0]->GetScale() - 0.2f && dead == false) {

                if (invulnerable_ == false) {

                    // Exploding collided enemy
                    game_objects_[game_objects_.size() - 3]->SetPosition(enObj->GetPosition());
                    enemies_.erase(enemies_.begin() + k);

                    // Exploding the player
                    if (lives_ <= 0) {
                        game_objects_[game_objects_.size() - 2]->SetPosition(game_objects_[0]->GetPosition());
                        game_objects_.erase(game_objects_.begin());
                        for (int l = 0; l < game_objects_.size(); l++) {
                            game_objects_[l]->SetVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
                        }
                        for (int m = 0; m < enemies_.size(); m++) {
                            enemies_[m]->SetVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
                        }
                        dead = true;
                    }

                    // Subtracting player lives and setting explosion end time
                    lives_ -= 1;
                    end_time_ = current_time_ + 2;

                }

            }

            // Rendering the enemy object
            enObj->Render(view_matrix, current_time_);
        }

        // Check for collision with other game objects
        // Note the loop bounds: we avoid testing the last object since
        // it's the background covering the whole game world
        for (int j = i + 1; j < (game_objects_.size()-3); j++) {
            GameObject* other_game_object = game_objects_[j];

            // Compute distance between object i and object j
            float distance = glm::length(current_game_object->GetPosition() - other_game_object->GetPosition());

            // If distance is below a lower threshold, we have a collision
            if (distance < current_game_object->GetScale() - 0.2f) {

                if (other_game_object->hostile_ == false) {

                    game_objects_.erase(game_objects_.begin() + j);
                    items_++;

                    if (items_ == 5) {
                        items_ = 0;
                        invulnerable_ = true;
                        SetTexture(tex_[0], (resources_directory_g + std::string("/textures/body_04.png")).c_str());
                        invTime_ = current_time_ + 10;
                    }

                }

            }
        }

        // Resetting the explosion at the proper time
        if (current_time_ >= end_time_ && end_time_ > 0) {
            game_objects_[game_objects_.size() - 3]->SetPosition(glm::vec3(100.0f, 100.0f, 100.0f));
            end_time_ = 0;

            // Ending the game upon player death
            if (lives_ < 0) {
                std::cout << "Game Over" << std::endl;
                breakout_ = true;
            }
        }

        // Reseting the player at the proper time
        if (current_time_ >= invTime_ && invTime_ > 0) {
            SetTexture(tex_[0], (resources_directory_g + std::string("/textures/body_01.png")).c_str());
            invulnerable_ = false;
            invTime_ = 0;
        }

        // Render game object
        current_game_object->Render(view_matrix, current_time_);
    }
}

void Game::Controls(double delta_time)
{
    // Get player game object
    GameObject *player = game_objects_[0];
    // Get current position
    glm::vec3 curpos = player->GetPosition();
    // Set standard forward and right directions
    glm::vec3 dir = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 right = glm::vec3(1.0, 0.0, 0.0);
    // Adjust motion increment based on a given speed
    float speed = 2.5;
    float motion_increment = speed*delta_time;

    // Check for player input and make changes accordingly
    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
        player->SetPosition(curpos + motion_increment*dir);
    }
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
        player->SetPosition(curpos - motion_increment*dir);
    }
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
        player->SetPosition(curpos + motion_increment*right);
    }
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
        player->SetPosition(curpos - motion_increment*right);
    }
    if (glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }
}
       
} // namespace game
