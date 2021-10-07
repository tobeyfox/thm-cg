#include "model.h"
#include "filereader.h"
#include <iostream>

Model::Model(std::string filename)
{
    FileReader* reader = new FileReader(filename);
    while (reader->hasLine())
    {
        std::string type = reader->getString();
        if (type == "m")
        {
            //mesh
            std::string m = reader->getString();
            this->mesh = new Mesh(m);
        }
        else if (type == "s")
        {
            //shader
            std::string vs = reader->getString();
            std::string fs = reader->getString();
            this->shader = new Shader(vs, fs);
        }
        else if (type == "t")
        {
            //textures
            std::string slot = reader->getString();
            Texture* tex = new Texture(reader->getString().c_str());
            this->textures.insert(std::pair<std::string,Texture*>(slot, tex));
        }
    }
}

void Model::render(Matrix projectionMatrix, Matrix viewMatrix, Vector3 sunLight, Vector3 cameraPosition)
{
    shader->activate();
    shader->setMatrix("ProjectionMatrix", projectionMatrix);
    shader->setMatrix("ViewMatrix", viewMatrix);
    shader->setMatrix("WorldMatrix", matrixMultiply(matrixTranslate(position.x, position.y, position.z), matrixMultiply(matrixRotateX(rotation.x), matrixMultiply(matrixRotateY(rotation.y), matrixRotateZ(rotation.z)))));
    shader->setVector3("SunLight", sunLight);
    shader->setVector3("CameraPos", cameraPosition);
    for (std::map<std::string,Texture*>::iterator itr = textures.begin(), itr_end = textures.end(); itr != itr_end; ++itr)
    {
        shader->setTexture(itr->first.c_str(), itr->second);
    }
    mesh->draw();
}

void Model::setTransform(Vector3 position, Vector3 rotation)
{
    this->position = position;
    this->rotation = rotation;
}

Model::~Model()
{
    delete mesh;
    delete shader;
    for (std::map<std::string,Texture*>::iterator itr = textures.begin(), itr_end = textures.end(); itr != itr_end; ++itr)
    {
        delete itr->second;
    }
}
