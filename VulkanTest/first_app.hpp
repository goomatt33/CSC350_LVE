//
// Created by cdgira on 6/30/2023.
//

#ifndef VULKANTEST_FIRST_APP_HPP
#define VULKANTEST_FIRST_APP_HPP

#include "lve_window.hpp"
#include "lve_game_object.hpp"
#include "lve_device.hpp"
#include "lve_renderer.hpp"
#include "lve_descriptors.hpp"
#include "lve_image.hpp"
#include "gameObjects/Actor.h"


#include <memory>
#include <vector>

namespace lve {
    class FirstApp {

    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp &operator=(const FirstApp&) = delete;

        void run();

    private:
        void loadGameObjects();

        Actor* createActor(std::string file, std::string name, int textureBinding,
                           glm::vec3 translation = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3 (0.f),
                           glm::vec3 scale = glm::vec3(1.0f));

        LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        LveDevice lveDevice{lveWindow};
        std::shared_ptr<LveImage> textureImage = LveImage::createImageFromFile(lveDevice, "../textures/Ch_Mai_95_D.png");

        std::vector<std::shared_ptr<LveImage>> texVec;
        // Here we need to setup the TextureMapping.
        //createTextureImage();
        //createTextureImageView();
        // Here is where they create the VertexBuffer, IndexBuffer, and UniformBuffer.
        LveRenderer lveRenderer{lveWindow, lveDevice};

        //note: Order of declaration is important.
        std::unique_ptr<LveDescriptorPool> globalPool{};
        LveGameObject::Map gameObjects;

        // Vector of game actors for updating
        std::vector<Actor*> actors;
    };
}

#endif //VULKANTEST_FIRST_APP_HPP
