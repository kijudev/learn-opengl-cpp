// clang-format off
#include <epoxy/gl.h>
#include <GLFW/glfw3.h>
#include <epoxy/gl_generated.h>
// clang-format on

#include <cstdint>
#include <cstdlib>
#include <print>

#include "shader.hpp"

void error_callback(int32_t error, const char* description) {
    std::println(stderr, "GLFW Error ({}): {}", error, description);
}

void framebuffer_size_callback(GLFWwindow*, int32_t width, int32_t height) {
    glViewport(0, 0, width, height);
}

int main() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        std::println(stderr, "Failed to initialize GLFW");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window =
        glfwCreateWindow(800, 600, "LearnOpenGL - C++23 & Epoxy", nullptr, nullptr);

    if (!window) {
        std::println(stderr, "Failed to create GLFW window");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::println("Successfully loaded OpenGL Version: {}",
                 glVersion ? glVersion : "Unknown");

    auto program = shader::load_program("shaders/triangle.vert", "shaders/triangle.frag");
    if (!program) {
        const shader::Error error = program.error();
        std::println(stderr, "Shader error: {}; {}", error.message, error.log);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    float vertices[] {
        -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f,
    };

    uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };

    uint32_t vbo;
    uint32_t vao;
    uint32_t ebo;

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program->get());
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, static_cast<void*>(0));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    program->reset();

    glfwTerminate();

    return EXIT_SUCCESS;
}
