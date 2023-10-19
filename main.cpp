/* - In geometry, a cyclogon is the curve traced by a vertex of a regular polygon that rolls
without slipping along a straight line. In the limit, as the number of sides increases to infinity,
the cyclogon becomes a cycloid. */

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// GLM header file
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;


// shaders header file
#include <common/shader.hpp>

// Vertex array object (VAO)
GLuint VertexArrayID;

// Vertex buffer object (VBO)
GLuint vertexbuffer;

// color buffer object (CBO)
GLuint colorbuffer;

// GLSL program from the shaders
GLuint programID;


#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 900

#define pi 3.14159265359
#define A 0.707106781187
#define a 3.92699081699
#define d 3.93
#define b 4
#define R 1

float functions_fg(float x, int i) {
    float top_f = R * cos(pi / b);
    float down_f = cos(x - a - ((2 * pi * floor(((b * (x - a)) /(2 * pi)) - b * floor((1 / (2 * pi)) * (x - a))) + pi) / b));
    float first_f = top_f / down_f;
    if (i == 0) {
        float f = first_f * cos(x);
        return f;
    }
    else {
        float g = first_f * sin(x) + 0.7071067811865;
        return g;
    }
}

float function_b(float x) {
    return (- ((2 * pi) / b)) * floor((x * b )/ (2 * pi));
}

float function_c(float x) {
    return (2 * A * floor((x * b) / (2 * pi)));
}

float function_d(float x) {
    return (x - ((2 * pi) / b) * floor((x * b) / (2 * pi)));
}


//--------------------------------------------------------------------------------
float cx(float x) {
    return (((functions_fg(d + function_b(x), 0) - A) * cos(function_d(x)) + (functions_fg(d + function_b(x),1) * sin(function_d(x)))) + A + function_c(x));
}

float cy(float x) {
    return ((functions_fg(d + function_b(x), 1) * cos(function_d(x))) - (functions_fg(d + function_b(x), 0) - A) * sin(function_d(x)));
}

//--------------------------------------------------------------------------------
std::vector<GLfloat> *sincpoints(GLfloat xmin, GLfloat xmax, GLfloat step){
    
    float length = xmax - xmin;
    int N = (int) length / step;    // number of points on the curve

    std::vector<GLfloat> *array = new std::vector<GLfloat>(N*3);
    float x = xmin;

    for( int i = 0 ; i < N; i++)
    {
        (*array)[i * 3] = cx(x)*3;                   // x value
        (*array)[(i * 3) + 1] = cy(x)*0.2;         // y value
        (*array)[(i * 3)+2] = 0;                 // z value
        x += step;
    }
    
//    for (int i = 0; i < N; i++)
//        std::cout<<"point: "<<(*array)[i * 3]<<" "<<(*array)[(i * 3)+1]<<" "<<(*array)[(i * 3)+2]<<std::endl;
// 
    return array;
}

//--------------------------------------------------------------------------------
int transferDataToGPUMemory(void)
{
    GLfloat xmin = -20.0f;          // xmin of the domain
    GLfloat xmax = 20.0f;           // xmin of the domain
    float step = 0.1f;
    int n;    // number of points on the curve

    // VAO
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders
    (
        "C:/OpenGL/sinc/SimpleVertexShader.vertexshader",
        "C:/OpenGL/sinc/SimpleFragmentShader.fragmentshader" );
    

    std::vector<GLfloat> *vertex_data = sincpoints(xmin, xmax, step);
    n = (int) vertex_data->size() / 3; // number of points on the curve

    std::cout<< "n = "<<n<<std::endl;
    
//    for (int i = 0; i < n; i++)
//        std::cout<<"point oe: "<<(*vertex_data)[i * 3]<<" "<<(*vertex_data)[(i * 3)+1]<<" "<<(*vertex_data)[(i * 3)+2]<<std::endl;

    std::vector<GLfloat> *color_data = new std::vector<GLfloat>(3*n);
    for (int i = 0; i < n; i++)
    {
        (*color_data)[i * 3] = 1.0f;
        (*color_data)[(i * 3) + 1] = 0.0f;
        (*color_data)[(i * 3) + 2] = 0.0f;
    }

    std::cout<< "n bytes = "<<sizeof(GLfloat)*n*3<<std::endl;

    // Move vertex data to video memory; specifically to VBO called vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, (int)vertex_data->size() * sizeof(GLfloat), vertex_data->data(), GL_STATIC_DRAW);
    
    // Move color data to video memory; specifically to CBO called colorbuffer
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, (int)color_data->size() * sizeof(GLfloat), color_data->data(), GL_STATIC_DRAW);
    
    std::cout<< "nv = "<<(int)vertex_data->size()<<std::endl;
    std::cout<< "nc = "<<(int)color_data->size()<<std::endl;

    return n;
}


//--------------------------------------------------------------------------------
void cleanupDataFromGPU()
{
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);
}


//--------------------------------------------------------------------------------
void draw (int n)
{
    //std::cout<< "n = "<<n<<std::endl;

    // Clear the screen
    glClear( GL_COLOR_BUFFER_BIT );
    
    // Use our shader
    glUseProgram(programID);
    
    // create domain in R^2
    glm::mat4 mvp = glm::ortho(-20.0f, 20.0f, -1.0f, 1.5f);
    // retrieve the matrix uniform locations
    unsigned int matrix = glGetUniformLocation(programID, "mvp");
    glUniformMatrix4fv(matrix, 1, GL_FALSE, &mvp[0][0]);


    
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
                          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(
                          1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                          3,                                // size
                          GL_FLOAT,                         // type
                          GL_FALSE,                         // normalized?
                          0,                                // stride
                          (void*)0                          // array buffer offset
                          );
    
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    //glPointSize(10);
    // Draw the triangle !
    //glEnable(GL_LINE_WIDTH);
    //glLineWidth(10.0);
    glDrawArrays(GL_LINE_STRIP, 0, n); // 3 indices starting at 0 -> 1 triangle
    //glDrawArrays(GL_POINTS, 0, 80); // 3 indices starting at 0 -> 1 triangle
    //glDisable(GL_LINE_WIDTH);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
//--------------------------------------------------------------------------------


int main(void)
{
    GLFWwindow* window;
    
    // Initialize the library
    if (!glfwInit())
    {
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    
    
    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "sinc(x)", NULL, NULL);
    
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    // Make the window's context current
    glfwMakeContextCurrent(window);
    
    glewExperimental = true;
    // Needed for core profile
    if (glewInit() != GLEW_OK){
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    
   
    // transfer my data (vertices, colors, and shaders) to GPU side
    int n = transferDataToGPUMemory();
    std::cout<< "n = "<<n<<std::endl;

    
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
 
        draw(n);
        
        // Swap front and back buffers
        glfwSwapBuffers(window);
        
        // Poll for and process events
        glfwPollEvents();
    }
    
    // Cleanup VAO, VBOs, and shaders from GPU
    cleanupDataFromGPU();
    
    glfwTerminate();
    
    return 0;
}

