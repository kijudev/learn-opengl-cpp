// clang-format off
#include <epoxy/gl.h>
#include <GLFW/glfw3.h>
#include <epoxy/gl_generated.h>
// clang-format on

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <print>

#include "shader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

struct Camera {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 front    = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up       = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw        = -90.0f;
    float pitch      = 0.0f;
    float last_x     = 800.0f / 2.0f;
    float last_y     = 600.0f / 2.0f;
    bool first_mouse = true;
};

Camera camera;

void error_callback(int32_t error, const char* description) {
    std::println(stderr, "GLFW Error ({}): {}", error, description);
}

void framebuffer_size_callback(GLFWwindow*, int32_t width, int32_t height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow*, double xpos_arg, double ypos_arg) {
    float xpos = static_cast<float>(xpos_arg);
    float ypos = static_cast<float>(ypos_arg);

    float xoff    = xpos - camera.last_x;
    float yoff    = -(ypos - camera.last_y);
    camera.last_x = xpos;
    camera.last_y = ypos;

    float sensitivity = 0.1f;
    xoff *= sensitivity;
    yoff *= sensitivity;

    camera.yaw += xoff;
    camera.pitch = std::clamp(camera.pitch + yoff, -89.0f, 89.0f);

    glm::vec3 front = glm::vec3(
        std::cos(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch)),
        std::sin(glm::radians(camera.pitch)),
        std::sin(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch)));

    camera.front = glm::normalize(front);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

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

    uint32_t tex {};
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int32_t tex_width {}, tex_height {}, tex_nr_channels {};
    unsigned char* tex_data =
        stbi_load("assets/wall.jpg", &tex_width, &tex_height, &tex_nr_channels, 0);

    if (tex_data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, tex_data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::println("Failed to load the texture!");
        return EXIT_FAILURE;
    }

    stbi_image_free(tex_data);
    glBindTexture(GL_TEXTURE_2D, tex);

    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  // <-
        0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,  // <-
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,  // <-
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,  // <-
        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f,  // <-
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  // <-

        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,  // <-
        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,  // <-
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  // <-
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  // <-
        -0.5f, 0.5f,  0.5f,  0.0f, 1.0f,  // <-
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,  // <-

        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,  // <-
        -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,  // <-
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // <-
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // <-
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,  // <-
        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,  // <-

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // <-
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,  // <-
        0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,  // <-
        0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,  // <-
        0.5f,  -0.5f, 0.5f,  0.0f, 0.0f,  // <-
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // <-

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // <-
        0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,  // <-
        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,  // <-
        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,  // <-
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,  // <-
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // <-

        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f,  // <-
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,  // <-
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // <-
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // <-
        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f,  // <-
        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f   // <-
    };

    uint32_t vbo;
    uint32_t vao;

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          static_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glUseProgram(program->get());
    glUniform1i(glGetUniformLocation(program->get(), "ourTexture"), 0);

    glEnable(GL_DEPTH_TEST);

    glm::vec3 cube_positions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)
    };

    float camera_speed = 10.0f;

    float last_frame {};
    float delta_time {};

    while (!glfwWindowShouldClose(window)) {
        float time = static_cast<float>(glfwGetTime());
        delta_time = time - last_frame;
        last_frame = time;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera.position += camera_speed * camera.front * delta_time;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera.position -= camera_speed * camera.front * delta_time;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) *
                               camera_speed * delta_time;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera.position += glm::normalize(glm::cross(camera.front, camera.up)) *
                               camera_speed * delta_time;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            camera.position += camera_speed * camera.up * delta_time;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            camera.position -= camera_speed * camera.up * delta_time;
        }

        glClearColor(0.9f, 0.95f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program->get());

        float rainbow_color_green    = std::sin(time) / 2.0f + 0.5f;
        float rainbow_color_red      = std::sin(time / 3.0f) / 2.0f + 0.5f;
        float rainbow_color_blue     = std::sin(time / 4.0f) / 2.0f + 0.5f;
        GLint rainbow_color_location = glGetUniformLocation(program->get(), "rainbowColor");
        glUniform4f(rainbow_color_location, rainbow_color_green, rainbow_color_red,
                    rainbow_color_blue, 1.0f);

        glm::mat4 view =
            glm::lookAt(camera.position, camera.position + camera.front, camera.up);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        GLint model_uniform = glGetUniformLocation(program->get(), "model");

        GLint view_uniform = glGetUniformLocation(program->get(), "view");
        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));

        GLint projection_uniform = glGetUniformLocation(program->get(), "projection");
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(vao);

        for (const glm::vec3& pos : cube_positions) {
            glm::mat4 model = glm::mat4(1.0f);
            model           = glm::translate(model, pos);
            model = glm::rotate(model, static_cast<float>(time) * glm::radians(-55.0f),
                                glm::vec3(0.5f, 0.7f, 0.3f));

            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    program->reset();

    glfwTerminate();

    return EXIT_SUCCESS;
}
