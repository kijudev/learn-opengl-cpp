// clang-format off
#include <epoxy/gl.h>
#include <GLFW/glfw3.h>
#include <epoxy/gl_generated.h>
// clang-format on

#include <cstdint>
#include <cstdlib>
#include <print>

const char* triangle_vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec3 pos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n"
    "}\n";

const char* triangle_fragment_shader_source =
    "#version 330 core\n"
    "out vec4 color;\n"
    "void main() {\n"
    "    color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n";

void error_callback(int error, const char* description) {
    std::println(stderr, "GLFW Error ({}): {}", error, description);
}

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
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

    float vertices[] { -0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f };

    uint32_t vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    uint32_t triangle_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(triangle_vertex_shader, 1, &triangle_vertex_shader_source, nullptr);
    glCompileShader(triangle_vertex_shader);

    int32_t shader_success;
    char shader_info_log[512];
    glGetShaderiv(triangle_vertex_shader, GL_COMPILE_STATUS, &shader_success);

    if (!shader_success) {
        glGetShaderInfoLog(triangle_vertex_shader, 512, nullptr, shader_info_log);
        std::println("ERROR::SHADER::VERTEX::COMPILATION_FAILED -> {}", shader_info_log);
    }

    uint32_t triangle_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(triangle_fragment_shader, 1, &triangle_fragment_shader_source, nullptr);
    glCompileShader(triangle_fragment_shader);

    glGetShaderiv(triangle_fragment_shader, GL_COMPILE_STATUS, &shader_success);
    if (!shader_success) {
        glGetShaderInfoLog(triangle_fragment_shader, 512, nullptr, shader_info_log);
        std::println("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED -> {}", shader_info_log);
    }

    uint32_t triangle_shader_program = glCreateProgram();
    glAttachShader(triangle_shader_program, triangle_vertex_shader);
    glAttachShader(triangle_shader_program, triangle_fragment_shader);
    glLinkProgram(triangle_shader_program);

    glGetProgramiv(triangle_shader_program, GL_LINK_STATUS, &shader_success);
    if (!shader_success) {
        glGetProgramInfoLog(triangle_shader_program, 512, nullptr, shader_info_log);
        std::println("ERROR::SHADER::PROGRAM::LINKING_FAILED -> {}", shader_info_log);
    }

    glUseProgram(triangle_shader_program);
    glDeleteShader(triangle_vertex_shader);
    glDeleteShader(triangle_fragment_shader);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
