#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include "Simulation.hpp"
#include "Integrator.hpp"

struct Renderer {
    GLuint shaderProgram = 0;
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLint colorUniformLocation = -1;
    GLint pointSizeUniformLocation = -1;
    std::array<std::array<float, 3>, 6> palette = {{
        {{0.95f, 0.85f, 0.25f}},
        {{0.25f, 0.80f, 0.95f}},
        {{0.95f, 0.35f, 0.45f}},
        {{0.45f, 0.95f, 0.35f}},
        {{0.85f, 0.45f, 0.95f}},
        {{0.95f, 0.60f, 0.20f}}
    }};

    static GLuint compileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation failed: " << infoLog << "\n";
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    static GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

        if (vertexShader == 0 || fragmentShader == 0) {
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Shader link failed: " << infoLog << "\n";
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(program);
            return 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

    bool initialize() {
        const char* vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec2 aPos;
uniform float uPointSize;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    gl_PointSize = uPointSize;
}
)";

        const char* fragmentShaderSource = R"(#version 330 core
uniform vec3 uColor;
out vec4 FragColor;

void main() {
    vec2 coord = gl_PointCoord - vec2(0.5, 0.5);
    float dist = length(coord);
    float alpha = 1.0 - smoothstep(0.35, 0.5, dist);
    if (alpha <= 0.01) {
        discard;
    }
    FragColor = vec4(uColor, alpha);
}
)";

        shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
        if (shaderProgram == 0) {
            return false;
        }
        colorUniformLocation = glGetUniformLocation(shaderProgram, "uColor");
        pointSizeUniformLocation = glGetUniformLocation(shaderProgram, "uPointSize");

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return true;
    }

    void drawParticlePoint(const Vector2& position,
                           const std::array<float, 3>& color,
                           float pointSizePixels) {
        float pointData[2] = {
            static_cast<float>(position.x),
            static_cast<float>(position.y)
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(pointData), pointData, GL_DYNAMIC_DRAW);
        glUniform3f(colorUniformLocation, color[0], color[1], color[2]);
        glUniform1f(pointSizeUniformLocation, pointSizePixels);
        glDrawArrays(GL_POINTS, 0, 1);
    }

    void draw(const std::vector<Particle>& particles,
              const std::vector<std::vector<Vector2>>& trailHistory,
              int framebufferWidth,
              int framebufferHeight) {
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        glClearColor(0.03f, 0.03f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        int referencePixels = framebufferWidth;

        for (size_t i = 0; i < trailHistory.size(); i++) {
            const std::array<float, 3>& baseColor = palette[i % palette.size()];
            const std::vector<Vector2>& trail = trailHistory[i];

            for (size_t k = 0; k < trail.size(); k++) {
                float t = static_cast<float>(k + 1) / static_cast<float>(trail.size());
                std::array<float, 3> trailColor = {
                    baseColor[0] * (0.25f + 0.50f * t),
                    baseColor[1] * (0.25f + 0.50f * t),
                    baseColor[2] * (0.25f + 0.50f * t)
                };
                float trailSize = 4.0f + 8.0f * t;
                drawParticlePoint(trail[k], trailColor, trailSize);
            }
        }

        for (size_t i = 0; i < particles.size(); i++) {
            const std::array<float, 3>& color = palette[i % palette.size()];
            float pointSizePixels = static_cast<float>(particles[i].radius) * static_cast<float>(referencePixels) * 1.08f;
            if (pointSizePixels < 8.0f) {
                pointSizePixels = 8.0f;
            }
            drawParticlePoint(particles[i].position, color, pointSizePixels);
        }
    }

    void cleanup() {
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
        }
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
        }
        if (shaderProgram != 0) {
            glDeleteProgram(shaderProgram);
        }
    }
};

int main() {

    //fail outputs
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "NBody Simulator", NULL, NULL);

    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glEnable(GL_PROGRAM_POINT_SIZE);

    Renderer renderer;
    if (!renderer.initialize()) {
        std::cerr << "Failed to initialize renderer\n";
        glfwTerminate();
        return -1;
    }

    //sim setup starts here!!
    Simulation sim;

    Particle p1;
    p1.position = Vector2(-0.5, 0.0);
    p1.velocity = Vector2(0.0, 0.0);
    p1.acceleration = Vector2(0.0, 0.0);
    p1.mass = 1.0;
    p1.charge = 0.0;
    p1.radius = 0.05;

    Particle p2;
    p2.position = Vector2(0.5, 0.0);
    p2.velocity = Vector2(0.0, 0.0);
    p2.acceleration = Vector2(0.0, 0.0);
    p2.mass = 1.0;
    p2.charge = 0.0;
    p2.radius = 0.05;

    sim.addParticle(p1);
    sim.addParticle(p2);

    double dt = 0.001;

    SemiImplicitEuler integrator; 
    int frameCount = 0;
    std::vector<std::vector<Vector2>> trailHistory(sim.particles.size());
    const size_t maxTrailLength = 90;

    std::cout << "N-Body Simulator started successfully.\n";
    std::cout << "Integrator: Semi-Implicit Euler | dt: " << dt << "\n";
    std::cout << "Particles: " << sim.particles.size() << " | Trails: enabled\n";
    std::cout << "Diagnostics: printing every 120 frames\n\n";
    
    while (!glfwWindowShouldClose(window)) {
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        integrator.step(sim, dt);

        for (size_t i = 0; i < sim.particles.size(); i++) {
            trailHistory[i].push_back(sim.particles[i].position);
            if (trailHistory[i].size() > maxTrailLength) {
                trailHistory[i].erase(trailHistory[i].begin());
            }
        }

        frameCount++;

        double kineticEnergy = sim.kineticEnergy();
        double potentialEnergy = sim.potentialEnergy();
        double totalEnergy = kineticEnergy + potentialEnergy;

        if (frameCount % 120 == 0) {
            Vector2 totalMomentum = sim.totalMomentum();

            std::ostringstream diagnostics;
            diagnostics << std::fixed << std::setprecision(6)
                        << "Frame " << std::setw(6) << frameCount
                        << " | KE: " << std::setw(10) << kineticEnergy
                        << " | PE: " << std::setw(10) << potentialEnergy
                        << " | TE: " << std::setw(10) << totalEnergy
                        << " | P: (" << totalMomentum.x << ", " << totalMomentum.y << ")";

            std::cout << diagnostics.str() << std::endl;
        }

        if (frameCount % 15 == 0) {
            std::ostringstream title;
            title << std::fixed << std::setprecision(4)
                  << "N-Body Simulator | Gravity | Semi-Implicit Euler | KE =" << kineticEnergy;
            glfwSetWindowTitle(window, title.str().c_str());
        }

        renderer.draw(sim.particles, trailHistory, framebufferWidth, framebufferHeight);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer.cleanup();

    glfwTerminate();
}