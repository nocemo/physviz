#include "Shader.h"

#include <glad/glad.h>  // あなたのプロジェクトに合わせて glad/glad.h
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

Shader::~Shader()
{
    destroy();
}

void Shader::destroy()
{
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    clearCache();
}

void Shader::clearCache()
{
    m_uniformLocCache.clear();
}

std::string Shader::readTextFile(const std::string& path)
{
    std::ifstream ifs(path, std::ios::in);
    if (!ifs) {
        return {};
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source, std::string* outLog)
{
    if (outLog) outLog->clear();

    const char* src = source.c_str();
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        if (len > 1) glGetShaderInfoLog(sh, len, nullptr, log.data());

        if (outLog) *outLog = log;
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

unsigned int Shader::linkProgram(unsigned int vs, unsigned int fs, std::string* outLog)
{
    if (outLog) outLog->clear();

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        if (len > 1) glGetProgramInfoLog(prog, len, nullptr, log.data());

        if (outLog) *outLog = log;
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath)
{
    destroy();

    const std::string vsSrc = readTextFile(vertexPath);
    const std::string fsSrc = readTextFile(fragmentPath);

    if (vsSrc.empty() || fsSrc.empty()) {
        std::cerr << "[Shader] Failed to read shader files:\n"
                  << "  VS: " << vertexPath << "\n"
                  << "  FS: " << fragmentPath << "\n";
        return false;
    }

    std::string logVS, logFS, logLink;

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vsSrc, &logVS);
    if (!vs) {
        std::cerr << "[Shader] Vertex shader compile failed:\n" << logVS << "\n";
        return false;
    }

    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fsSrc, &logFS);
    if (!fs) {
        std::cerr << "[Shader] Fragment shader compile failed:\n" << logFS << "\n";
        glDeleteShader(vs);
        return false;
    }

    unsigned int prog = linkProgram(vs, fs, &logLink);

    // もう不要
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!prog) {
        std::cerr << "[Shader] Program link failed:\n" << logLink << "\n";
        return false;
    }

    m_program = prog;
    clearCache();
    return true;
}

void Shader::use() const
{
    glUseProgram(m_program);
}

int Shader::getUniformLocation(const char* name)
{
    auto it = m_uniformLocCache.find(name);
    if (it != m_uniformLocCache.end()) return it->second;

    int loc = glGetUniformLocation(m_program, name);
    m_uniformLocCache.emplace(name, loc);
    return loc;
}

void Shader::setInt(const char* name, int v)
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform1i(loc, v);
}

void Shader::setFloat(const char* name, float v)
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform1f(loc, v);
}

void Shader::setVec3(const char* name, const glm::vec3& v)
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform3fv(loc, 1, glm::value_ptr(v));
}

void Shader::setMat4(const char* name, const glm::mat4& m)
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}
