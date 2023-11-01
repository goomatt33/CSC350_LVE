//
// Created by mb6457 on 10/31/23.
//

#ifndef COMPGRAPH_MOVINGGAMEOBJECT_H
#define COMPGRAPH_MOVINGGAMEOBJECT_H

#include "lve_game_object.hpp"

namespace lve {

    class MovingGameObject : public LveGameObject
    {
    public:
        float movementSpeed;
        static MovingGameObject createGameObject() {
            static id_t currentId = 0;
            return MovingGameObject{currentId++};
        }

        MovingGameObject(const MovingGameObject&) = delete;
        MovingGameObject &operator=(const MovingGameObject&) = delete;
        MovingGameObject(MovingGameObject&&) = default;
        MovingGameObject &operator=(MovingGameObject&&) = default;


    protected:
        MovingGameObject(id_t id) : LveGameObject(id) {}
    };

} // lve

#endif //COMPGRAPH_MOVINGGAMEOBJECT_H
