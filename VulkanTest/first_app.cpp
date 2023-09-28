//
// Created by cdgira on 6/30/2023.
//
#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <iostream>
#include <array>
#include <chrono>
#include <fstream>
#include <sstream>

namespace lve {

    FirstApp::FirstApp() {
        loadGameObjects();
    }

    FirstApp::~FirstApp() { }

    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        LveCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = LveGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects,camera);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }

        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    // temporary helper function, creates a 1x1x1 cube centered at offset
    std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
        std::vector<LveModel::Vertex> vertices{

                // left face (white)
                {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
                {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
                {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
                {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
                {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
                {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

                // right face (yellow)
                {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
                {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
                {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
                {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
                {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
                {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

                // top face (orange, remember y axis points down)
                {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
                {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
                {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
                {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
                {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
                {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

                // bottom face (red)
                {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
                {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
                {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
                {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
                {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
                {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

                // nose face (blue)
                {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
                {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
                {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
                {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
                {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
                {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

                // tail face (green)
                {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
                {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
                {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
                {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
                {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
                {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

        };
        for (auto& v : vertices) {
            v.position += offset;
        }
        return std::make_unique<LveModel>(device, vertices);
    }

    FirstApp::Cube FirstApp::MakeCube(float topFLX, float topFLY, float topFLZ, float size, int palletColor)
    {
        lve::FirstApp::Cube cu = {};

        cu.FrontTopLeft.position = glm::vec3(topFLX, topFLY, topFLZ);
        cu.FrontTopRight.position = glm::vec3(topFLX + size, topFLY, topFLZ);
        cu.FrontBottomLeft.position = glm::vec3(topFLX, topFLY + size, topFLZ);
        cu.FrontBottomRight.position = glm::vec3(topFLX + size, topFLY + size, topFLZ);

        cu.BackTopLeft.position = glm::vec3(topFLX, topFLY, topFLZ - size);
        cu.BackTopRight.position = glm::vec3(topFLX + size, topFLY, topFLZ - size);
        cu.BackBottomLeft.position = glm::vec3(topFLX, topFLY + size, topFLZ - size);
        cu.BackBottomRight.position = glm::vec3(topFLX + size, topFLY + size, topFLZ - size);

        cu.FrontTopLeft.color = pallet[palletColor];
        cu.FrontTopRight.color = pallet[palletColor];
        cu.FrontBottomLeft.color = pallet[palletColor];
        cu.FrontBottomRight.color = pallet[palletColor];

        cu.BackTopLeft.color = pallet[palletColor];
        cu.BackTopRight.color = pallet[palletColor];
        cu.BackBottomLeft.color = pallet[palletColor];
        cu.BackBottomRight.color = pallet[palletColor];

        cu.color = pallet[palletColor];
        
        return cu;
    }

    void FirstApp::MakeModel(LveDevice& device, glm::vec3 pos, glm::vec3 offset)
    {
        Cube cube = MakeCube(-0.5f, -0.5f, 0.5f, 1.0f, 5);

        std::vector<Cube> cubes;

        float currentx = pos.x;
        float currenty = pos.y;
        float s = 0.25;

        for (int y = 0; y < colorArray.size(); y++) {
            for (int x = 0; x < colorArray[y].size(); x++) {
                cubes.push_back(MakeCube(currentx, currenty, pos.z, s, colorArray[y][x]));
                currentx += s;
            }
            currentx = pos.x;
            currenty += s;
        }

        for (int i = 0; i < cubes.size(); i++)
        {
            std::vector<LveModel::Vertex> vertices
            {
                // left face 
                    cubes[i].FrontTopLeft,
                    cubes[i].BackTopLeft,
                    cubes[i].BackBottomLeft,
                    cubes[i].FrontTopLeft,
                    cubes[i].BackBottomLeft,
                    cubes[i].FrontBottomLeft,

                    // right face (yellow)
                    cubes[i].FrontTopRight,
                    cubes[i].BackTopRight,
                    cubes[i].BackBottomRight,
                    cubes[i].FrontTopRight,
                    cubes[i].BackBottomRight,
                    cubes[i].FrontBottomRight,

                    // top face (orange, remember y axis points down)
                    cubes[i].FrontTopLeft,
                    cubes[i].FrontTopRight,
                    cubes[i].BackTopRight,
                    cubes[i].FrontTopLeft,
                    cubes[i].BackTopRight,
                    cubes[i].BackTopLeft,

                    // bottom face (red)
                    cubes[i].FrontBottomLeft,
                    cubes[i].FrontBottomRight,
                    cubes[i].BackBottomRight,
                    cubes[i].FrontBottomLeft,
                    cubes[i].BackBottomRight,
                    cubes[i].BackBottomLeft,

                    // nose face (blue)
                    cubes[i].FrontTopLeft,
                    cubes[i].FrontTopRight,
                    cubes[i].FrontBottomRight,
                    cubes[i].FrontTopLeft,
                    cubes[i].FrontBottomRight,
                    cubes[i].FrontBottomLeft,

                    // tail face (green)
                    cubes[i].BackTopLeft,
                    cubes[i].BackTopRight,
                    cubes[i].BackBottomRight,
                    cubes[i].BackTopLeft,
                    cubes[i].BackBottomRight,
                    cubes[i].BackBottomLeft,
            };
            for (auto& v : vertices) {
                v.position += offset;
            }

            auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);
            auto triangleGameObject = LveGameObject::createGameObject();
            triangleGameObject.model = lveModel;
            triangleGameObject.color = cubes[i].color;
            triangleGameObject.transform.translation = pos;
            gameObjects.push_back(std::move(triangleGameObject));
        }

        //return std::make_unique<LveModel>(device, vertices);
    }



    void FirstApp::loadGameObjects() {

        loadFromFile("./image.txt");

        MakeModel(lveDevice, { -0.5f, -0.5f, 0.f }, { 0.0f, 0.0f, 0.0f });

    }

    void FirstApp::loadFromFile(std::string File) {
        std::string line;
        std::ifstream file(File);


        for (int i = 0; i < 9; i++)
        {
            std::getline(file, line);
            std::stringstream ss(line);
            std::string word;
            std::vector<float> rgb;
            while (!ss.eof())
            {
                std::getline(ss, word, ',');
                rgb.push_back(std::stof(word));
            }
            glm::vec3 color = { rgb[0], rgb[1], rgb[2] };

            pallet.push_back(color);
        }

        for (int i = 0; i < 8; i++)
        {
            std::getline(file, line);
            std::stringstream ss(line);
            std::string word;
            std::vector<int> nums;
            while (!ss.eof())
            {
                std::getline(ss, word, ',');
                nums.push_back(std::stoi(word));
            }
            colorArray.push_back(nums);
        }

    }
}

