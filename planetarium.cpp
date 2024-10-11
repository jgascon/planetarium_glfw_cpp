/*
    Simple demo of planet earth with obj loader.

    Jorge Gascon Perez
*/

#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <iostream>

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "mesh.hpp"

using namespace std;


GLenum error_gl = GL_NO_ERROR;
double global_last_x;
double global_last_y;
double global_rotation_x = 0.0f;
double global_rotation_y = 0.0f;

float global_light_pos[] = {-10.0, 0.0, -5.0};
//float global_light_pos[] = {0.0, 4.0, 0.0};
static float LIGHT_MOVE_OFFSET = 1.0f;

static GLuint global_gpu_program;

Mesh * global_mesh = NULL;



void init() {
    global_mesh = load_obj("../models/sphere.obj");
    //global_mesh = load_obj("../models/sphere.obj");
    //global_mesh = load_obj("../models/rectangle.obj");

    // printf("\nVertices [%d]\n", global_mesh->vertices.size());
    // for (uint i=0; i<global_mesh->vertices.size(); i++) {
    //     Vertex & v = global_mesh->vertices[i];
    //     printf("v %f %f %f\n", v.pos[0], v.pos[1], v.pos[2]);
    //     printf("n %f %f %f\n", v.normal[0], v.normal[1], v.normal[2]);
    //     printf("uv %f %f\n\n", v.uv[0], v.uv[1]);
    // }
    
    // printf("\nFaces [%d]\n", global_mesh->faces.size());
    // for (uint i=0; i<global_mesh->faces.size(); i+=3) {    
    //     printf("f %d %d %d\n", global_mesh->faces[i], global_mesh->faces[i+1], global_mesh->faces[i+2]);
    // }
};



static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"uniform vec3 vLightPos;\n"
"in vec3 vPos;\n"
"in vec2 vUV;\n"
"in vec3 vNormal;\n"
"out vec2 uv;\n"
"out vec3 normal;\n"
"out vec3 lightPos;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    uv = vUV;\n"
"    lightPos = normalize(vLightPos);\n"
"    mat3 normalMatrix = transpose(inverse(mat3(MVP)));\n"
"    normal = normalize(normalMatrix * vNormal);\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"in vec2 uv;\n"
"in vec3 normal;\n"
"in vec3 lightPos;\n"
"out vec4 fragment;\n"
"uniform sampler2D DiffuseMap;\n"
"void main()\n"
"{\n"
"    vec4 diffuse_color = texture(DiffuseMap, uv);\n"
"    float intensity = clamp(dot(normal, lightPos), 0.3, 1.0); \n"
"    fragment = diffuse_color * intensity; \n"
"}\n";



static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_W)
        global_light_pos[1] -= LIGHT_MOVE_OFFSET;
    if (key == GLFW_KEY_S)
        global_light_pos[1] += LIGHT_MOVE_OFFSET;
    if (key == GLFW_KEY_A)
        global_light_pos[0] -= LIGHT_MOVE_OFFSET;
    if (key == GLFW_KEY_D)
        global_light_pos[0] += LIGHT_MOVE_OFFSET;
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    const GLint light_pos_location = glGetUniformLocation(global_gpu_program, "vLightPos");
    if(light_pos_location < 0)
        cout << "ERROR: light_pos_location not found ..." << endl;
    glUniform3f(light_pos_location, global_light_pos[0], global_light_pos[1], global_light_pos[2]);

    printf("Light position (%f %f %f)\n",global_light_pos[0], global_light_pos[1], global_light_pos[2]);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        global_last_x = xpos;
        global_last_y = ypos;
        printf("First [%f, %f]\n", xpos, ypos);
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state != GLFW_PRESS) {
        return;
    }
    global_rotation_x += (ypos - global_last_y) * 0.005;
    global_rotation_y += (xpos - global_last_x) * 0.005;

    global_last_x = xpos;
    global_last_y = ypos;
}


GLuint loadTexture(string filename) {
    SDL_Surface * surface;
    GLuint texture_id;

    if ( (surface = IMG_Load(filename.c_str())) ) {
        glGenTextures( 1, &texture_id );

        error_gl = glGetError();
        if (error_gl != GL_NO_ERROR) {
            printf("ERROR: after glGenTextures %d\n", error_gl);
        }

        glBindTexture( GL_TEXTURE_2D, texture_id);

        error_gl = glGetError();
        if (error_gl != GL_NO_ERROR) {
            printf("ERROR: after glBindTexture %d\n", error_gl);
        }

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glTexImage2D(GL_TEXTURE_2D, 0, 4,
               surface->w, surface->h, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );

        error_gl = glGetError();
        if (error_gl != GL_NO_ERROR) {
            printf("ERROR: after glTexImage2D %d\n", error_gl);
        }
        SDL_FreeSurface( surface );
        printf("Texture loaded: size: %dx%d ID:%d\n",surface->w, surface->h, texture_id);
        return texture_id;
    } else {
        printf("SDL could not load image: %s  ERROR: %s\n", filename.c_str(), SDL_GetError());
        //SDL_Quit();
        return 0;
    }
}


