//
// Created by mb6457 on 11/1/23.
//

#include <iostream>
#include "Actor.h"

namespace lve {


    void Actor::update(float deltaTime, GLFWwindow *window) {

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            inAnimation =true;
        }

        if(inAnimation)
        {
            try
            {
                gameObject->transform.update(deltaTime);
            }
            catch (EndOfAnimationException)
            {
                inAnimation = false;
                gameObject->transform.translation = originalTrans;
                gameObject->transform.rotation = originalRot;
                gameObject->transform.scale = originalScale;
            }

        }

    }

    Actor::Actor(LveGameObject *Object) {
        gameObject = Object;
        originalRot = gameObject->transform.rotation;
        originalTrans = gameObject->transform.translation;
        originalScale = gameObject->transform.scale;
    }


    Actor::~Actor() = default;
} // lve