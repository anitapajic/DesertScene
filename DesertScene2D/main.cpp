
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//stb_image.h je header-only biblioteka za ucitavanje tekstura.
//Potrebno je definisati STB_IMAGE_IMPLEMENTATION prije njenog ukljucivanja
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath); //Ucitavanje teksture, izdvojeno u funkciju

void getPyramidVertices(float x, float y, float size, const std::vector<float>& baseColor, std::vector<float>& vertices, float colorChangeProgress) {
    float offset = size * 0.1f;
    float peakX = x;
    float peakY = y + size;

    // Calculate darker color
    std::vector<float> darkerColor = { baseColor[0] * 0.8f, baseColor[1] * 0.8f, baseColor[2] * 0.8f };

    // Function to interpolate between base color and red based on progress
    auto interpolateColor = [&](const std::vector<float>& color, float vertexX) -> std::vector<float> {
        float progressThreshold = (vertexX - (x - size / 2)) / size;
        if (colorChangeProgress >= progressThreshold) {
            float changeAmount = std::min(colorChangeProgress - progressThreshold, 1.0f);
            return { color[0] * (1 - changeAmount) + 1.0f * changeAmount, // Red component
                     color[1] * (1 - changeAmount),                       // Green component
                     color[2] * (1 - changeAmount) };                     // Blue component
        }
        return color;
        };

    // Left face
    std::vector<float> leftColor = interpolateColor(baseColor, x - size / 2);
    vertices.insert(vertices.end(), {
        x - size / 2, y, leftColor[0], leftColor[1], leftColor[2],
        peakX, peakY, leftColor[0], leftColor[1], leftColor[2],
        x, y, leftColor[0], leftColor[1], leftColor[2]
        });

    // Right face
    std::vector<float> rightColor = interpolateColor(darkerColor, x);
    vertices.insert(vertices.end(), {
        x + size / 2 - offset, y + offset, rightColor[0], rightColor[1], rightColor[2],
        peakX, peakY, rightColor[0], rightColor[1], rightColor[2],
        x, y, rightColor[0], rightColor[1], rightColor[2]
        });
}

void createPVAOandPVBO(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t size) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void createTVAOandTVBO(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t size) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    glBindVertexArray(0); // Unbind VAO
}
void createVAOandVBO(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t size) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

const int numVertices = 40; // Number of vertices for the circle

void generateCircleVertices(float* vertices, float radius, float offsetX, float offsetY) {
    int vertexIndex = 0;
    int width, height;
 
    // Center vertex
    vertices[vertexIndex++] = offsetX; // X
    vertices[vertexIndex++] = offsetY; // Y

    for (int i = 0; i <= numVertices; ++i) {
        float angle = 2.0f * M_PI * i / numVertices;
        vertices[vertexIndex++] = radius * cos(angle) + offsetX;  // X
        vertices[vertexIndex++] = radius * sin(angle) + offsetY; // Y
    }
}

void generateHalfCircleVertices(float* vertices, float radius, float offsetX, float offsetY) {
    int vertexIndex = 0;

    // Center vertex
    vertices[vertexIndex++] = offsetX; // X
    vertices[vertexIndex++] = offsetY; // Y

    // Half-circle vertices
    for (int i = 0; i <= numVertices; ++i) {
        float angle = (3 * M_PI / 2) - (M_PI * i / numVertices);
        vertices[vertexIndex++] = radius * cos(angle) + offsetX; // X, adjusted for aspect ratio
        vertices[vertexIndex++] = radius * sin(angle) + offsetY; // Y
    }
}

bool isNightTime(float sunPosY, float moonPosY) {
    if (sunPosY > 0) {
        return false; // Dan
    }
    else if (moonPosY > 0) {
        return true; // Noc
    }
    return false; // Podrazumevano je dan
}

void generateEllipseVertices(float* vertices, float centerX, float centerY, float radiusX, float radiusY, int numVertices) {
    int vertexIndex = 0;
    for (int i = 0; i < numVertices; ++i) {
        float angle = 2.0f * M_PI * i / numVertices;
        vertices[vertexIndex++] = centerX + radiusX * cos(angle); // X
        vertices[vertexIndex++] = centerY + radiusY * sin(angle); // Y
    }
}

