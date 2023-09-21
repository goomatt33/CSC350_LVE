//
// Created by cdgira on 6/30/2023.
//
#include "first_app.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <iostream>
#include <array>
#include <fstream>
#include <sstream>

namespace lve {

    FirstApp::FirstApp() {
        loadGameObjects();
    }

    FirstApp::~FirstApp() { }

    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();
            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }

        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::loadGameObjects() {
        loadFromFile("./image.txt");

        std::vector<square> squares;
        float startx = -1.0f;
        float currentx = -1.0f;
        float starty = -1.0f;
        float s = 0.25;
        for (int y = 0; y < colorArray.size(); y++) {
            for (int x = 0; x < colorArray[y].size(); x++) {
                squares.push_back(MakeSquare(currentx, starty, s, colorArray[y][x]));
                currentx += s;
            }
            currentx = startx;
            starty += s;
        }

        std::vector<LveGameObject> objects;
        for(int i = 0; i < squares.size(); i++)
        {
            std::vector<LveModel::Vertex> vertices;

            vertices.push_back(squares[i].TopLeft);
            vertices.push_back(squares[i].TopRight);
            vertices.push_back(squares[i].BottomRight);
            vertices.push_back(squares[i].TopLeft);
            vertices.push_back(squares[i].BottomRight);
            vertices.push_back(squares[i].BottomLeft);
            auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);
            auto triangleGameObject = LveGameObject::createGameObject();
            triangleGameObject.model = lveModel;
            triangleGameObject.color = squares[i].color;
            gameObjects.push_back(std::move(triangleGameObject));
        }
        std::cout << "Num game objects loaded: " << objects.size() << "\n";


        //triangleGameObject.transform2d.translation.x = 0.2f;
        //triangleGameObject.transform2d.scale = {2.0f, 0.5f};
        //triangleGameObject.transform2d.rotation = 0.25f * glm::two_pi<float>();
        //for(int i = 0; i < gameObjects.size(); i++)
        //    gameObjects.push_back(std::move(objects[i]));
    }

}

lve::FirstApp::square lve::FirstApp::MakeSquare(float topLeftX, float topLeftY, float size, float r, float g, float b) {
    lve::FirstApp::square sq = {};

    //float t = ((float)WIDTH / 1000);
    //float scaleX = size * ((float)WIDTH / 1000);
    //float scaleY = size * ((float)HEIGHT / 1000);

    sq.TopLeft.position = glm::vec2(topLeftX, topLeftY);
    sq.TopRight.position = glm::vec2(topLeftX + size, topLeftY);
    sq.BottomLeft.position = glm::vec2(topLeftX, topLeftY + size);
    sq.BottomRight.position = glm::vec2(topLeftX + size, topLeftY + size);

    sq.TopLeft.color = glm::vec3(r, g, b);
    sq.TopRight.color = glm::vec3(r, g, b);
    sq.BottomLeft.color = glm::vec3(r, g, b);
    sq.BottomRight.color = glm::vec3(r, g, b);

    return sq;
}

lve::FirstApp::square lve::FirstApp::MakeSquare(float topLeftX, float topLeftY, float size, int palletColor) {
    lve::FirstApp::square sq = {};

    //float t = ((float)WIDTH / 1000);
    //float scaleX = size * ((float)WIDTH / 1000);

//    //float scaleY = size * ((float)HEIGHT / 1000);

    sq.TopLeft.position = glm::vec2(topLeftX, topLeftY);
    sq.TopRight.position = glm::vec2(topLeftX + size, topLeftY);
    sq.BottomLeft.position = glm::vec2(topLeftX, topLeftY + size);
    sq.BottomRight.position = glm::vec2(topLeftX + size, topLeftY + size);

    sq.TopLeft.color = pallet[palletColor];
    sq.TopRight.color = pallet[palletColor];
    sq.BottomLeft.color = pallet[palletColor];
    sq.BottomRight.color = pallet[palletColor];

    sq.color = pallet[palletColor];

    return sq;
}

void lve::FirstApp::loadFromFile(std::string File) {
    std::string line;
    std::ifstream file(File);


    for(int i = 0; i < 9; i++)
    {
        std::getline(file, line);
        std::stringstream ss(line);
        std::string word;
        std::vector<float> rgb;
        while(!ss.eof())
        {
            std::getline(ss, word, ',');
            rgb.push_back(std::stof(word));
        }
        glm::vec3 color = {rgb[0], rgb[1], rgb[2]};

        pallet.push_back(color);
    }

    for(int i = 0; i < 8; i++)
    {
        std::getline(file, line);
        std::stringstream ss(line);
        std::string word;
        std::vector<int> nums;
        while(!ss.eof())
        {
            std::getline(ss, word, ',');
            nums.push_back(std::stoi(word));
        }
        colorArray.push_back(nums);
    }
}

