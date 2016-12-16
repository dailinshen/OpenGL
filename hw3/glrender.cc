
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"
#include "parse_obj.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

typedef vec4  point4;
typedef vec4  color4;

// Initial up direction
// Fixed center
point4 center = {0.0, 0.0, 0.0, 1.0};
vec4 upY = {0.0, 1.0, 0.0, 0.0};

float theta = 0.0;  // rotation around Y axis
float phi = 90.0; // rotation around X axis
float r = 5.0; // distance from center

float global_ratio;

GLuint InitShader( const char* vertexShaderFile,
                  const char* fragmentShaderFile );

bool vshader_turnoff = true;

int NumVertices = 0;

vec4  *vertices = nullptr;
vec4  *norms = nullptr;

point4 eye;
vec4 up;
mat4x4 view_transform;

// viewer, light & material definitions, for lighting calculations:

// light & material definitions, again for lighting calculations:
point4 light_position = {100.0, 100.0, 100.0, 1.0};
color4 light_ambient = {0.2, 0.2, 0.2, 1.0};
color4 light_diffuse = {1.0, 1.0, 1.0, 1.0};
color4 light_specular = {1.0, 1.0, 1.0, 1.0};

// material definitions:
color4 material_ambient = {1.0, 0.0, 1.0, 1.0};
color4 material_diffuse = {1.0, 0.8, 0.0, 1.0};
color4 material_specular = {1.0, 0.8, 0.0, 1.0};
float material_shininess = 100.0;

// "names" for the various buffers, shaders, programs etc:
GLuint vertex_buffer, program;
GLuint eye_pos, view, projection;
GLuint light_p, light_a, light_d, light_s;
GLuint material_a, material_d, material_s,material_sh;
GLuint turnoff;
// for keeping track of the rotation or translation:

// for converting:
const float deg_to_rad = (3.1415926 / 180.0);

// three helper functions for the vec4 class:
void vecproduct(vec4 &res, const vec4 &v1, const vec4 &v2) {
    for (int i = 0; i < 4; ++i) res[i] = v1[i] * v2[i];
}

void vecset(vec4 &res, const vec4 &v1) {
    for (int i = 0; i < 4; ++i) res[i] = v1[i];
}

void vecclear(vec4 &res) {
    for (int i = 0; i < 4; ++i) res[i] = 0.0;
}


static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
    // exit if the ESC key is pressed:
    if (key == GLFW_KEY_ESCAPE and action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    if (key == GLFW_KEY_Z && r > 3.0) {  //zoom in
        r *= 0.9;
    }
    else if (key == GLFW_KEY_X && r < 50.0){  // zoom out
        r *= 1.1;
    }
    
    if (key == GLFW_KEY_S && action == GLFW_PRESS && vshader_turnoff == false) {
        vshader_turnoff = true;
    }
    else if (key == GLFW_KEY_S && action == GLFW_PRESS && vshader_turnoff == true){
        vshader_turnoff = false;
    }
}


