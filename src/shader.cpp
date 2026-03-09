#include <epoxy/gl.h>
#include <epoxy/gl_generated.h>

#include <cstdint>
#include <expected>
#include <format>
#include <fstream>
#include <ios>
#include <span>
#include <string>
#include <utility>

#include "shader.hpp"

namespace shader {

// UniqueShader

UniqueShader::UniqueShader(GLuint id) : m_id(id) {}

UniqueShader::UniqueShader(UniqueShader&& other) noexcept
    : m_id(std::exchange(other.m_id, 0U)) {}

UniqueShader& UniqueShader::operator=(UniqueShader&& other) noexcept {
    if (this != &other) {
        reset();
        m_id = std::exchange(other.m_id, 0U);
    }

    return *this;
}

UniqueShader::~UniqueShader() { reset(); }

GLuint UniqueShader::get() const { return m_id; }

UniqueShader::operator bool() const { return m_id != 0U; }

GLuint UniqueShader::release() { return std::exchange(m_id, 0U); }

void UniqueShader::reset() {
    if (m_id != 0U) {
        glDeleteShader(m_id);
        m_id = 0U;
    }
}

// UniqueProgram

UniqueProgram::UniqueProgram(GLuint id) : m_id(id) {}

UniqueProgram::UniqueProgram(UniqueProgram&& other) noexcept
    : m_id(std::exchange(other.m_id, 0U)) {}

UniqueProgram& UniqueProgram::operator=(UniqueProgram&& other) noexcept {
    if (this != &other) {
        reset();
        m_id = std::exchange(other.m_id, 0U);
    }
    return *this;
}

UniqueProgram::~UniqueProgram() { reset(); }

GLuint UniqueProgram::get() const { return m_id; }

UniqueProgram::operator bool() const { return m_id != 0U; }

GLuint UniqueProgram::release() { return std::exchange(m_id, 0U); }

void UniqueProgram::reset() {
    if (m_id != 0U) {
        glDeleteProgram(m_id);
        m_id = 0U;
    }
}

namespace impl {
std::expected<std::string, Error> read_text_file(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);

    if (!in) {
        return std::unexpected(Error {
            .stage   = ErrorStage::ReadFile,
            .message = std::format("Error reading file: {}", path.string()),
            .log     = "",
        });
    }

    in.seekg(0, std::ios::end);
    std::string out(static_cast<size_t>(in.tellg()), '\0');
    in.seekg(0, std::ios::beg);
    in.read(out.data(), static_cast<int64_t>(out.size()));

    return out;
}

std::string get_shader_info_log(GLuint shader) {
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

    if (len <= 1) {
        return {};
    }

    std::string log(static_cast<size_t>(len), '\0');
    GLsizei written = 0;
    glGetShaderInfoLog(shader, len, &written, log.data());

    if (written > 0 && static_cast<size_t>(written) <= log.size()) {
        log.resize(static_cast<size_t>(written));
    }

    return log;
}

std::string get_program_info_log(GLuint program) {
    GLint len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

    if (len <= 1) {
        return {};
    }

    std::string log(static_cast<size_t>(len), '\0');
    GLsizei written = 0;
    glGetProgramInfoLog(program, len, &written, log.data());

    if (written > 0 && static_cast<size_t>(written) <= log.size()) {
        log.resize(static_cast<size_t>(written));
    }

    return log;
}
}  // namespace impl

std::expected<std::string, Error> load_source(const std::filesystem::path& path) {
    return impl::read_text_file(path);
}

std::expected<UniqueShader, Error> compile_from_source(GLenum shader_type,
                                                       std::string_view shader_source) {
    GLuint id = glCreateShader(shader_type);
    if (id == 0U) {
        return std::unexpected(Error {
            .stage   = ErrorStage::Compile,
            .message = "glCreateShader returned 0",
            .log     = "",
        });
    }

    const char* src_ptr = shader_source.data();
    auto src_length     = static_cast<GLint>(shader_source.size());
    glShaderSource(id, 1, &src_ptr, &src_length);
    glCompileShader(id);

    GLint success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE) {
        std::string log = impl::get_shader_info_log(id);
        glDeleteShader(id);
        return std::unexpected(Error {
            .stage   = ErrorStage::Compile,
            .message = "Shader compilation failed",
            .log     = std::move(log),
        });
    }

    return UniqueShader(id);
}

std::expected<UniqueProgram, Error> link_program(
    std::span<const GLuint, std::dynamic_extent> shaders) {
    GLuint id = glCreateProgram();
    if (id == 0U) {
        return std::unexpected(Error {
            .stage   = ErrorStage::Link,
            .message = "glCreateProgram returned 0",
            .log     = "",
        });
    }

    for (GLuint shader : shaders) {
        glAttachShader(id, shader);
    }

    glLinkProgram(id);

    GLint success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (success == GL_FALSE) {
        std::string log = impl::get_program_info_log(id);
        glDeleteProgram(id);
        return std::unexpected(Error {
            .stage   = ErrorStage::Link,
            .message = "Program linking failed",
            .log     = std::move(log),
        });
    }

    return UniqueProgram(id);
}

std::expected<UniqueProgram, Error> load_program(
    const std::filesystem::path& vertex_shader_path,
    const std::filesystem::path& fragment_shader_path) {
    auto vertex_source = load_source(vertex_shader_path);
    if (!vertex_source) {
        return std::unexpected(vertex_source.error());
    }

    auto fragment_source = load_source(fragment_shader_path);
    if (!fragment_source) {
        return std::unexpected(fragment_source.error());
    }

    auto vertex_shader = compile_from_source(GL_VERTEX_SHADER, *vertex_source);
    if (!vertex_shader) {
        return std::unexpected(vertex_shader.error());
    }

    auto fragment_shader = compile_from_source(GL_FRAGMENT_SHADER, *fragment_source);
    if (!fragment_shader) {
        return std::unexpected(fragment_shader.error());
    }

    const GLuint shader_ids[] = { vertex_shader->get(), fragment_shader->get() };
    return link_program(shader_ids);
}

}  // namespace shader
