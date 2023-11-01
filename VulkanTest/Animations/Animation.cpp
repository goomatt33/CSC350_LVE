//
// Created by mb6457 on 11/1/23.
//

#include <glm/geometric.hpp>
#include <iostream>
#include "Animation.h"

namespace lve {
    Animation::Animation() {
    }

    Animation::~Animation() {

    }

    glm::vec3 Animation::getAnimationStep(glm::vec3 current, float deltaTime)
    {
        if(nodes.size() < 2)
            throw NotEnoughFramesException();

        currentTime += deltaTime;
        if(currentTime >= animationDuration) {
            animationStarted = false;
            throw EndOfAnimationException();
        }
        if(!animationStarted)
        {
            animationStarted = true;
            return nodes[currentNode].targetTrans;
        }

        float alpha = glm::distance(current, nodes[nextNode].targetTrans) / nodes[nextNode].time;
        glm::vec3 translation = glm::mix(current, nodes[nextNode].targetTrans, alpha);
        return translation;

    }

    void Animation::calculateStep()
    {
        animationStep = (nodes[currentNode + 1].targetTrans - nodes[currentNode].targetTrans) / nodes[currentNode + 1].time;
    }

    bool Animation::ShouldMoveToNext(glm::vec3 currentPos)
    {
        float currentPosDist = glm::length(nodes[currentNode + 1].targetTrans - currentPos);
        float stepDist = glm::length(nodes[currentNode + 1].targetTrans - animationStep);
        std::cout << currentPos.x <<"," << currentPos.y <<"," << currentPos.z << " ";
        if(currentPosDist < stepDist)
        {
            return true;
        }
        return false;
    }


} // lve