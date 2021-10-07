#pragma once
#include <string>
#include "cgmath.h"

class Shader
{
    public:
    Shader(std::string vertexShaderFile, std::string fragmentShaderFile);
    ~Shader();
    void activate();
    void setMatrix(const char*, Matrix);
    void setVector3(const char*, Vector3);

    private:
    unsigned int shaderProgram;
    bool shaderCompile(const char* filename, unsigned int* shader, unsigned int type);
    char* readFile(const char* filename);
};
