//
// Created by cdgira on 6/30/2023.
//
#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"
#include "lve_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <iostream>
#include <array>
#include <chrono>

namespace lve {



    FirstApp::FirstApp() {
        // We need to add a pool for the textureImages.
        globalPool = LveDescriptorPool::Builder(lveDevice)
                .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 * LveSwapChain::MAX_FRAMES_IN_FLIGHT) // This is for the texture maps.
                .build();

        loadGameObjects();
        // Texture Image loaded in first_app.hpp file.
    }

    FirstApp::~FirstApp()
    {
        for(int i = 0; i < actors.size(); i++)
        {
            delete actors[i];
        }
    }

    void FirstApp::run() {

        // The GlobalUbo is a fixed size so we can setup this up here then load in the data later.
        // So I need to load in the images first.
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i=0;i<uboBuffers.size();i++) {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                    lveDevice,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        //Add the three images to an vector
        texVec.push_back(LveImage::createImageFromFile(lveDevice, "../textures/ground.png"));
        texVec.push_back(LveImage::createImageFromFile(lveDevice, "../textures/grass.png"));
        texVec.push_back(LveImage::createImageFromFile(lveDevice, "../textures/LittleDudesLittleFace.png"));


        // Added two additional bindings from the original code
        // for the two extra images
        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT) // Added 2nd texture
                .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT) // Added 3rd texture

                .build();
        // Need to see if anything needs to be done here for the texture maps.
        // Something isn't beting setup right for the Image Info, information is not getting freed correctly.
        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i=0;i<globalDescriptorSets.size();i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            //auto imageInfo = textureImage->descriptorImageInfo();
            //auto imageInfo = texVec[i]->descriptorImageInfo();
            auto imageInfo1 = texVec[0]->descriptorImageInfo();     // Get the info for the first texture
            auto imageInfo2 = texVec[1]->descriptorImageInfo();     // Get the info for the second texture
            auto imageInfo3 = texVec[2]->descriptorImageInfo();     // Third texture
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo1)     // Write the first texture to the descriptor set
                .writeImage(2, &imageInfo2)     // Write the second
                .writeImage(3, &imageInfo3)     // Write the third
                .build(globalDescriptorSets[i]); // Should only build a set once.
        }

        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        LveCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            if (frameTime > 1.6f)
                frameTime = 1.6f;

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            for(int i = 0; i < actors.size(); i++)
            {
                actors[i]->prepare();
            }

            for(int i = 0; i < actors.size(); i++)
            {
                actors[i]->update(frameTime, lveWindow.getGLFWwindow());
            }


            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
            if (auto commandBuffer = lveRenderer.beginFrame()) {
                int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer,camera, globalDescriptorSets[frameIndex], gameObjects};
                //update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                //render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.render(frameInfo); // Solid Objects
                pointLightSystem.render(frameInfo);  // Transparent Objects
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }

        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    // temporary helper function, creates a 1x1x1 cube centered at offset with an index buffer
    std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
        LveModel::Builder modelBuilder{};
        modelBuilder.vertices = {
                // left face (white)
                {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
                {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
                {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
                {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

                // right face (yellow)
                {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
                {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
                {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
                {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

                // top face (orange, remember y axis points down)
                {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
                {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
                {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
                {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

                // bottom face (red)
                {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
                {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
                {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
                {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

                // nose face (blue)
                {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
                {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
                {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
                {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

                // tail face (green)
                {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
                {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
                {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
                {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        };
        for (auto& v : modelBuilder.vertices) {
            v.position += offset;
        }

        modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                                12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

        return std::make_unique<LveModel>(device, modelBuilder);
    }

    Actor* FirstApp::createActor(std::string file, std::string name, int textureBinding, Actor::ANIMATION_MODE animationMode,
                       glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
    {
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, file);
        auto gameObj = LveGameObject::createGameObject();
        gameObj.model = lveModel;
        gameObj.localTransform.translation = translation;
        gameObj.localTransform.rotation = rotation;
        gameObj.transform.scale = scale;
        gameObj.textureBinding = textureBinding;
        gameObjects.emplace(gameObj.getId(), std::move(gameObj));
        Actor* actor = new Actor(&gameObjects.find(gameObj.getId())->second, name, nullptr, animationMode);
        return actor;
    }

    void FirstApp::loadGameObjects() {

        // Load all three models into game objects, apply their transforms
        // and emplace them into the scene

        Actor* Torso = createActor("../models/LittleDudeTorso.obj", "Torso", 3, Actor::TRIGGERED,
                                   glm::vec3(-5.0f,0.0f,10.0f),
                                   glm::vec3(glm::radians(225.0f), 0.0f, glm::radians(180.f)));
        actors.push_back(Torso);


        Torso->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(-5.0f,0.0f,10.0f),
                glm::vec3(glm::radians(225.0f), 0.0f, glm::radians(180.0f)),
                glm::vec3(1.0f),
                0.0f));

        Torso->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(5.0f, 0.f, 0.0f),
                glm::vec3(glm::radians(225.0f), 0.0f, glm::radians(180.0f)),
                glm::vec3(1.0f),
                2.0f));
        Torso->getGameObject()->animationSequence.duration = 2.0f;

        Actor* head = createActor("../models/LittleDudeHead.obj", "Head", 3);
        actors.push_back(head);
        head->parent = Torso;

        Actor* leftThigh = createActor("../models/LittleDudeLeftThigh.obj", "leftThigh", 3);
        actors.push_back(leftThigh);
        leftThigh->parent = Torso;

        leftThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.0f));
        leftThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                0.5f));
        leftThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                1.f));
        leftThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                1.5f));
        leftThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                2.f));

        leftThigh->getGameObject()->animationSequence.duration = 2.0f;

        Actor* rightThigh = createActor("../models/LittleDudeRightThigh.obj", "RightThigh", 3);
        actors.push_back(rightThigh);
        rightThigh->parent = Torso;

        rightThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.0f));
        rightThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                0.5f));
        rightThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                1.f));
        rightThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                1.5f));
        rightThigh->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                2.f));


        rightThigh->getGameObject()->animationSequence.duration = 2.0f;

        Actor* leftCalf = createActor("../models/LittleDudeLeftCalf.obj", "leftLeg", 3);
        actors.push_back(leftCalf);
        leftCalf->parent = leftThigh;

        leftCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.0f));

        leftCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(1.5f, 0.0f, 0.0f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                0.25f));

        leftCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.5f));
        leftCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                1.f));
        leftCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(2.0f, 0.0f, 0.0f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                1.25f));
        leftCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                1.5f));
        leftCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                2.f));

        leftCalf->getGameObject()->animationSequence.duration = 2.0f;

        Actor* rightCalf = createActor("../models/LittleDudeRightCalf.obj", "rightCalf", 3);
        actors.push_back(rightCalf);
        rightCalf->parent = rightThigh;

        rightCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.0f));
        rightCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.5f));
        rightCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(2.0f, 0.0f, 0.0f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                0.75f));
        rightCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                1.0f));
        rightCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                1.5f));
        rightCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(2.0f, 0.0f, 0.0f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                1.75f));
        rightCalf->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                2.f));

        rightCalf->getGameObject()->animationSequence.duration = 2.0f;

        Actor* leftShoulder = createActor("../models/LittleDudeLeftShoulder.obj", "leftShoulder", 3);
        actors.push_back(leftShoulder);
        leftShoulder->parent = Torso;

        leftShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.0f));

        leftShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(-2.f,1.f,0.f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                0.5f));
        leftShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(2.f,1.f,0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                1.f));
        leftShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(-2.f,1.f,0.f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                1.5f));
        leftShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(2.f,1.f,0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                2.f));

        leftShoulder->getGameObject()->animationSequence.duration = 2.0f;

        Actor* rightShoulder = createActor("../models/LittleDudeRightShoulder.obj", "rightShoulder", 3);
        actors.push_back(rightShoulder);
        rightShoulder->parent = Torso;

        rightShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.0f));

        rightShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(2.f,1.f,0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                0.5f));
        rightShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(-2.f,1.f,0.f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                1.f));
        rightShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(2.f,1.f,0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                1.5f));
        rightShoulder->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(-2.f,1.f,0.f),
                glm::vec3(0.f, 0.f, glm::radians(-45.f)),
                glm::vec3(0.0f),
                2.f));

        rightShoulder->getGameObject()->animationSequence.duration = 2.0f;


        Actor* leftArm = createActor("../models/LittleDudeLeftArm.obj", "leftArm", 3);
        actors.push_back(leftArm);
        leftArm->parent = leftShoulder;


        leftArm->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.0f));

        leftArm->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(1.f, 0.5f, 0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                0.5f));
        leftArm->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(1.f, 0.5f, 0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                2.f));

        leftArm->getGameObject()->animationSequence.duration = 2.0f;

        Actor* rightArm = createActor("../models/LittleDudeRightArm.obj", "rightArm", 3);
        actors.push_back(rightArm);
        rightArm->parent = rightShoulder;

        rightArm->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f),
                glm::vec3(0.f),
                glm::vec3(0.0f),
                0.0f));

        rightArm->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(1.f, 0.5f, 0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                0.5f));
        rightArm->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(1.f, 0.5f, 0.f),
                glm::vec3(0.f, 0.f, glm::radians(45.f)),
                glm::vec3(0.0f),
                2.f));

        rightArm->getGameObject()->animationSequence.duration = 2.0f;

        Actor* ground = createActor("../models/Ground.obj", "ground", 1, Actor::TRIGGERED,
                                    glm::vec3(0.0f, 8, 10.f),
                                    glm::vec3(glm::radians(45.0f), 0.0f, 0.0f));
        actors.push_back(ground);

        Actor* grassBase = createActor("../models/GrassBlade.obj", "grassBase", 2, Actor::CONTINUOUS,
                                       glm::vec3(5.f, 4.f, 15.f),
                                       glm::vec3(glm::radians(-85.f), glm::radians(180.f), 0.f));
        actors.push_back(grassBase);

        Actor* grassTop = createActor("../models/GrassBlade.obj", "grassBase", 2, Actor::CONTINUOUS,
                                      glm::vec3(0.f, 2.f, 0.f));
        actors.push_back(grassTop);
        grassTop->parent = grassBase;

        grassBase->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(5.f, 4.f, 15.f),
                glm::vec3(glm::radians(-85.f),glm::radians(225.f),0.f),
                glm::vec3(1.0f),
                0.0f));

        grassBase->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(5.f, 4.f, 15.f),
                glm::vec3(glm::radians(-85.f), glm::radians(135.f), 0.f),
                glm::vec3(1.0f),
                0.5f));

        grassBase->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(5.f, 4.f, 15.f),
                glm::vec3(glm::radians(-85.f),glm::radians(225.f),0.f),
                glm::vec3(1.0f),
                1.0f));

        grassBase->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(5.f, 4.f, 15.f),
                glm::vec3(glm::radians(-85.f), glm::radians(135.f), 0.f),
                glm::vec3(1.0f),
                1.5f));

        grassBase->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(5.f, 4.f, 15.f),
                glm::vec3(glm::radians(-85.f),glm::radians(225.f),0.f),
                glm::vec3(1.0f),
                2.0f));

        grassBase->getGameObject()->animationSequence.duration = 2.f;


        grassTop->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 2.f, 0.f),
                glm::vec3(0.f,glm::radians(45.f),0.f),
                glm::vec3(1.0f),
                0.0f));

        grassTop->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 2.f, 0.f),
                glm::vec3(0.f, glm::radians(-45.f), 0.f),
                glm::vec3(1.0f),
                0.5f));

        grassTop->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 2.f, 0.f),
                glm::vec3(0.f,glm::radians(45.f),0.f),
                glm::vec3(1.0f),
                1.0f));

        grassTop->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 2.f, 0.f),
                glm::vec3(0.f, glm::radians(-45.f), 0.f),
                glm::vec3(1.0f),
                1.5f));

        grassTop->getGameObject()->animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 2.f, 0.f),
                glm::vec3(0.f,glm::radians(45.f),0.f),
                glm::vec3(1.0f),
                2.0f));

        grassTop->getGameObject()->animationSequence.duration = 2.f;

        auto star1 = LveGameObject::makePointLight();
        star1.transform.translation = glm::vec3(0.f, -5.f, 10.f);
        gameObjects.emplace(star1.getId(), std::move(star1));

        auto star2 = LveGameObject::makePointLight();
        star2.transform.translation = glm::vec3(3.f, -4.f, 10.f);
        gameObjects.emplace(star2.getId(), std::move(star2));

        auto star3 = LveGameObject::makePointLight();
        star3.transform.translation = glm::vec3(1.5f, -3.f, 10.f);
        gameObjects.emplace(star3.getId(), std::move(star3));

    }
}