// set up the vertex data buffers and the program (shaders):
void init()
{
    
    // set up vertex buffer object - this will be memory on the GPU where
    // we are going to store our vertex data (that is currently in the "points"
    // array)
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    
    
    // specify that its used for vertex data, what its size is, and where the
    // data is located, and finally a "hint" about how we are going to use
    // the data (the driver will put it in a good memory location, hopefully)
    
    glBufferData(GL_ARRAY_BUFFER, NumVertices* sizeof(point4) + NumVertices * sizeof(vec4), NULL, GL_STATIC_DRAW);
    
    // tell the VBO to get the data from the vertices array and the colors array:
    glBufferSubData(GL_ARRAY_BUFFER, 0, NumVertices * sizeof(point4), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, NumVertices * sizeof(point4),NumVertices * sizeof(vec4), norms);
    
    // load in these two shaders...  (note: InitShader is defined in the
    // accompanying initshader.c code).
    // the shaders themselves must be text glsl files in the same directory
    // as we are running this program:
    program = InitShader("vshader_passthrough_lit.glsl", "fshader_passthrough_lit.glsl");
    
    // ...and set them to be active
    glUseProgram(program);
    
    GLuint loc_1 = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(loc_1);
    
    // the vPosition attribute is a series of 4-vecs of floats, starting at the
    // beginning of the buffer
    glVertexAttribPointer(loc_1, 4, GL_FLOAT, GL_FALSE, 0, (void*)(0));
    
    GLuint loc_2 = glGetAttribLocation(program, "vNorm");
    glEnableVertexAttribArray(loc_2);
    
    // the vColor attribute is a series of 4-vecs of floats, starting just after
    // the points in the buffer
    glVertexAttribPointer(loc_2, 4, GL_FLOAT, GL_FALSE, 0,
                          (void*)(sizeof(point4) * NumVertices));
    
    eye_pos = glGetUniformLocation(program, "eye");
    view = glGetUniformLocation(program, "view");
    projection = glGetUniformLocation(program, "projection");
    
    light_p = glGetUniformLocation(program, "light_position");
    light_a = glGetUniformLocation(program, "light_ambient");
    light_d = glGetUniformLocation(program, "light_diffuse");
    light_s = glGetUniformLocation(program, "light_specular");
    
    material_a = glGetUniformLocation(program, "material_ambient");
    material_d = glGetUniformLocation(program, "material_diffuse");
    material_s = glGetUniformLocation(program, "material_specular");
    material_sh = glGetUniformLocation(program, "material_shininess");
    
    turnoff = glGetUniformLocation(program, "vshader_turnoff");
    
    glUniform1f(turnoff, vshader_turnoff);
    
    
    glUniform4fv(light_p, 1, light_position);
    glUniform4fv(light_a, 1, light_ambient);
    glUniform4fv(light_d, 1, light_diffuse);
    glUniform4fv(light_s, 1, light_specular);
    glUniform4fv(material_a, 1, material_ambient);
    glUniform4fv(material_d, 1, material_diffuse);
    glUniform4fv(material_s, 1, material_specular);
    glUniform1f(material_sh, material_shininess);
}

// use this motionfunc to demonstrate rotation - it adjusts "theta" based
// on how the mouse has moved.
static void mouse_move_rotate (GLFWwindow* window, double x, double y)
{
    static float lastx = 0;// keep track of where the mouse was last:
    static float lasty = 0;// keep track of where the mouse was last:
    
    float amntX = x - lastx;
    
    if (amntX != 0.)
    theta +=  amntX;
    
    if (theta > 360.0) theta -= 360.0;
    if (theta < 0.0 )  theta += 360.0;
    
    lastx = x;
    
    float amntY = y - lasty;
    
    if (amntY != 0.)
    phi +=  amntY;
    
    if (phi > 175.0) phi = 175;
    if (phi < 5.0 )  phi = 5.0;
    
    lasty = y;
}


