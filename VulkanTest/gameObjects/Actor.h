//
// Created by mb6457 on 11/1/23.
//

#ifndef COMPGRAPH_ACTOR_H
#define COMPGRAPH_ACTOR_H

#include "lve_game_object.hpp"
#include "Animations/Animation.h"

/**
 * The Actor
 * The Actor class is a container for the game object class. While the game object handles the
 * objects existence in 3D space, the actor is the driving force behind that existence, handling
 * game logic and updates. This is because the game object class is not extensible because of the
 * architecture of the render engine, so the actor exists outside of the render system, making it
 * extensible and overrideable.
 */

namespace lve {
    class Actor
    {

    private:
        LveGameObject* gameObject;              // The contained game object
        std::vector<Animation> animations;      // Vector of animations available to the actor <NOT IMPLEMENTED>
        bool inAnimation = false;               // Boolean if the actor is currently in an animation
        glm::vec3 originalTrans;                // Original translation of the game object
        glm::vec3 originalRot;                  // Original rotation of the game object
        glm::vec3 originalScale;                // Original scale of the game object

        glm::vec3 translationLocal;
        glm::vec3 rotationGlobal;
    public:

        enum ANIMATION_MODE
        {
            TRIGGERED,
            CONTINUOUS
        };

        ANIMATION_MODE animMode;

        std::string name;
        Actor* parent;
        bool updatedThisFrame;

        LveGameObject* getGameObject() { return gameObject; }

        /**
         * Constructor.
         * @param Object Reference to the game object in memory that is contained.
         */
        Actor(LveGameObject* Object, std::string Name, Actor* Parent = nullptr, ANIMATION_MODE animationMode = TRIGGERED);
        ~Actor();


        /**
         * Update
         * Updates the actor accoring to defined game logic.
         * Overrideable.
         * @param deltaTime Time since last frame of the game
         * @param window Reference to the current window context for handling inputs.
         */
        virtual void update(float deltaTime, GLFWwindow *window, Actor* p = nullptr);

        glm::mat4 getRenderMatrix();

        void prepare();

    private:

    };
} // lve

#endif //COMPGRAPH_ACTOR_H
