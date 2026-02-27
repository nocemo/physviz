#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader() = default;
    ~Shader();

    // ファイルから読み込み（頂点/フラグメント）
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    // 既に読み込んだ/作ったシェーダを破棄
    void destroy();

    // 使用開始
    void use() const;

    // program id
    unsigned int id() const { return m_program; }
    bool valid() const { return m_program != 0; }

    // uniform helpers
    void setInt(const char* name, int v);
    void setFloat(const char* name, float v);
    void setVec3(const char* name, const glm::vec3& v);
    void setMat4(const char* name, const glm::mat4& m);

private:
    unsigned int m_program = 0;
    std::unordered_map<std::string, int> m_uniformLocCache;

    static std::string readTextFile(const std::string& path);
    static unsigned int compileShader(unsigned int type, const std::string& source, std::string* outLog);
    static unsigned int linkProgram(unsigned int vs, unsigned int fs, std::string* outLog);

    int getUniformLocation(const char* name);
    void clearCache();
};
