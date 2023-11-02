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
        texVec.push_back(LveImage::createImageFromFile(lveDevice, "../textures/Ch_Mai_95_D.png"));
        texVec.push_back(LveImage::createImageFromFile(lveDevice, "../textures/material_0.png"));
        texVec.push_back(LveImage::createImageFromFile(lveDevice, "../textures/material_1.png"));


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

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

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

    void FirstApp::loadGameObjects() {

        // Load all three models into game objects, apply their transforms
        // and emplace them into the scene
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "../models/SU-27CGLOWPOLY.obj");
        auto airoplane = LveGameObject::createGameObject();
        airoplane.model = lveModel;
        airoplane.transform.translation = {0.f, 2.f, 0.f};
        airoplane.transform.scale = {1.f, 1.f, 1.0f};
        airoplane.transform.rotation = {0.0f, 0.0f, glm::radians(180.0f)};

        airoplane.transform.animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.0f, 2.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, glm::radians(180.0f)),
                glm::vec3(1.0f),
                0.0f));
        airoplane.transform.animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.0f, -5.0f, 10.0f),
                glm::vec3(0.0f, glm::radians(90.0f), glm::radians(180.0f)),
                glm::vec3(1.0f),
                1.0f));
        airoplane.transform.animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.0f, -100.0f, 10.0f),
                glm::vec3(0.0f, glm::radians(90.0f), glm::radians(180.0f)),
                glm::vec3(1.0f),
                2.0f));
        airoplane.transform.animationSequence.duration = 2.0f;

        airoplane.textureBinding = 3;

        gameObjects.emplace(airoplane.getId(),std::move(airoplane));
        Actor* aactor = new Actor(&gameObjects.find(airoplane.getId())->second);
        actors.push_back(aactor);

        lveModel = LveModel::createModelFromFile(lveDevice, "../models/dergen.obj");
        auto dergen = LveGameObject::createGameObject();
        dergen.model = lveModel;
        dergen.transform.translation = {0.f, 50.0f, 50.f};
        dergen.transform.scale = {0.1f, 0.1f, 0.1f};
        dergen.transform.rotation = {0.0f, glm::radians(180.0f), 0.0f};

        dergen.textureBinding = 2;
        dergen.transform.animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 50.0f, 50.f),
                glm::vec3(0.0f, glm::radians(180.0f), 0.0f),
                glm::vec3(0.1f),
                1.0f
                ));
        dergen.transform.animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 50.0f, 50.f),
                glm::vec3(0.0f, 0.0, 0.0f),
                glm::vec3(0.01f),
                2.0f
        ));
        dergen.transform.animationSequence.duration = 2.0f;
        gameObjects.emplace(dergen.getId(),std::move(dergen));
        Actor* bactor = new Actor(&gameObjects.find(dergen.getId())->second);
        actors.push_back(bactor);

        lveModel = LveModel::createModelFromFile(lveDevice, "../models/Mai.obj");
        auto mai = LveGameObject::createGameObject();
        mai.model = lveModel;
        mai.transform.translation = {0.f, 1.35f, 0.f};
        mai.transform.scale = {1.f, 1.f, 1.f};
        mai.transform.rotation = { glm::radians(180.0f), glm::radians(180.0f), 0.0f};

        mai.transform.animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 1.35f, 0.f),
                glm::vec3(glm::radians(180.0f), glm::radians(180.0f), 0.0f),
                glm::vec3(1.f, 1.f, 1.f),
                0.5
                ));
        mai.transform.animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 1.35f, 0.f),
                glm::vec3(glm::radians(180.0f), glm::radians(90.0f), 0.0f),
                glm::vec3(1.1f, 1.1f, 1.f),
                1.0
        ));
        mai.transform.animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 2.35f, 0.f),
                glm::vec3(glm::radians(180.0f), glm::radians(0.0f), 0.0f),
                glm::vec3(2.0f, 2.0f, 2.0f),
                1.5
        ));
        mai.transform.animationSequence.keyFrames.push_back(AnimationKeyFrame(
                glm::vec3(0.f, 4.35f, 0.f),
                glm::vec3(glm::radians(180.0f), glm::radians(-90.0f), 0.0f),
                glm::vec3(1.1f, 1.1f, 1.f),
                2.0f
        ));

        mai.transform.animationSequence.duration = 2.0f;

        mai.textureBinding = 1;

        gameObjects.emplace(mai.getId(),std::move(mai));
        Actor* cactor = new Actor(&gameObjects.find(mai.getId())->second);
        actors.push_back(cactor);

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}  //
        };

        for (int i=0;i<lightColors.size();i++) {
            auto pointLight = LveGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(),
                                           {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }


    }



}