int main(void)
{
    // Initialize GLFW
    if (!glfwInit())
        return -1;

    // Set window hints here (if necessary, for specific OpenGL version/context)

    // Get the primary monitor and video mode
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Create a fullscreen window
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Desert Scene", monitor, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Set viewport to the full window size
    glViewport(0, 0, mode->width, mode->height);


    float circleVertices[(numVertices+2) * 2];
    generateCircleVertices(circleVertices, 0.1f, 0.0f, 0.0f); // Za Sunce

    float halfCircleVertices[(numVertices + 2) * 2]; // Array for half-circle vertices
    generateHalfCircleVertices(halfCircleVertices, 0.1f, 0.0f, 0.0f); // Za polumesec


    float skyVertices[] = {
    -1.0f, 1.0f,  // Top Left
     1.0f, 1.0f,  // Top Right
     1.0f, 0.0f,  // Bottom Right
    -1.0f, 0.0f   // Bottom Left
    };
  
    const int numWaterVertices = 360; 
    float waterVertices[numWaterVertices * 2]; // Svaki vertex ima X i Y koordinatu

    // Poziv funkcije za generisanje elipse
    generateEllipseVertices(waterVertices, -0.5f, -0.3f, 0.3f, 0.2f, numWaterVertices);

    // Fish position and velocity
    float fishPositionX = -0.5f;  // More centered initial position
    float fishWidth = 0.2f;       // Width of the fish
    float fishHeight = 0.2f;      // Height of the fish
    float fishCenterY = -0.2f;    // Vertical center of the fish
    // Fish vertices
    float fishVertices[] = {
        fishPositionX - fishWidth, fishCenterY - fishHeight / 2,  // Left corner
        fishPositionX, fishCenterY,                               // Top center
        fishPositionX + fishWidth, fishCenterY - fishHeight / 2   // Right corner
    };

    float grassVertices[] = {
        // Positions    // Texture Coords (s, t)
        -0.8f, -0.7f, 0.0f, 0.0f, // Bottom Left
        -0.2f, -0.7f, 1.0f, 0.0f, // Bottom Right
        -0.2f, 0.06f,    1.0f, 1.0f, // Top Right
        -0.8f, 0.06f,    0.0f, 1.0f  // Top Left
    };

    float indexVertices[] = {
        // Positions    // Texture Coords (s, t)
        0.4f, -1.0f,   0.0f, 0.0f, // Bottom Left
        1.0f, -1.0f,    1.0f, 0.0f, // Bottom Right
        1.0f, -0.8f,    1.0f, 1.0f, // Top Right
        0.4f, -0.8f,   0.0f, 1.0f  // Top Left
    };
    float desertVertices[] = {
    -1.0f,  0.0f,  // Top Left
     1.0f,  0.0f,  // Top Right
     1.0f, -1.0f,  // Bottom Right
    -1.0f, -1.0f   // Bottom Left
    };

 
    float largestPyramidRed = 0.92f;
    float largestPyramidGreen = 0.80f;
    float largestPyramidBlue = 0.65f;

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");

    // Function to create VAO and VBO

    // Sun
    unsigned int circleVAO, circleVBO;
    createVAOandVBO(circleVAO, circleVBO, circleVertices, sizeof(circleVertices));

    // Half Moon
    unsigned int moonVAO, moonVBO;
    createVAOandVBO(moonVAO, moonVBO, halfCircleVertices, sizeof(halfCircleVertices));

    // Sky
    unsigned int skyVAO, skyVBO;
    createVAOandVBO(skyVAO, skyVBO, skyVertices, sizeof(skyVertices));

    // Water
    unsigned int waterVAO, waterVBO;
    createVAOandVBO(waterVAO, waterVBO, waterVertices, sizeof(waterVertices));

    // Fish
    unsigned int fishVAO, fishVBO;
    createVAOandVBO(fishVAO, fishVBO, fishVertices, sizeof(fishVertices));

    // Desert
    unsigned int desertVAO, desertVBO;
    createVAOandVBO(desertVAO, desertVBO, desertVertices, sizeof(desertVertices));

    // Grass
    unsigned int grassVAO, grassVBO;
    createTVAOandTVBO(grassVAO, grassVBO, grassVertices, sizeof(grassVertices));

    // Index
    unsigned int indexVAO, indexVBO;
    createTVAOandTVBO(indexVAO, indexVBO, indexVertices, sizeof(indexVertices));

    // Pyramids
    std::vector<float> pyramidVertices1, pyramidVertices2, pyramidVertices3;
    std::vector<float> baseColor = { 0.92f, 0.80f, 0.65f }; 

    getPyramidVertices(0.5f, -0.2f, 0.5f, baseColor, pyramidVertices1, 0.0f); 
    getPyramidVertices(0.2f, -0.3f, 0.3f, baseColor, pyramidVertices2, 0.0f);
    getPyramidVertices(0.8f, -0.3f, 0.4f, baseColor, pyramidVertices3, 0.0f);

    unsigned int pyramidVAO1, pyramidVBO1;
    createPVAOandPVBO(pyramidVAO1, pyramidVBO1, pyramidVertices1.data(), pyramidVertices1.size() * sizeof(float));

    unsigned int pyramidVAO2, pyramidVBO2;
    createPVAOandPVBO(pyramidVAO2, pyramidVBO2, pyramidVertices2.data(), pyramidVertices2.size() * sizeof(float));

    unsigned int pyramidVAO3, pyramidVBO3;
    createPVAOandPVBO(pyramidVAO3, pyramidVBO3, pyramidVertices3.data(), pyramidVertices3.size() * sizeof(float));


    //Tekstura
    // Activate the first texture unit and bind the grass texture
    glActiveTexture(GL_TEXTURE0);
    unsigned grassTexture = loadImageToTexture("res/grass2.png");
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    // Set texture parameters for the grass texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Activate the second texture unit and bind the index texture
    glActiveTexture(GL_TEXTURE1);
    unsigned indexTexture = loadImageToTexture("res/index.jpg");
    glBindTexture(GL_TEXTURE_2D, indexTexture);

    // Set texture parameters for the index texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);


    bool showGrass = true;

    bool changingColor = false;
    float colorChangeProgress = 0.0f; // Ranges from 0.0 (no change) to 1.0 (fully red)
    float colorChangeStep = 0.009f; // Smaller value for slower color change

    float fishVelocityX = 0.001f;

    const float sunRadius = 0.8f;
    const float moonRadius = 0.8f;
    const float sunRotationSpeed = 0.8f;
    const float moonRotationSpeed = 0.8f;

    bool isDayPaused = false;
    float currentTime = 0.0f; // Trenutno vreme za kontrolu doba dana
    bool lastPState = false;
    bool lastRState = false;
    float lastFrameTime = 0.0f;
    float deltaTime = 0.0f;



    //const int numStars = 100;

    //float starPositions[numStars * 2]; 

    //srand(static_cast<unsigned int>(time(nullptr)));

    //for (int i = 0; i < numStars * 2; i += 2) {
    //    starPositions[i] = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;   // X koordinata
    //    starPositions[i + 1] = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f; // Y koordinata
    //}

    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Update fish position
        fishPositionX += fishVelocityX;

        // Adjust for fish width
        float leftBoundary = -0.8f + fishWidth;
        float rightBoundary = -0.2f - fishWidth;

        // Check for boundary collision
        if (fishPositionX > rightBoundary || fishPositionX < leftBoundary) {
            fishVelocityX = -fishVelocityX; // Reverse direction
            fishPositionX += fishVelocityX;  // Immediate position correction
        }

        // Update whether the fish is facing right or left
        bool facingRight = fishVelocityX > 0;

        // Update vertices
        fishVertices[0] = facingRight ? fishPositionX - fishWidth : fishPositionX; // Left corner X
        fishVertices[1] = fishCenterY - fishHeight / 2; // Left corner Y

        fishVertices[2] = fishPositionX; // Top center X
        fishVertices[3] = fishCenterY; // Top center Y

        fishVertices[4] = facingRight ? fishPositionX : fishPositionX + fishWidth; // Right corner X
        fishVertices[5] = fishCenterY - fishHeight / 2; // Right corner Y

        // Update fish VBO with new vertices
        glBindBuffer(GL_ARRAY_BUFFER, fishVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fishVertices), fishVertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
      
        // Check if escape key was pressed
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            showGrass = false;
        }
        // Show grass on key press '2'
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            showGrass = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            changingColor = true;
            colorChangeProgress = std::max(0.0f, colorChangeProgress - colorChangeStep); // Decrease color change, clamp to 0.0
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            changingColor = true;
            colorChangeProgress = std::min(1.0f, colorChangeProgress + colorChangeStep); // Increase color change, clamp to 1.0
        }
        bool currentPState = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
        bool currentRState = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

        // Provera za P taster - samo reaguje na promenu stanja
        if (currentPState && !lastPState) {
            isDayPaused = !isDayPaused;
        }

        // Provera za R taster - samo reaguje na promenu stanja
        if (currentRState && !lastRState) {
            isDayPaused = false;
            currentTime = 0.0f; // Resetuje vreme na jutro
        }

      
        lastPState = currentPState;
        lastRState = currentRState;

        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
   
        GLuint colorLoc = glGetUniformLocation(unifiedShader, "uColor");
        GLuint useVertexColorLoc = glGetUniformLocation(unifiedShader, "useVertexColor");
        GLuint useTextureLoc = glGetUniformLocation(unifiedShader, "useTexture");
        GLuint uSunPosLoc = glGetUniformLocation(unifiedShader, "uSunPos");
        GLuint uMoonPosLoc = glGetUniformLocation(unifiedShader, "uMoonPos");
        GLuint uIsSunLoc = glGetUniformLocation(unifiedShader, "uIsSun");
        GLuint uUseSunMoonPositioningLoc = glGetUniformLocation(unifiedShader, "uUseSunMoonPositioning");
        GLuint isNightLoc = glGetUniformLocation(unifiedShader, "isNight");
        GLuint nightColorLoc = glGetUniformLocation(unifiedShader, "nightColor");


        glUseProgram(unifiedShader);
        
        glUniform1i(uUseSunMoonPositioningLoc, GL_FALSE);

        if (!isDayPaused) {
            currentTime += deltaTime; 
        }

        // Conversion from degrees to radians
        float degreesToRadians = M_PI / 180.0f;
        float moonAdjustment = 45.0f * degreesToRadians; // 20 degrees in radians


        float sunPosX = sunRadius * cos(currentTime * sunRotationSpeed);
        float sunPosY = sunRadius * sin(currentTime * sunRotationSpeed);

        // Adjusted moon position
        float moonPosX = moonRadius * cos((currentTime + M_PI + moonAdjustment) * moonRotationSpeed);
        float moonPosY = moonRadius * sin((currentTime + M_PI + moonAdjustment) * moonRotationSpeed);
     
        bool isNight = isNightTime(sunPosY, moonPosY);
        glUniform1i(isNightLoc, isNight);

        // Draw sky
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_FALSE);
        glUniform1i(isNightLoc, isNight);
        glUniform4f(isNight ? nightColorLoc : colorLoc, 0.53f, 0.81f, 0.98f, 1.0f);
        glBindVertexArray(skyVAO);
        glDrawArrays(GL_QUADS, 0, 4);

        /*if (isNight) {
            glPointSize(2.0f); 
            glColor3f(1.0f, 1.0f, 1.0f); 

            glBegin(GL_POINTS);
            for (int i = 0; i < numStars * 2; i += 2) {
                glVertex2f(starPositions[i], starPositions[i + 1]);
            }
            glEnd();
        }*/

        glUniform1i(isNightLoc, GL_FALSE);
    

        // Draw sun
        glUniform1i(uUseSunMoonPositioningLoc, GL_TRUE);
        glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
        glUniform1i(uIsSunLoc, GL_TRUE);
        glUniform2f(uSunPosLoc, sunPosX, sunPosY);
        glBindVertexArray(circleVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices + 2);


        // Draw Moon
        glUniform1i(uUseSunMoonPositioningLoc, GL_TRUE);
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glUniform1i(uIsSunLoc, GL_FALSE);
        glUniform2f(uMoonPosLoc, moonPosX, moonPosY);
        glBindVertexArray(moonVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices + 2);


        glUniform1i(uUseSunMoonPositioningLoc, GL_FALSE);

        // Draw Desert
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_FALSE);
        glUniform4f(colorLoc, 0.76f, 0.70f, 0.50f, 1.0f);
        glBindVertexArray(desertVAO);
        glDrawArrays(GL_QUADS, 0, 4);

        // Draw Water
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_FALSE);
        glUniform4f(colorLoc, 0.0f, 0.5f, 1.0f, 0.5f);
        glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), waterVertices, GL_STATIC_DRAW);
        glBindVertexArray(waterVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numWaterVertices);


        // Update the color of the largest pyramid if needed
        if (changingColor) {
            std::vector<float> newPyramidVertices1;
            getPyramidVertices(0.5f, -0.2f, 0.5f, baseColor, newPyramidVertices1, colorChangeProgress);
            glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO1);
            glBufferSubData(GL_ARRAY_BUFFER, 0, newPyramidVertices1.size() * sizeof(float), newPyramidVertices1.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            changingColor = false; // Reset the flag after updating the vertices
        }

        // Draw Pyramid 1
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_TRUE); // Use vertex color
        glBindVertexArray(pyramidVAO1);
        glDrawArrays(GL_TRIANGLES, 0, pyramidVertices1.size() / 5);

        // Draw Pyramid 2
      
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_TRUE); // Use vertex color
        glBindVertexArray(pyramidVAO2);
        glDrawArrays(GL_TRIANGLES, 0, pyramidVertices2.size() / 5);

        // Draw Pyramid 3
        
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_TRUE); // Use vertex color
        glBindVertexArray(pyramidVAO3);
        glDrawArrays(GL_TRIANGLES, 0, pyramidVertices3.size() / 5);

         // Draw Grass
         // // Bind grass texture

        glBindTexture(GL_TEXTURE_2D, grassTexture);

        // Draw grass strip
        
        if (showGrass) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grassTexture);
            glUniform1i(useTextureLoc, GL_TRUE);
            glUniform1i(useVertexColorLoc, GL_TRUE); // Use vertex color
            glBindVertexArray(grassVAO);
            glDrawArrays(GL_QUADS, 0, 4); 

        }
        else {
            // Draw fish
            glUniform1i(useTextureLoc, GL_FALSE);
            glUniform1i(useVertexColorLoc, GL_FALSE);
            glUniform4f(colorLoc, 1.0f, 0.5f, 0.0f, 1.0f);  // Fish color (example: orange)
            glBindVertexArray(fishVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);  // Assuming 3 vertices for the fish
        } 

        // Draw index
        glBindTexture(GL_TEXTURE_2D, indexTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, indexTexture);
        glUniform1i(useTextureLoc, GL_TRUE);
        glUniform1i(useVertexColorLoc, GL_TRUE); // Use vertex color
        glBindVertexArray(indexVAO);
        glDrawArrays(GL_QUADS, 0, 4);



        glfwSwapBuffers(window);
        glfwPollEvents();

    
    }

    glDeleteVertexArrays(1, &skyVAO);
    glDeleteBuffers(1, &skyVBO);

    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);

    glDeleteVertexArrays(1, &desertVAO);
    glDeleteBuffers(1, &desertVBO);

    glDeleteVertexArrays(1, &pyramidVAO2);
    glDeleteBuffers(1, &pyramidVBO2);

    glDeleteVertexArrays(1, &pyramidVAO2);
    glDeleteBuffers(1, &pyramidVBO2);

    glDeleteVertexArrays(1, &pyramidVAO3);
    glDeleteBuffers(1, &pyramidVBO3);

    glDeleteVertexArrays(1, &indexVAO);
    glDeleteBuffers(1, &indexVBO);

    glDeleteVertexArrays(1, &fishVAO);
    glDeleteBuffers(1, &fishVBO);

    glDeleteVertexArrays(1, &grassVAO);
    glDeleteBuffers(1, &grassVBO);

    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);

    glDeleteVertexArrays(1, &moonVAO);
    glDeleteBuffers(1, &moonVBO);



    glDeleteProgram(unifiedShader);

    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
     std::string temp = ss.str();
     const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);
    
    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}

static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}