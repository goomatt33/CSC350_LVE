//
// Created by mb6457 on 11/1/23.
//

#include <iostream>
#include "Actor.h"

namespace lve {


    void Actor::update(float deltaTime) {

        gameObject->transform.translation = animations[0].getAnimationStep(gameObject->transform.translation, deltaTime);

    }

    Actor::Actor(LveGameObject *Object) {
        gameObject = Object;
    }


    Actor::~Actor() = default;
} // lve