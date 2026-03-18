#pragma once //prevents a header file from being included multiple times
#include "Simulation.hpp"
#include <vector>


class Integrator {
    public:
    virtual void step(Simulation& sim, double dt) = 0; //making Integrator class an abstract interface (=0 means that the function has no implementation here)
};

class ExplicitEuler : public Integrator {
    public:
    void step(Simulation& sim, double dt) override {
        int i;
        sim.computeForces();
        std::vector<Vector2> oldVelocities;
        oldVelocities.reserve(sim.particles.size());
        for(i = 0; i<sim.particles.size(); i++) {
            oldVelocities.push_back(sim.particles[i].velocity);
        }
        for(i = 0; i<sim.particles.size(); i++) { //velo update
            sim.particles[i].velocity = sim.particles[i].velocity.add(sim.particles[i].acceleration.multiply(dt));
        }
        for(i = 0; i<sim.particles.size(); i++) { //pos update
            sim.particles[i].position = sim.particles[i].position.add(oldVelocities[i].multiply(dt));
        }
        //resolve overlap and bounce after positions are updated
        sim.handleCollisions();

    }
};

class SemiImplicitEuler : public Integrator {
    public:
    void step(Simulation& sim, double dt) override {
        int i; 
        sim.computeForces();
        for(i = 0; i<sim.particles.size(); i++) {
            sim.particles[i].velocity = sim.particles[i].velocity.add(sim.particles[i].acceleration.multiply(dt));
        }
        for(i = 0; i<sim.particles.size(); i++) {
            sim.particles[i].position = sim.particles[i].position.add(sim.particles[i].velocity.multiply(dt));
        }
        //resolve overlap and bounce after positions are updated
        sim.handleCollisions();
    }
};