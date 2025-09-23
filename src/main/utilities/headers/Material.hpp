#pragma once
#include <string>
#include <glm/glm.hpp>
using glm::vec3;
using std::string;

struct Material {
    float Ns;
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    vec3 Ke;
    float Ni;
    float d;
    string name;
    string map_Kd;

    // Used for texture loading - TODO check if we should do texture loading from ModelImport?
    string pathToTextureFile;
};
