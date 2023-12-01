#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath); //Ucitavanje teksture, izdvojeno u funkciju
// FUNKCIJE ZA KREIRANJE VAO I VBO ==================================================================================
void createPVAOandPVBO(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t size) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Pozicija
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Boja
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

    // Pozicija
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Kordinate teksture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
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

// POMOCNE FUNKCIJE ZA KREIRANJE KOORDINATA ===============================================================================
void getPyramidVertices(float x, float y, float size, const std::vector<float>& baseColor, std::vector<float>& vertices, float colorChangeProgress) {
    float offset = size * 0.1f;
    float peakX = x;
    float peakY = y + size;

    // Izracunavanje tamnije boje za desnu stranu piramide
    std::vector<float> darkerColor = { baseColor[0] * 0.8f, baseColor[1] * 0.8f, baseColor[2] * 0.8f };

    // Funkcija za interpolaciju menjanje boje postepeno u crvenu
    auto interpolateColor = [&](const std::vector<float>& color, float vertexX) -> std::vector<float> {
        float progressThreshold = (vertexX - (x - size / 2)) / size;
        if (colorChangeProgress >= progressThreshold) {
            float changeAmount = std::min(colorChangeProgress - progressThreshold, 1.0f);
            return { color[0] * (1 - changeAmount) + 1.0f * changeAmount, // R
                     color[1] * (1 - changeAmount),                       // G
                     color[2] * (1 - changeAmount) };                     // B
        }
        return color;
        };

    // Leva strana
    std::vector<float> leftColor = interpolateColor(baseColor, x - size / 2);
    vertices.insert(vertices.end(), {
        x - size / 2, y, leftColor[0], leftColor[1], leftColor[2],
        peakX, peakY, leftColor[0], leftColor[1], leftColor[2],
        x, y, leftColor[0], leftColor[1], leftColor[2]
        });

    // Desna strana
    std::vector<float> rightColor = interpolateColor(darkerColor, x);
    vertices.insert(vertices.end(), {
        x + size / 2 - offset, y + offset, rightColor[0], rightColor[1], rightColor[2],
        peakX, peakY, rightColor[0], rightColor[1], rightColor[2],
        x, y, rightColor[0], rightColor[1], rightColor[2]
        });
}
//Broj kordinata za krugove
const int numVertices = 40; 
void generateCircleVertices(float* vertices, float radius, float offsetX, float offsetY, float aspectRatio) {
    int vertexIndex = 0;
    int width, height;
 
    // Centar
    vertices[vertexIndex++] = offsetX; // X
    vertices[vertexIndex++] = offsetY; // Y

    for (int i = 0; i <= numVertices; ++i) {
        float angle = 2.0f * M_PI * i / numVertices;
        vertices[vertexIndex++] = radius * cos(angle) * aspectRatio + offsetX;  // X
        vertices[vertexIndex++] = radius * sin(angle) + offsetY; // Y
    }
}
void generateHalfCircleVertices(float* vertices, float radius, float offsetX, float offsetY) {
    int vertexIndex = 0;

    // Centar
    vertices[vertexIndex++] = offsetX; // X
    vertices[vertexIndex++] = offsetY; // Y

    // Koordinate polukruga
    for (int i = 0; i <= numVertices; ++i) {
        float angle = (3 * M_PI / 2) - (M_PI * i / numVertices);
        vertices[vertexIndex++] = radius * cos(angle) + offsetX; // X
        vertices[vertexIndex++] = radius * sin(angle) + offsetY; // Y
    }
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
    // Inicijalizacija GLFW-a
    if (!glfwInit())
        return -1;

    // Primarni monitor i video mode
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Kreiranje fullscreen prozora
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Desert Scene", monitor, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Kreiranje konteksta
    glfwMakeContextCurrent(window);

    // Inicijalizacija GLEW-a
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Setovanje viewport-a na fullscreen
    glViewport(0, 0, mode->width, mode->height);

    // KOORDINATE =================================================================================
    float aspectRatio = (float)mode->height / (float)mode->width;

    float circleVertices[(numVertices+2) * 2];
    generateCircleVertices(circleVertices, 0.2f, 0.0f, 0.0f, aspectRatio); // Za Sunce

    float halfCircleVertices[(numVertices + 2) * 2]; 
    generateHalfCircleVertices(halfCircleVertices, 0.1f, 0.0f, 0.0f); // Za polumesec

    float skyVertices[] = {
     -1.0f, 1.0f,  // Top-left
     -1.0f, -1.0f, // Bottom-left
      1.0f, 1.0f,  // Top-right
      1.0f, -1.0f  // Bottom-right
    };
  
    // Oaza (voda)
    const int numWaterVertices = 360; 
    float waterVertices[numWaterVertices * 2]; 
    generateEllipseVertices(waterVertices, -0.5f, -0.3f, 0.3f, 0.2f, numWaterVertices);

    // Pozicija ribice
    float fishPositionX = -0.5f;  // Inicijalna pozicija
    float fishWidth = 0.2f;       // Sirina ribice
    float fishHeight = 0.2f;      // Visina ribice
    float fishCenterY = -0.2f;    // Centar ribice (vertikalni)
    // Koordinate ribice
    float fishVertices[] = {
        fishPositionX - fishWidth, fishCenterY - fishHeight / 2,  // Levo teme
        fishPositionX, fishCenterY,                               // Gornje teme
        fishPositionX + fishWidth, fishCenterY - fishHeight / 2   // Desno teme
    };

    float grassVertices[] = {
    -0.8f, 0.06f, 0.0f, 1.0f, // Top-left (Leva gornja)
    -0.8f, -0.7f, 0.0f, 0.0f, // Bottom-left (Leva donja)
    -0.2f, 0.06f, 1.0f, 1.0f, // Top-right (Desna gornja)
    -0.2f, -0.7f, 1.0f, 0.0f  // Bottom-right (Desna donja)
    };

    float indexVertices[] = {
    0.4f, -0.8f, 0.0f, 1.0f, // Top-left (Leva gornja)
    0.4f, -1.0f, 0.0f, 0.0f, // Bottom-left (Leva donja)
    1.0f, -0.8f, 1.0f, 1.0f, // Top-right (Desna gornja)
    1.0f, -1.0f, 1.0f, 0.0f  // Bottom-right (Desna donja)
    };
    float desertVertices[] = {
    -1.0f,  0.0f,  // Top-left (Leva gornja)
    -1.0f, -1.0f,  // Bottom-left (Leva donja)
     1.0f,  0.0f,  // Top-right (Desna gornja)
     1.0f, -1.0f   // Bottom-right (Desna donja)
    };

    // Zvezdice
    const int numStars = 100;
    float starPositions[numStars * 2];
    srand(static_cast<unsigned int>(time(nullptr)));

    for (int i = 0; i < numStars * 2; i += 2) {
        starPositions[i] = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;   // X koordinata
        starPositions[i + 1] = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f; // Y koordinata
    }

    // Inicijalizovanje VAO i VBO =============================================================================

    // Sunce
    unsigned int circleVAO, circleVBO;
    createVAOandVBO(circleVAO, circleVBO, circleVertices, sizeof(circleVertices));

    // Polumesec
    unsigned int moonVAO, moonVBO;
    createVAOandVBO(moonVAO, moonVBO, halfCircleVertices, sizeof(halfCircleVertices));

    // Nebo
    unsigned int skyVAO, skyVBO;
    createVAOandVBO(skyVAO, skyVBO, skyVertices, sizeof(skyVertices));

    // Voda
    unsigned int waterVAO, waterVBO;
    createVAOandVBO(waterVAO, waterVBO, waterVertices, sizeof(waterVertices));

    // Ribica
    unsigned int fishVAO, fishVBO;
    createVAOandVBO(fishVAO, fishVBO, fishVertices, sizeof(fishVertices));

    // Pustinja
    unsigned int desertVAO, desertVBO;
    createVAOandVBO(desertVAO, desertVBO, desertVertices, sizeof(desertVertices));

    // Trava
    unsigned int grassVAO, grassVBO;
    createTVAOandTVBO(grassVAO, grassVBO, grassVertices, sizeof(grassVertices));

    // Index
    unsigned int indexVAO, indexVBO;
    createTVAOandTVBO(indexVAO, indexVBO, indexVertices, sizeof(indexVertices));

    // Piramide
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

    // Zvezdice
    unsigned int starVAO, starVBO;
    createVAOandVBO(starVAO, starVBO, starPositions, sizeof(starPositions));


    //Tekstura
    // 
    // Aktiviranje prve teksture ( trava)
    glActiveTexture(GL_TEXTURE0);
    unsigned grassTexture = loadImageToTexture("res/grass2.png");
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    // Setovanje parametara
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Aktiviranje druge teksture (index)
    glActiveTexture(GL_TEXTURE1);
    unsigned indexTexture = loadImageToTexture("res/index.jpg");
    glBindTexture(GL_TEXTURE_2D, indexTexture);

    // Setovanje parametara
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Unbindovanje tekstura
    glBindTexture(GL_TEXTURE_2D, 0);

    // POMOCNE PROMENLJIVE =====================================================================

    // Inicijalne boje najvece piramide
    float largestPyramidRed = 0.92f;
    float largestPyramidGreen = 0.80f;
    float largestPyramidBlue = 0.65f;

    // Kreiranje zajednickog shader-a
    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");

    bool showGrass = true;

    bool changingColor = false;
    float colorChangeProgress = 0.0f; // Od 0.0 (bez promene) do 1.0 (skroz crven)
    float colorChangeStep = 0.009f; // Vrednost promene ( sto manje = manja promena)

    float fishVelocityX = 0.001f;  // brzina ribice

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

    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Update brzine ribice
        fishPositionX += fishVelocityX;

        // Sirina prostora gde moze da se krece
        float leftBoundary = -0.8f + fishWidth;
        float rightBoundary = -0.2f - fishWidth;

        // Provera da li je u dozvoljenom prostoru
        if (fishPositionX > rightBoundary || fishPositionX < leftBoundary) {
            fishVelocityX = -fishVelocityX; // Promena smera
            fishPositionX += fishVelocityX;  // Korekcija pozicije
        }

        // Promena smera (izgled)
        bool facingRight = fishVelocityX > 0;

        // Update koordinata
        fishVertices[0] = facingRight ? fishPositionX - fishWidth : fishPositionX; // Levo teme X
        fishVertices[1] = fishCenterY - fishHeight / 2; // Levo teme Y

        fishVertices[2] = fishPositionX; // Top teme X
        fishVertices[3] = fishCenterY; // Top teme Y

        fishVertices[4] = facingRight ? fishPositionX : fishPositionX + fishWidth; // Desno teme X
        fishVertices[5] = fishCenterY - fishHeight / 2; // Desno teme Y

        // Update VBO ribice i koordinata
        glBindBuffer(GL_ARRAY_BUFFER, fishVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fishVertices), fishVertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Omogucavanje alpha blenda
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
      
        // Reagovanje na tastere =================================================================
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            showGrass = false;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            showGrass = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            changingColor = true;
            colorChangeProgress = std::max(0.0f, colorChangeProgress - colorChangeStep); 
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            changingColor = true;
            colorChangeProgress = std::min(1.0f, colorChangeProgress + colorChangeStep); 
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

        // Clear-uj ekran
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
   
        // Promenljive iz shader-a
        GLuint colorLoc = glGetUniformLocation(unifiedShader, "uColor");
        GLuint useVertexColorLoc = glGetUniformLocation(unifiedShader, "useVertexColor");
        GLuint useTextureLoc = glGetUniformLocation(unifiedShader, "useTexture");
        GLuint uSunPosLoc = glGetUniformLocation(unifiedShader, "uSunPos");
        GLuint uMoonPosLoc = glGetUniformLocation(unifiedShader, "uMoonPos");
        GLuint uIsSunLoc = glGetUniformLocation(unifiedShader, "uIsSun");
        GLuint uUseSunMoonPositioningLoc = glGetUniformLocation(unifiedShader, "uUseSunMoonPositioning");
        GLuint uRenderStarsLoc = glGetUniformLocation(unifiedShader, "uRenderStars");
        GLuint uStarSizeLoc = glGetUniformLocation(unifiedShader, "uStarSize");
        GLuint uStarIntensityLoc = glGetUniformLocation(unifiedShader, "uStarIntensity");

        glUseProgram(unifiedShader);
                
        glUniform1i(uUseSunMoonPositioningLoc, GL_FALSE);

        if (!isDayPaused) {
            currentTime += deltaTime; 
        }

        // Konverzija stepena u radijane
        float degreesToRadians = M_PI / 180.0f;
        float moonAdjustment = 45.0f * degreesToRadians; 


        float sunPosX = sunRadius * cos(currentTime * sunRotationSpeed);
        float sunPosY = sunRadius * sin(currentTime * sunRotationSpeed);

     
        float moonPosX = moonRadius * cos((currentTime + M_PI + moonAdjustment) * moonRotationSpeed);
        float moonPosY = moonRadius * sin((currentTime + M_PI + moonAdjustment) * moonRotationSpeed);
        
        // Crtanje neba
        GLfloat lightBlueColor[4] = { 0.53f, 0.81f, 0.98f, 1.0f };
        GLfloat darkBlueColor[4] = { 0.0f, 0.0f, 0.2f, 1.0f }; 

        float interpolationFactor = (sunPosY + 1.0f) / 2.0f; 

        GLfloat skyColor[4];
        for (int i = 0; i < 4; i++) {
            skyColor[i] = lightBlueColor[i] * interpolationFactor + darkBlueColor[i] * (1 - interpolationFactor);
        }

        glUniform4fv(colorLoc, 1, skyColor);
        glUseProgram(unifiedShader);
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_FALSE);
        glBindVertexArray(skyVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
       
        float starIntensity = 1.0f - interpolationFactor; // Maksimalan kada je interpolationFactor 0 

        glUniform1i(uRenderStarsLoc, GL_TRUE);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glUniform1f(uStarSizeLoc, 3.0f); 
        glUniform1f(uStarIntensityLoc, starIntensity);
        glBindVertexArray(starVAO);
        glDrawArrays(GL_POINTS, 0, numStars);

        glUniform1i(uRenderStarsLoc, GL_FALSE);

        // Crtanje sunca
        glUniform1i(uUseSunMoonPositioningLoc, GL_TRUE);
        glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
        glUniform1i(uIsSunLoc, GL_TRUE);
        glUniform2f(uSunPosLoc, sunPosX, sunPosY);
        glBindVertexArray(circleVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices + 2);


        // Crtanje polumeseca
        glUniform1i(uUseSunMoonPositioningLoc, GL_TRUE);
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glUniform1i(uIsSunLoc, GL_FALSE);
        glUniform2f(uMoonPosLoc, moonPosX, moonPosY);
        glBindVertexArray(moonVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices + 2);


        glUniform1i(uUseSunMoonPositioningLoc, GL_FALSE);

        // Crtanje pustinje
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_FALSE);
        glUniform4f(colorLoc, 0.76f, 0.70f, 0.50f, 1.0f);
        glBindVertexArray(desertVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Crtanje vode
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_FALSE);
        glUniform4f(colorLoc, 0.0f, 0.5f, 1.0f, 0.5f);
        glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), waterVertices, GL_STATIC_DRAW);
        glBindVertexArray(waterVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numWaterVertices);


        // Promena boje najvece piramide ukoliko su tasteri pritisnuti
        if (changingColor) {
            std::vector<float> newPyramidVertices1;
            getPyramidVertices(0.5f, -0.2f, 0.5f, baseColor, newPyramidVertices1, colorChangeProgress);
            glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO1);
            glBufferSubData(GL_ARRAY_BUFFER, 0, newPyramidVertices1.size() * sizeof(float), newPyramidVertices1.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            changingColor = false; 
        }

        // Crtanje piramide 1
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_TRUE);
        glBindVertexArray(pyramidVAO1);
        glDrawArrays(GL_TRIANGLES, 0, pyramidVertices1.size() / 5);

        // Crtanje piramide 2 
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_TRUE); // Use vertex color
        glBindVertexArray(pyramidVAO2);
        glDrawArrays(GL_TRIANGLES, 0, pyramidVertices2.size() / 5);

        // Crtanje piramide 3   
        glUniform1i(useTextureLoc, GL_FALSE);
        glUniform1i(useVertexColorLoc, GL_TRUE); 
        glBindVertexArray(pyramidVAO3);
        glDrawArrays(GL_TRIANGLES, 0, pyramidVertices3.size() / 5);

         // // Bind-ovanje teksture trave
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        // Crtanje trave 
        if (showGrass) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grassTexture);
            glUniform1i(useTextureLoc, GL_TRUE);
            glUniform1i(useVertexColorLoc, GL_TRUE); 
            glBindVertexArray(grassVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        }
        else {
            // Crtanje ribice
            glUniform1i(useTextureLoc, GL_FALSE);
            glUniform1i(useVertexColorLoc, GL_FALSE);
            glUniform4f(colorLoc, 1.0f, 0.5f, 0.0f, 1.0f);  
            glBindVertexArray(fishVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);  
        } 

        // Crtanje teksture indexa
        glBindTexture(GL_TEXTURE_2D, indexTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, indexTexture);
        glUniform1i(useTextureLoc, GL_TRUE);
        glUniform1i(useVertexColorLoc, GL_TRUE); 
        glBindVertexArray(indexVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


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

    glDeleteVertexArrays(1, &starVAO);
    glDeleteBuffers(1, &starVBO);

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