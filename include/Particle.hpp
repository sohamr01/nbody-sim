#pragma once
#include "Vector2.hpp" //note: <> is used for system libraries, while "" for my own proj. file

struct Particle
{
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    double mass;
    double charge;
    double radius;
};
