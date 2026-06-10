#pragma once
#include "Enemigo.hpp"

class Blob : public Enemigo {
public:
    Blob(float x, float y);

    void actualizar(float dt) override;
};