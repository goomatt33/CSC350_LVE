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

    public:

        /**
         * Enum for the animation modes available to use
         */
        enum ANIMATION_MODE
        {
            TRIGGERED,
            CONTINUOUS
        };

        /**
         * Stores the actors animation mode.
         * If the mode is TRIGGERED, the object will animate when triggered.
         * If the mode is CONTINUOUS, the object will animate at start
         * and continue looping until program end.
         */
        ANIMATION_MODE animMode;

        std::string name;
        Actor* parent;          // Stores a reference to the parent actor.
        bool updatedThisFrame;  // Stores if the actor has been updated this frame

        /**
         * Returns the game object contained by the actor
         */
        LveGameObject* getGameObject() { return gameObject; }

        /**
         * Constructor.
         * @param Object Reference to the game object in memory that is contained.
         * @param Name The friendly name of the object. Used for debugging.
         * @param Parent A reference to the parent actor.
         * @param animationMode the mode for animating.
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

        /**
         * Gets the render matrix for the actor
         * @return mat4 render matrix
         */
        glm::mat4 getRenderMatrix();

        /**
         * Prepares the actor for the next frame.
         * Essentially just sets 'updatedThisFrame' to
         * false.
         */
        void prepare();

    private:

    };
} // lve

#endif //COMPGRAPH_ACTOR_H
