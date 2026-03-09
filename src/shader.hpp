#pragma once

#include <epoxy/gl.h>
#include <epoxy/gl_generated.h>

#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace shader {
enum class ErrorStage : uint8_t {
    ReadFile,
    Compile,
    Link,
};

struct Error {
    ErrorStage stage {};
    std::string message;
    std::string log;
};

namespace impl {
[[nodiscard]] std::expected<std::string, Error> read_text_file(
    const std::filesystem::path& path);

[[nodiscard]] std::string get_shader_info_log(GLuint shader);
[[nodiscard]] std::string get_program_info_log(GLuint program);
}  // namespace impl

class UniqueShader {
   public:
    UniqueShader() = default;
    UniqueShader(GLuint id);

    UniqueShader(const UniqueShader&)            = delete;
    UniqueShader& operator=(const UniqueShader&) = delete;

    UniqueShader(UniqueShader&& other) noexcept;
    UniqueShader& operator=(UniqueShader&& other) noexcept;

    ~UniqueShader();

    [[nodiscard]] GLuint get() const;
    [[nodiscard]] explicit operator bool() const;

    GLuint release();
    void reset();

   private:
    GLuint m_id = 0U;
};

class UniqueProgram {
   public:
    UniqueProgram() = default;
    UniqueProgram(GLuint id);

    UniqueProgram(const UniqueProgram&)            = delete;
    UniqueProgram& operator=(const UniqueProgram&) = delete;

    UniqueProgram(UniqueProgram&& other) noexcept;
    UniqueProgram& operator=(UniqueProgram&& other) noexcept;

    ~UniqueProgram();

    [[nodiscard]] GLuint get() const;
    [[nodiscard]] explicit operator bool() const;

    GLuint release();
    void reset();

   private:
    GLuint m_id = 0U;
};

[[nodiscard]] std::expected<std::string, Error> load_source(
    const std::filesystem::path& path);

[[nodiscard]] std::expected<UniqueShader, Error> compile_from_source(
    GLenum shader_type, std::string_view shader_source);

[[nodiscard]] std::expected<UniqueProgram, Error> link_program(
    std::span<const GLuint, std::dynamic_extent> shaders);

[[nodiscard]] std::expected<UniqueProgram, Error> load_program(
    const std::filesystem::path& vertex_shader_path,
    const std::filesystem::path& fragment_shader_path);
}  // namespace shader
