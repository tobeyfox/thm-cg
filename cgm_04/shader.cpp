#include "shader.h"
#include <iostream>
#include <cstring>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

Shader::Shader(std::string vertexShaderFile, std::string fragmentShaderFile)
{
    int success;
    char infoLog[512];

    unsigned int vertexShader, fragmentShader;
    if (!shaderCompile(vertexShaderFile.c_str(), &vertexShader, GL_VERTEX_SHADER))
    {
        return;
    }

    if (!shaderCompile(fragmentShaderFile.c_str(), &fragmentShader, GL_FRAGMENT_SHADER))
    {
        return;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Failed to link shader program" << std::endl << infoLog << std::endl;
        return;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::activate()
{
    glUseProgram(shaderProgram);
}

void Shader::setMatrix(const char* key, Matrix m)
{
    GLint location = glGetUniformLocation(shaderProgram, key);
    glUniformMatrix4fv(location, 1, GL_FALSE, m.values);
}

void Shader::setVector3(const char* key, Vector3 v)
{
    GLint location = glGetUniformLocation(shaderProgram, key);
    glUniform3fv(location, 1, v.values);
}

bool Shader::shaderCompile(const char* filename, unsigned int* shader, unsigned int type)
{
    char * shaderSource = readFile(filename);

    int success;
    char infoLog[512];

    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &shaderSource, NULL);
    glCompileShader(*shader);
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(*shader, 512, NULL, infoLog);
        std::cout << "Failed to compile shader: " << filename << std::endl << infoLog << std::endl;
        return false;
    }
    return true;
}

char* Shader::readFile(const char* filename)
{
    char * shaderSource = 0;
    long length;
    FILE * f = fopen (filename, "rb");

    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        shaderSource = (char*)malloc (length+1);
        if (shaderSource)
        {
            fread (shaderSource, 1, length, f);
            shaderSource[length] = '\0';
        }
        fclose (f);
        return shaderSource;
    }
    return 0;
}

Shader::~Shader()
{
    glDeleteProgram(shaderProgram);
}