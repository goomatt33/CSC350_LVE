//
// Created by mb6457 on 11/1/23.
//

#include <iostream>
#include "Actor.h"

namespace lve {


    void Actor::update(float deltaTime, GLFWwindow *window, Actor* Parent) {

        if(parent != nullptr)
        {
            parent->update(deltaTime, window, parent);
        }

        if(updatedThisFrame)
        {
            return;
        }
        if(animMode == TRIGGERED)
        {
            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
                if (gameObject->animationSequence.keyFrames.size() > 1)
                    inAnimation = true;
            }
        }

        if(animMode == CONTINUOUS)
        {
            if(!inAnimation)
            {
                inAnimation = true;
            }
        }

        if(inAnimation)
        {
            try
            {
                if(parent != nullptr)
                    gameObject->update(deltaTime, parent->gameObject);
                else
                    gameObject->update(deltaTime);
            }
            catch (EndOfAnimationException)
            {
                // Stop the animation and return the object to
                // its origin.
                inAnimation = false;
                gameObject->transform.translation = originalTrans;
                gameObject->transform.rotation = originalRot;
                gameObject->transform.scale = originalScale;

                if(parent != nullptr)
                {
                    //gameObject->transform.translation = originalTrans + parent->gameObject->transform.translation;
                    //gameObject->transform.rotation = originalRot + parent->gameObject->transform.translation;
                    gameObject->setRenderMatrix(parent->gameObject);
                }
                else
                {
                    gameObject->setRenderMatrix();
                }

            }

            //std::cout << name << ": " << gameObject->transform.rotation.x << " "
            //          << gameObject->transform.rotation.y << " " << gameObject->transform.rotation.z << "\n";

        }
        else
        {
            if(parent != nullptr)
            {
                //gameObject->transform.translation = originalTrans + parent->gameObject->transform.translation;
                //gameObject->transform.rotation = originalRot + parent->gameObject->transform.rotation;
                gameObject->setRenderMatrix(parent->gameObject);
            }
            else
            {
                gameObject->setRenderMatrix();
            }
        }

        updatedThisFrame = true;

    }

    Actor::Actor(LveGameObject *Object, std::string Name, Actor* Parent, ANIMATION_MODE animationMode) {
        parent = Parent;
        animMode = animationMode;
        gameObject = Object; // Set the game object to the game object
        name = Name;
        // Record the original rotation, translation, and scale of the game object.
        originalRot = gameObject->localTransform.rotation;
        originalTrans = gameObject->localTransform.translation;
        originalScale = gameObject->transform.scale;


        gameObject->transform.translation = originalTrans;
        gameObject->transform.rotation = originalRot;

        parent = Parent;
        if(parent != nullptr)
        {
            gameObject->setRenderMatrix(parent->gameObject);

        }
        else
        {
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