//
// Created by mb6457 on 11/1/23.
//

#include <iostream>
#include "Actor.h"

namespace lve {


    void Actor::update(float deltaTime, GLFWwindow *window, Actor* Parent) {

        // If there is a parent, update the parent first
        if(parent != nullptr)
        {
            parent->update(deltaTime, window, parent);
        }

        // If the actor has already been updated this frame, stop updating this
        // actor here.
        if(updatedThisFrame)
        {
            return;
        }

        // If it should animate on a trigger
        if(animMode == TRIGGERED)
        {
            // Check the trigger
            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            {
                // Animate if triggered.
                if (gameObject->animationSequence.keyFrames.size() > 1)
                    inAnimation = true;
            }
        }

        // If it should animate continuously
        if(animMode == CONTINUOUS)
        {
            // And is not currently animating
            if(!inAnimation)
            {
                // Animate
                inAnimation = true;
            }
        }

        // If its animating...
        if(inAnimation)
        {
            try
            {
                // If the parent
                if(parent != nullptr)
                    gameObject->update(deltaTime, parent->gameObject); // update the game object with info from the parent.
                else
                    gameObject->update(deltaTime);  // Otherwise, update on its own.
            }
            catch (EndOfAnimationException)
            {
                // Stop the animation and return the object to
                // its origin.
                inAnimation = false;
                gameObject->transform.translation = originalTrans;
                gameObject->transform.rotation = originalRot;
                gameObject->transform.scale = originalScale;
                // If we have a parent
                if(parent != nullptr)
                {
                    // Update the render matrix with info from the parent
                    gameObject->setRenderMatrix(parent->gameObject);
                }
                else
                {
                    // Update the matrix on its own otherwise
                    gameObject->setRenderMatrix();
                }

            }
        }
        else
        {
            // If we have a parent
            if(parent != nullptr)
            {
                // Update the render matrix with info from the parent
                gameObject->setRenderMatrix(parent->gameObject);
            }
            else
            {
                // Update the matrix on its own otherwise
                gameObject->setRenderMatrix();
            }
        }
        // Tell everyone we're up to date
        updatedThisFrame = true;

    }

    Actor::Actor(LveGameObject *Object, std::string Name, Actor* Parent, ANIMATION_MODE animationMode) {
        parent = Parent;    // Set the parent
        animMode = animationMode;   // Set the animation mode
        gameObject = Object; // Set the game object to the game object
        name = Name;    // Set the friendly name
        // Record the original rotation, translation, and scale of the game object.
        originalRot = gameObject->localTransform.rotation;
        originalTrans = gameObject->localTransform.translation;
        originalScale = gameObject->transform.scale;


        gameObject->transform.translation = originalTrans;
        gameObject->transform.rotation = originalRot;

        // If we have a parent
        if(parent != nullptr)
        {
            // Update the render matrix with info from the parent
            gameObject->setRenderMatrix(parent->gameObject);
        }
        else
        {
            // Update the matrix on its own otherwise
            gameObject->setRenderMatrix();
        }
    }

    void Actor::prepare()
    {
        updatedThisFrame = false;
    }

    glm::mat4 Actor::getRenderMatrix() {
        if(parent != nullptr)
        {
            gameObject->transform.renderMatrix = gameObject->transform.mat4() * parent->getRenderMatrix();
            return gameObject->transform.renderMatrix;
        }
        else
        {
            gameObject->transform.renderMatrix = gameObject->transform.mat4();
            return gameObject->transform.renderMatrix;
        }
    }


    Actor::~Actor() = default;
} // lve