int main(void) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 1);
    //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Planetarium", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    const GLsizei LogLength = 500;
    GLchar compilationLog[LogLength];
    GLsizei lengthObtained;

    glEnable(GL_CULL_FACE);

    init();

    GLuint vertices_buffer;
    glGenBuffers(1, &vertices_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_buffer);
    glBufferData(GL_ARRAY_BUFFER, 
                 sizeof(Vertex) * global_mesh->vertices.size(), 
                 global_mesh->vertices.data(), 
                 GL_STATIC_DRAW);

    GLuint triangles_buffer;
    glGenBuffers(1, &triangles_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangles_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 sizeof(uint) * global_mesh->faces.size(), 
                 global_mesh->faces.data(), 
                 GL_STATIC_DRAW);
    error_gl = glGetError();
    if (error_gl != GL_NO_ERROR)
        printf("ERROR creating triangles_buffer [%d]\n", error_gl);                 

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    glGetShaderInfoLog(vertex_shader, LogLength, &lengthObtained,  compilationLog);
    if (lengthObtained > 0) {
        printf("Log Vertex Shader \n %s\n", compilationLog);
    }

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    glGetShaderInfoLog(fragment_shader, LogLength, &lengthObtained,  compilationLog);
    if (lengthObtained>0) {
        printf("Log fragment_shader \n %s\n", compilationLog);
    }

    global_gpu_program = glCreateProgram();
    glAttachShader(global_gpu_program, vertex_shader);
    glAttachShader(global_gpu_program, fragment_shader);
    glLinkProgram(global_gpu_program);
    glGetProgramInfoLog(global_gpu_program, LogLength, &lengthObtained,  compilationLog);
    if (lengthObtained>0) {
        printf("Log program \n %s\n", compilationLog);
    }
    glUseProgram(global_gpu_program);

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    const GLint mvp_location = glGetUniformLocation(global_gpu_program, "MVP");
    if(mvp_location < 0)
        cout << "ERROR: mvp_location not found ..." << endl;

    const GLint light_pos_location = glGetUniformLocation(global_gpu_program, "vLightPos");
    if(light_pos_location < 0)
        cout << "ERROR: light_pos_location not found ..." << endl;
    glUniform3f(light_pos_location, global_light_pos[0], global_light_pos[1], global_light_pos[2]);

    const GLint vpos_location = glGetAttribLocation(global_gpu_program, "vPos");
    if(vpos_location < 0)
        cout << "ERROR: vpos_location not found ..." << endl;
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, pos));    

    const GLint vuv_location = glGetAttribLocation(global_gpu_program, "vUV");
    if(vuv_location < 0)
        cout << "ERROR: vuv_location not found ..." << endl;
    glEnableVertexAttribArray(vuv_location);
    glVertexAttribPointer(vuv_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, uv));

    const GLint vnormal_location = glGetAttribLocation(global_gpu_program, "vNormal");
    if(vnormal_location < 0)
        cout << "ERROR: vnormal_location not found ..." << endl;
    glEnableVertexAttribArray(vnormal_location);
    glVertexAttribPointer(vnormal_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, normal));


    glActiveTexture(GL_TEXTURE0);
    GLuint diffuseMap = loadTexture("../images/earth.png");
    error_gl = glGetError();
    if (error_gl != GL_NO_ERROR)
        printf("ERROR loading texture diffuseMap %d\n", error_gl);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    const GLint diffuse_map_location = glGetUniformLocation(global_gpu_program, "DiffuseMap");
    if(diffuse_map_location < 0)
        cout << "ERROR: diffuse_map_location not found ..." << endl;
    glUniform1i(diffuse_map_location, 0);


    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_translate(m, 0.f, 0.f, -4.f);

        mat4x4_rotate_X(m, m, global_rotation_x);
        mat4x4_rotate_Y(m, m, global_rotation_y);

        mat4x4_identity(p);
        mat4x4_perspective(p, 45.f, ratio, 0.0f, 10.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(global_gpu_program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);

        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangles_buffer);
        glDrawElements(GL_TRIANGLES, global_mesh->faces.size(), GL_UNSIGNED_INT, NULL);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    SDL_Quit();
    exit(EXIT_SUCCESS);
}
