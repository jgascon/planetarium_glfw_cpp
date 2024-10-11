
#include <stdlib.h>
#include <stddef.h>

#include "linmath.h"

#include <string>
#include <array>
#include <vector>
#include <map>
#include <iostream>

#include "mesh.hpp"

using namespace std;


typedef array<float,2> float2;

typedef array<float,3> float3;

typedef array<uint,2> uint2;

typedef array<uint,3> uint3;


Mesh::Mesh() {
    this->clear();
}

Mesh::~Mesh() {
    this->clear();
}

void Mesh::clear() {
    this->vertices.clear();
    this->faces.clear();
}


string replace (string entrada, string busca, string cambia) {
    if (entrada != "" && busca != cambia) {
        string::size_type pos=0;
        pos = entrada.find(busca,pos);
        while (pos != string::npos) {
            entrada.replace(pos,busca.length(),cambia);
            pos = entrada.find(busca,pos+cambia.length());
        }
    }
    return entrada;
}


string int_to_str (int v) {
    static char s_v[100];
    sprintf( s_v, "%d", v);
    return replace (s_v, " ", "");
}


/**
   Generates a mesh from a Wavefront .obj file.
   @pre The file has to be a valid wavefront .obj file.
   @param filename [IN]: Filename of the .obj file to load.
   @return A reference of a mesh or NULL if the load failed.
*/
Mesh * load_obj(string filename) {

    FILE * file = fopen(filename.c_str(), "r");
    vector<float3> compact_vertices;
    vector<float3> compact_v_normals;
    vector<float2> compact_v_uvs;

    char line[1000];
    int bytes_read;
    char * text_read;
    float3 p;
    float2 vt;
    uint3 f_i;
    uint3 uv_i;
    uint3 n_i;

    if (!file) {
        cout << "ERROR: load_obj: File [" << filename << "] not found.\n";
        return NULL;
    }

    // cout << "load_obj [" << filename << "]\n";
    
    Mesh * m = new Mesh();
    string key;
    uint key_val;
    map<string, unsigned> coincidences;

    while (!feof(file)) {
        bytes_read = fscanf(file, "%s", line);

        if (strcmp(line, "vn") == 0) {
            bytes_read = fscanf(file, "%f %f %f\n", &p[0], &p[1], &p[2]);
            compact_v_normals.push_back(p);

        } else if (strcmp(line, "vt") == 0) {
            bytes_read = fscanf(file, "%f %f\n", &vt[0], &vt[1]);
            vt[1] = 1.0f - vt[1];
            compact_v_uvs.push_back(vt);

        } else if (strcmp(line, "v") == 0) {
            bytes_read = fscanf(file, "%f %f %f\n", &p[0], &p[1], &p[2]);
            compact_vertices.push_back(p);

        } else if (strcmp(line, "f") == 0) {
            if (compact_v_uvs.size() == 0) {
                //f 7407//3970 7408//3979 7404//3978
                bytes_read = fscanf(file, "%d//%d %d//%d %d//%d\n",
                                    &f_i[0], &n_i[0], &f_i[1], &n_i[1], &f_i[2], &n_i[2]);

                for (uint j=0; j<3; j++) {
                    f_i[j]--;
                    n_i[j]--;
                    key = int_to_str(f_i[j]) + "_" + int_to_str(n_i[j]);
                    // cout << "Key: " << key << endl;
                    //Searching the key of this vertex
                    if (coincidences.find(key) == coincidences.end()) {
                        //Key not found, creating new vertex and key.
                        Vertex a_vertex;
                        float3 & vert = compact_vertices[f_i[j]];
                        a_vertex.pos[0] = vert[0];
                        a_vertex.pos[1] = vert[1];
                        a_vertex.pos[2] = vert[2];

                        float3 & norm = compact_v_normals[n_i[j]];
                        a_vertex.normal[0] = norm[0];
                        a_vertex.normal[1] = norm[1];
                        a_vertex.normal[2] = norm[2];

                        a_vertex.uv[0] = 0.0;
                        a_vertex.uv[1] = 0.0;
                        key_val = m->vertices.size();
                        m->vertices.push_back(a_vertex);
                    } else {
                        //Key found, using found vertex.
                        key_val = coincidences[key];
                    }
                    coincidences[key] = key_val;            
                    m->faces.push_back(key_val);
                } //for (uint j=0; j<3; j++) 

            } else { //if (compact_v_uvs.size() != 0)

                //f 13/14/1 14/12/1 16/2/1
                //Reading a mesh with texture coordinates.
                bytes_read = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                                    &f_i[0], &uv_i[0], &n_i[0], &f_i[1], &uv_i[1], &n_i[1], &f_i[2], &uv_i[2], &n_i[2]);

                for (uint j=0; j<3; j++) {
                    f_i[j]--;
                    uv_i[j]--;
                    n_i[j]--;
                    key = int_to_str(f_i[j]) + "_" + int_to_str(uv_i[j])  + "_" + int_to_str(n_i[j]);
                    // cout << "Key: " << key << endl;
                    //Searching the key of this vertex
                    if (coincidences.find(key) == coincidences.end()) {
                        //Key not found, creating new vertex and key.
                        Vertex a_vertex;

                        float3 & vert = compact_vertices[f_i[j]];
                        a_vertex.pos[0] = vert[0];
                        a_vertex.pos[1] = vert[1];
                        a_vertex.pos[2] = vert[2];

                        float2 & vuv = compact_v_uvs[uv_i[j]];
                        a_vertex.uv[0] = vuv[0];
                        a_vertex.uv[1] = vuv[1];

                        float3 & norm = compact_v_normals[n_i[j]];
                        a_vertex.normal[0] = norm[0];
                        a_vertex.normal[1] = norm[1];
                        a_vertex.normal[2] = norm[2];

                        key_val = m->vertices.size();
                        m->vertices.push_back(a_vertex);
                    } else {
                        //Key found, using found vertex.
                        key_val = coincidences[key];
                    }
                    coincidences[key] = key_val;            
                    m->faces.push_back(key_val);
                } //for (uint j=0; j<3; j++) 

            } //if (compact_v_uvs.size() != 0)

        } else {
            //Ignore this line (It might be a comment)
            text_read = fgets(line, 499, file);
        }
    } //while (!feof(file)) 
    fclose(file);
    return m;
} //Mesh * load_obj(string filename) 
