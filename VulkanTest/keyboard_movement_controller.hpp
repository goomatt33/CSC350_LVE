//
// Created by cdgira on 7/25/2023.
//

#ifndef VULKANTEST_KEYBOARD_MOVEMENT_CONTROLLER_HPP
#define VULKANTEST_KEYBOARD_MOVEMENT_CONTROLLER_HPP

#include "lve_game_object.hpp"
#include "lve_window.hpp"

namespace lve {
    class KeyboardMovementController {

    public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
            int lookAtOrigin = GLFW_KEY_SPACE;
            int panKey = GLFW_KEY_R;
        };

        void moveInPlaneXZ(GLFWwindow* window, float deltaTime, LveGameObject& gameObject);



        float orbitSpeed = 2.0f;
        float rotAngle = 0.0f;

        bool shouldPan = false;


        KeyMappings keys{};
        float moveSpeed{3.0f};
        float lookSpeed{1.5f};
    };
}

#endif //VULKANTEST_KEYBOARD_MOVEMENT_CONTROLLER_HPP