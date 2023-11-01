//
// Created by mb6457 on 11/1/23.
//

#ifndef COMPGRAPH_ANIMATION_H
#define COMPGRAPH_ANIMATION_H

#include <glm/vec3.hpp>
#include <vector>
#include <string>

namespace lve {

    class AnimationNode {
    public:
        int nodeNum = 0;
        glm::vec3 targetTrans;
        glm::vec3 targetRot;
        glm::vec3 targetScale;
        float time;

        AnimationNode(glm::vec3 TargetTrans, glm::vec3 TargetRot, glm::vec3 TargetScale, float Time) {
            targetTrans = TargetTrans;
            targetRot = TargetRot;
            targetScale = TargetScale;
            time = Time;
        }
    };

    class Animation {
    private:
        std::vector<AnimationNode> nodes;
        unsigned int currentNode = 0;
        unsigned int nextNode = 1;
        glm::vec3 animationStep{0, 0, 0};
        float animationStarted = false;

        float currentTime = 0.0f;
        float animationDuration = 0.0f;
    public:
        Animation();

        ~Animation();

        void AddNode(AnimationNode node) {
            node.nodeNum = nodes.size();
            animationDuration += node.time;
            nodes.push_back(std::move(node));
        }

        glm::vec3 getAnimationStep(glm::vec3 currentPos, float deltaTime);


        static Animation BuildAnimation(std::string file);

    private:
        void calculateStep();

        bool ShouldMoveToNext(glm::vec3 currentPos);


    };

    class NotEnoughFramesException : public std::exception {

    };

    class EndOfAnimationException : public std::exception {};

} // lve

#endif //COMPGRAPH_ANIMATION_H
