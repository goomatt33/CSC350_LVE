//
// Created by mb6457 on 11/1/23.
//

#ifndef COMPGRAPH_ACTOR_H
#define COMPGRAPH_ACTOR_H

#include "lve_game_object.hpp"
#include "Animations/Animation.h"

namespace lve {
    class Actor
    {
    private:
        LveGameObject* gameObject;
        std::vector<Animation> animations;
        bool inAnimation = false;
        glm::vec3 animStep{0, 0, 0};
    public:
        Actor(LveGameObject* Object);
        ~Actor();

        virtual void update(float deltaTime);

        void addAnimation(Animation animation)
        {
            animations.push_back(std::move(animation));
        }
    private:
        void calculateStep(glm::vec3 target, float time);
        bool shouldSnap(glm::vec3 target);

    };
} // lve

#endif //COMPGRAPH_ACTOR_H
