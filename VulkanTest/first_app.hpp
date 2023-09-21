//
// Created by cdgira on 6/30/2023.
//

#ifndef VULKANTEST_FIRST_APP_HPP
#define VULKANTEST_FIRST_APP_HPP

#include "lve_window.hpp"
#include "lve_game_object.hpp"
#include "lve_device.hpp"
#include "lve_renderer.hpp"


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

        // Sprite Stuff

        // Defines a the vertices for
        // a square
        struct square
        {
            LveModel::Vertex TopLeft;
            LveModel::Vertex TopRight;
            LveModel::Vertex BottomLeft;
            LveModel::Vertex BottomRight;
            glm::vec3 color;
        };

        square MakeSquare(float topLeftX, float topLeftY, float size, float r, float g, float b);

        // Create a square of a color in the
        // color pallet from two triangles at
        // starting position topLeftX and
        // topLeftY of size size.
        square MakeSquare(float topLeftX, float topLeftY, float size, int palletColor);

    private:

        // Load every square from the image as
        // Its own game object
        void loadGameObjects();

        // Load a "sprite" from a file
        // Takes a txt file as a parameter and
        // loads the color pallet and color map
        // for the sprite and adds that data to
        // the pallet and collorArray vectors
        void loadFromFile(std::string file);


        LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        LveDevice lveDevice{lveWindow};
        LveRenderer lveRenderer{lveWindow, lveDevice};
        std::vector<LveGameObject> gameObjects;

        //Sprite Stuff

        // Holds the glm::vec3 definitions
        // for colors in an image.
        std::vector<glm::vec3> pallet;

        // Holds the integer values of the
        // colors in a pallet that make the
        // image.
        std::vector<std::vector<int>> colorArray;
    };
}

#endif //VULKANTEST_FIRST_APP_HPP