void load_obj_file (char *filename)
{
    
    std::vector<int> tri_ids;
    std::vector<float> tri_verts;
    
    read_wavefront_file(filename, tri_ids, tri_verts);
    
    NumVertices = (int)tri_ids.size();
    
    vertices = new point4[NumVertices];
//    points   = new point4[NumVertices];

    norms    = new vec4[NumVertices];
    vec4 *vertex_noraml = new vec4[tri_verts.size() / 3];
    
    // tri_ids is a list of the vertex indices for each triangle, so the first
    // triangle uses up the first 3 indices, etc.
    for (int k = 0; k < tri_ids.size() / 3; ++k) {
        
        vertices[3*k][0] = tri_verts[3*tri_ids[3*k]];
        vertices[3*k][1] = tri_verts[3*tri_ids[3*k]+1];
        vertices[3*k][2] = tri_verts[3*tri_ids[3*k]+2];
        vertices[3*k][3] = 1.;
        
        vertices[3*k+1][0] = tri_verts[3*tri_ids[3*k+1]];
        vertices[3*k+1][1] = tri_verts[3*tri_ids[3*k+1]+1];
        vertices[3*k+1][2] = tri_verts[3*tri_ids[3*k+1]+2];
        vertices[3*k+1][3] = 1.;
        
        vertices[3*k+2][0] = tri_verts[3*tri_ids[3*k+2]];
        vertices[3*k+2][1] = tri_verts[3*tri_ids[3*k+2]+1];
        vertices[3*k+2][2] = tri_verts[3*tri_ids[3*k+2]+2];
        vertices[3*k+2][3] = 1.;
        
    }
    
    for (int i = 0; i < tri_ids.size() / 3; ++i) {
        
        // compute the triangle's normal:
        vec4 e1, e2, n;
        vec4_sub(e1, vertices[3*i+1], vertices[3*i+0]);
        vec4_sub(e2, vertices[3*i+2], vertices[3*i+0]);
        vec4_mul_cross(n, e1, e2);
        n[3] = 0.f; // cross product in 4d sets this value to 1, which we do not want...
        vec4_norm(n, n);
        
        vec4_add(vertex_noraml[tri_ids[3 * i]], vertex_noraml[tri_ids[3 * i]], n);
        vec4_add(vertex_noraml[tri_ids[3 * i + 1]], vertex_noraml[tri_ids[3 * i + 1]], n);
        vec4_add(vertex_noraml[tri_ids[3 * i + 2]], vertex_noraml[tri_ids[3 * i + 2]], n);
    }
    for (int i = 0; i < tri_verts.size() / 3; ++i) {
        vec4_norm(vertex_noraml[i], vertex_noraml[i]);
    }
    for (int i = 0; i < NumVertices / 3; i++) {
        vecset(norms[3 * i], vertex_noraml[tri_ids[3 * i]]);
        vecset(norms[3 * i + 1], vertex_noraml[tri_ids[3 * i + 1]]);
        vecset(norms[3 * i + 2], vertex_noraml[tri_ids[3 * i + 2]]);
    }
    
}
void update_projection() {
    // clear the window (with white) and clear the z-buffer (which isn't used
    // for this example).
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // based on where the mouse has moved to, compute new eye position
    float theta_ = deg_to_rad * theta;
    float phi_ = deg_to_rad * phi;
    
    vec3 eye;
    eye[0] = r * sin(theta_) * sin(phi_);
    eye[1] = r * cos(phi_);
    eye[2] = r * sin(phi_) *cos(theta_);
    // specify the value of uniform variables
    
    point4 origin_eye = {0.0, 0.0, 3.0, 1.0};
    
    glUniform4fv(eye_pos, 1, origin_eye);
    
    // If transpose is GL_TRUE, each matrix is assumed to be supplied in row major order.
    vec3 center_ = {0.0, 0.0, 0.0};
    vec3 up_Y = {0, 1.0, 0.0};
    
    mat4x4_look_at(view_transform, eye, center_, up_Y);
    glUniformMatrix4fv(view, 1, GL_FALSE, (const GLfloat*) view_transform);
    
    mat4x4 projection_transform;
    mat4x4_perspective(projection_transform, 0.5, global_ratio, .1, 100);
    glUniformMatrix4fv(projection, 1, GL_FALSE, (const GLfloat*) projection_transform);
    
    
    glUniform1f(turnoff, vshader_turnoff);
    
    // draw the VAO:
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    
}

int main(int argc, char** argv)
{
    
    if (argc != 2) {
        std::cout << "usage: glrender objfile" << std::endl;
        exit (0);
    }
    
    // if there are errors, call this routine:
    glfwSetErrorCallback(error_callback);
    
    // start up GLFW:
    if (!glfwInit())
    exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    
    // for more modern version of OpenGL:
    //  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    
    GLFWwindow* window;
    window = glfwCreateWindow(640, 480, "hello triangle transform!", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwSetKeyCallback(window, key_callback);
    
    // call only once: demo for rotation:
    glfwSetCursorPosCallback(window, mouse_move_rotate);
    
    
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);
    
    // set the background to white:
    glClearColor(1.0, 1.0, 1.0, 1.0);
    // enable the z-buffer for depth tests:
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // clear everything:
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // read the obj file and put it into the points array:
    load_obj_file (argv[1]);
    
    init();
    
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        
        // in case the  window viewport size changed, we will need to adjust the
        // projection:
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        global_ratio = ratio;
        glViewport(0, 0, width, height);
        
        // clear the window (with white) and clear the z-buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // make up a transform that rotates around screen "Z" with time:
        update_projection();
        
        // tri() will multiply the points by ctm, and figure out the lighting
        
        // tell the VBO to re-get the data from the points and colors arrays:
        glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(point4)*NumVertices, vertices );
        glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4) *NumVertices, sizeof(vec4)*NumVertices, norms);
        
        // orthographically project to screen:
        
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
        
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }
    
    // free up GLFW objects:
    glfwDestroyWindow(window);
    
    // free up GL objects:
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteProgram(program);
    
    // free up memory:
    delete [] vertices;
    delete [] norms;
    
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
