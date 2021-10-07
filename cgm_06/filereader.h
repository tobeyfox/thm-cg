#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "cgmath.h"

struct FileReader
{
    FileReader(std::string filename);
    bool hasLine();
    std::string getString();
    Vector2 getVector2();
    Vector3 getVector3();

    std::ifstream fileStream;
    std::istringstream lineStream;
};
