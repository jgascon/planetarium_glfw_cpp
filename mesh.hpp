
#ifndef MESH
#define MESH

#include "linmath.h"

#include <stdlib.h>
#include <stddef.h>

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;


typedef struct Vertex {
    vec3 pos;
    vec2 uv;
    vec3 normal;
} Vertex;


typedef struct Mesh {
    vector<Vertex> vertices;
    vector<uint> faces;

    Mesh();
    ~Mesh();

    void clear();
} Mesh;


Mesh * load_obj(string filename);


#endif //MESH