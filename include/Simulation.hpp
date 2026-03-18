#pragma once
#include <vector>
#include "Particle.hpp"

/* this owns all particles, timestep, force calculations, integration, and diagnostics.
it basically contains a bunch of particles and updates them over time*/

class Simulation {
    public:
    std::vector <Particle> particles;

    void addParticle(const Particle& p) {
        particles.push_back(p);
    }

    void computeForces() {
        int i;
        int j;
        double G = 0.5; //arbitrary sim scale gravitational constant
        for(i = 0; i<particles.size(); i++) {
            particles[i].acceleration = Vector2(0.0,0.0); //resetting acceleration for every particle at the start of each frame
        }

        for(i = 0; i<particles.size(); i++) {
            for(j = i+1; j<particles.size(); j++) {
                Vector2 r = particles[j].position.subtract(particles[i].position); //direction vector from particle i to j
                double distance = r.magnitude();
                if (distance < 1e-8) { //so that 1/r^2 doesn't create an absurdly large force at super low r 
                    continue;
                }
                double forceMagnitude = (G * particles[i].mass * particles[j].mass) / (distance * distance + 0.2 * 0.2); 
                Vector2 direction = r.normalise(); // unit direction vector
                Vector2 force = direction.multiply(forceMagnitude); //so now we have a force vector
                //updating accelerations. note that we reset acceleration once per frame, then accumulate all pairwise force contributions during that frame
                particles[i].acceleration = particles[i].acceleration.add(
                    force.multiply(1.0 / particles[i].mass)
                );
                particles[j].acceleration = particles[j].acceleration.add(
                    force.multiply(-1.0 / particles[j].mass)
                );

            }
        }
    }

    void update(double dt) {
        int i;
        computeForces(); 
        for(i = 0; i<particles.size(); i++) { //velocity update
            particles[i].velocity = particles[i].velocity.add(particles[i].acceleration.multiply(dt)); //v = u+at
        }

        for(i = 0; i<particles.size(); i++) { //position update
            particles[i].position = particles[i].position.add(particles[i].velocity.multiply(dt));
        }
        handleCollisions();
    }

    void handleCollisions() {
        for(int i = 0; i<particles.size(); i++) {
            for(int j = i+1; j<particles.size(); j++) {
                Vector2 r = particles[j].position.subtract(particles[i].position); //direction vector from particle i to j
                double distance = r.magnitude();
                if (distance <= particles[i].radius + particles[j].radius) { //if collision
                    if(distance < 1e-8) {
                        continue;
                    }
                    Vector2 normal = r.normalise();
                    Vector2 v_rel = particles[j].velocity.subtract(particles[i].velocity); //relative vel
                    double v_alongNormal = v_rel.dot(normal);
                    if(v_alongNormal > 0) { //if they're already moving apart then no collision response necessary
                        continue;
                    }

                    //computing impulse - perfectly elastic collision, restitution coefficient e = 1
                    double impulseScalar = -(1+1) * v_alongNormal / ((1.0 / particles[i].mass) + (1.0 / particles[j].mass));
                    Vector2 impulse = normal.multiply(impulseScalar);
                    particles[i].velocity = particles[i].velocity.subtract(impulse.multiply(1.0 / particles[i].mass));
                    particles[j].velocity = particles[j].velocity.add(impulse.multiply(1.0 / particles[j].mass));

                    double overlap = (particles[i].radius + particles[j].radius) - distance; //how much two particles penetrate into each other
                    Vector2 correction_factor = normal.multiply(overlap / 2.0);
                    particles[i].position = particles[i].position.subtract(correction_factor);
                    particles[j].position = particles[j].position.add(correction_factor);
                }
            }
        }
        
    }

    //energy diagnostics: 

    double kineticEnergy() {
        double total = 0.0;
        for(int i = 0; i<particles.size(); i++) {
            double speed = particles[i].velocity.magnitude();
            double k_e = 0.5 * particles[i].mass * speed * speed; 
            total += k_e;
        }
        return total;
    }

    double potentialEnergy() {
        double total = 0.0;
        double G = 0.5; //arbitrary sim scale gravitational constant
        for(int i = 0; i<particles.size(); i++) {
            for(int j = i+1; j<particles.size(); j++) {
                Vector2 r = particles[j].position.subtract(particles[i].position); //direction vector from particle i to j
                double distance = r.magnitude();
                if(distance < 1e-8) {
                    continue;
                }
                total += (-G * particles[i].mass * particles[j].mass) / (distance); //formula for GPE (-Gm1m2/r)
            }
        }
        return total;
    }

Vector2 totalMomentum() {
    Vector2 total = Vector2(0.0, 0.0);
    for(int i = 0; i < particles.size(); i++) {
        Vector2 particleMomentum = particles[i].velocity.multiply(particles[i].mass);
        total = total.add(particleMomentum);
    }
    return total;
}
};
