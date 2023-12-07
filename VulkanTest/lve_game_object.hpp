//
// Created by cdgira on 7/13/2023.
//

#ifndef VULKANTEST_LVE_GAME_OBJECT_HPP
#define VULKANTEST_LVE_GAME_OBJECT_HPP

#include "lve_model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>

namespace lve {


    // Animation KeyFrame class
    class AnimationKeyFrame {
    public:
        AnimationKeyFrame(glm::vec3 trans, glm::vec3 rot, glm::vec3 scl, float time)
        {
            translation = trans;
            rotation = rot;
            scale = scl;
            timeStamp = time;
        }
        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;

        float timeStamp;
    };

    // Animation Sequence class
    class AnimationSequence {
    public:
        std::vector<AnimationKeyFrame> keyFrames;
        float duration;
    };

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{0.0f};

        glm::mat4 globalMat4;

        //AnimationSequence could be changed to a vector to support multiple animations.
        // Need to go over base form of each in class.
        // Need to show standard rotation matrix found in most books.
        glm::mat4 mat4();
        glm::mat4 normalMatrix();
        glm::mat4 renderMatrix;

    };


    struct PointLightComponent {
        float lightIntensity = 1.0f;
    };


    class LveGameObject {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, LveGameObject>;


        static LveGameObject createGameObject() {
            static id_t currentId = 0;
            return LveGameObject{currentId++};
        }


        static LveGameObject makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));


        LveGameObject(const LveGameObject&) = delete;
        LveGameObject &operator=(const LveGameObject&) = delete;
        LveGameObject(LveGameObject&&) = default;
        LveGameObject &operator=(LveGameObject&&) = default;


        id_t getId() const { return id; }

        /**
         * Sets the render matrix that the render system sees. If the object has a parent,
         * the render matrix is calculated with the parent's transform.
         * NOTE: SCALE CURRENTLY NOT SUPPORTED! MAKE SURE ALL OBJECTS HAVE A SCALE OF 1!
         * @param parent Game object this one is child to.
         */
        void setRenderMatrix(LveGameObject* parent = nullptr);




        glm::vec3 color{};
        TransformComponent transform{};
        TransformComponent localTransform{};
        int32_t textureBinding = -1;


        // Optional components
        std::shared_ptr<LveModel> model{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;


        AnimationSequence animationSequence; // sequence of the animation;
        void update(float deltaTime, LveGameObject* parent = nullptr);  // New method to update based on animations

        glm::mat4 getRenderMatrix();


    private:
        LveGameObject(id_t id) : id(id) {}
        id_t id;

        float currentTime = 0.0f;  // Current time since the animation started

    };

    // Exception signalling the end of the animation
    class EndOfAnimationException : public std::exception {};

}


#endif //VULKANTEST_LVE_GAME_OBJECT_HPP
