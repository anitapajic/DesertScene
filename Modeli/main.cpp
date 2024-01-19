//Opis: Primjer ucitavanja modela upotrebom ASSIMP biblioteke
//Preuzeto sa learnOpenGL

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "model.hpp"
#include <vector>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define _CRT_SECURE_NO_WARNINGS
#define CRES 30 

#pragma region Directional light properties
glm::vec3 lerpedDirLightColor;
glm::vec3 softYellow = glm::vec3(1.0, 1.0, 0.6);
glm::vec3 softGrey = glm::vec3(0.7529, 0.7529, 0.7529);
glm::vec3 lerpedBackroundColor;
glm::vec3 lightBlue = glm::vec3(0.7, 0.7, 1.0);
glm::vec3 darkBlue = glm::vec3(0.1176, 0.1569, 0.2745);
glm::vec3 dirLightIntensityStart = glm::vec3(1.0);
glm::vec3 dirLightIntensityEnd = glm::vec3(0.5);
glm::vec3 lerpedDirLightIntensity;
#pragma endregion

#pragma region Point light properties
glm::vec3 lightPos = glm::vec3(5.51705, 3.20288, 1.28056);
glm::vec3 lightPos2 = glm::vec3(-2.1852, 3.827, -1.34884);
glm::vec3 lightPos3 = glm::vec3(1.98089f, 3.56757f, -1.32073f);
glm::vec3 softGreen = glm::vec3(0.0, 1.0, 0.0);
glm::vec3 lightColor = softGreen;
glm::vec3 lightIntensityStart = glm::vec3(0.9, 0.9, 0.9);
glm::vec3 lightIntensityEnd = glm::vec3(0.5, 0.5, 0.5);
glm::vec3 lightIntensity = lightIntensityStart;
#pragma endregion

#pragma region Spotlight properties
glm::vec3 spotlightPos = glm::vec3(5.51705, 2.0, 1.28056);
glm::vec3 spotlightDir = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 purple = glm::vec3(1.0, 0.0, 0.0);
#pragma endregion

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);

static unsigned loadImageToTexture(const char* filePath);
void initializeTexture(unsigned int VAO, unsigned int VBO, float* vertices, int verticesCount, unsigned indexTexture);

int main()
{
    if(!glfwInit())
    {
        std::cout << "GLFW fail!\n" << std::endl;
        return -1;
    }
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    const unsigned int width = mode->width;
    const unsigned int height = mode->height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Desert Scene", monitor, NULL);
    float aspectRatio = static_cast<float>(width) / height;

    if (!window)
    {
        std::cout << "Window fail!\n" << std::endl;
        glfwTerminate();
        return -2;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() !=GLEW_OK)
    {
        std::cout << "GLEW fail! :(\n" << std::endl;
        return -3;
    }

    glViewport(0, 0, mode->width, mode->height);

    Model trava("res/trava/uploads_files_3153011_grass_set.obj");
    Model lija("res/riba/fish.obj");
    Model piramida("res/pyramid/pyramid.obj");
    Model pustinja("res/pustinja/desert.obj");
    Model oaza("res/water/pool3dview.obj");
    
    float speed2 = 0.0005f;
    //Tjemena i baferi su definisani u model klasi i naprave se pri stvaranju objekata

    Shader unifiedShader("basic.vert", "basic.frag");
    unifiedShader.use();

    unsigned int VAO[9];
    glGenVertexArrays(9, VAO);
    unsigned int VBO[9];
    glGenBuffers(9, VBO);


    unsigned int stride = (3 + 4) * sizeof(float);

    float indexVertices[] = {
        // x,     y,     texCoordX, texCoordY
        1.0f, -1.0f,    1.0f,      0.0f,
        1.0f, -0.85f,   1.0f,      1.0f,
        0.7f, -1.0f,    0.0f,      0.0f,
        0.7f, -0.85f,   0.0f,      1.0f
    };
    unsigned indexTexture = loadImageToTexture("res/index/index2.png");
    initializeTexture(VAO[3], VBO[3], indexVertices, sizeof(indexVertices), indexTexture);

    
    //Render petlja
    unifiedShader.setVec3("uLightPos", 0, 1, 3);
    unifiedShader.setVec3("uViewPos", 0, 0, 5);
    unifiedShader.setVec3("uLightColor", 1, 1, 1);

    glm::mat4 projectionP = glm::perspective(glm::radians(45.0f), (float)aspectRatio, 0.1f, 100.0f); //Matrica perspektivne projekcije (FOV, Aspect Ratio, prednja ravan, zadnja ravan)
    glm::mat4 projectionO = glm::ortho(-6.0f, 6.0f, -6.0f, 6.0f, 0.1f, 100.0f); //Matrica ortogonalne projekcije (Lijeva, desna, donja, gornja, prednja i zadnja ravan)
    unifiedShader.setMat4("uP", projectionP);

    clock_t lastKeyPressTime = clock();
    
    float zoomLevel = 5.0f; // Pocetna vrednost za zum
    float zoomSpeed = 0.1; // Brzina kojom se menja zum
    float minimalniZoom = -1.0f; // Minimalna udaljenost kamere od objekta
    float maksimalniZoom = 20.0f;
    glm::mat4 view; //Matrica pogleda (kamere)
    zoomLevel = glm::clamp(zoomLevel, minimalniZoom, maksimalniZoom);


    glm::vec3 camPosition = glm::vec3(5.41705f, 12.3029f, 2.48056f);
    glm::vec3 camOrientation = glm::vec3(5.4f, 7.4f, 1.5f);
    glm::vec3 camRotation = glm::vec3(0.0f, 1.0f, 0.0f);

    view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
    unifiedShader.setMat4("uV", view);

    glm::mat4 model = glm::mat4(1.0f);
    unifiedShader.setMat4("uM", model);

    // material properties
    unifiedShader.setInt("material.diffuse", 0);
    unifiedShader.setInt("material.specular", 1);
    unifiedShader.setFloat("material.shininess", 32.0f);

    // directional light properties
    unifiedShader.setVec3("dirLight.direction", 0.0f, 0.0f, 0.0f);
    unifiedShader.setVec3("dirLight.color", softYellow);
    unifiedShader.setVec3("dirLight.intensity", dirLightIntensityStart);

    unifiedShader.setVec3("dirLight.ambient", 0.4f, 0.4f, 0.4f);
    unifiedShader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
    unifiedShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

    // point light properties
    unifiedShader.setVec3("pointLight.position", lightPos);
    unifiedShader.setVec3("pointLight.intensity", lightIntensity);

    unifiedShader.setVec3("pointLight.ambient", 0.07f, 0.07f, 0.07f);
    unifiedShader.setVec3("pointLight.diffuse", 2.8f, 2.8f, 2.8f);
    unifiedShader.setVec3("pointLight.specular", 4.0f, 4.0f, 4.0f);

    unifiedShader.setFloat("pointLight.constant", 1.0f);
    unifiedShader.setFloat("pointLight.linear", 0.09f);
    unifiedShader.setFloat("pointLight.quadratic", 0.032f);

    // spotlight properties
    unifiedShader.setVec3("spotLight.position", spotlightPos);
    unifiedShader.setVec3("spotLight.direction", spotlightDir);
    unifiedShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(20.0f)));
    unifiedShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(25.0f)));
    unifiedShader.setVec3("spotLight.color", purple);

    unifiedShader.setFloat("spotLight.constant", 1.0f);
    unifiedShader.setFloat("spotLight.linear", 0.09f);
    unifiedShader.setFloat("spotLight.quadratic", 0.032f);

    unifiedShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    unifiedShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    unifiedShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);

    unifiedShader.setBool("useTexture", false);
    unifiedShader.setBool("useDesert", false);
    unifiedShader.setBool("useWater", false);
    unifiedShader.setBool("useFish", false);
    unifiedShader.setBool("useGrass", false);
    unifiedShader.setBool("usePyramidModel", false);
    unifiedShader.setBool("useGourad", false);

    bool seeFish = false;
    float sunPosition = 1.0f;
    float currentTime = 0.0f;
    float speed = 0.001;
    float r = 1.0;
    float intensityChange = 1.0;
    const float intensityIncrement = 0.1f;
    unifiedShader.setFloat("intensityChange", intensityChange);
    glClearColor(0.7, 0.7, 1.0, 1.0);

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        bool speedKeyPressed = false;
        bool firstClick = true;
        float sensitivity = 100.0f;
        float angle = currentTime * speed;
        currentTime += 1;
        static double lastZoomTime = glfwGetTime();
        double currentTimeZoom = glfwGetTime();

        // Kretanje po sceni A, D, W, S, Z, X
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;
            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camPosition = glm::vec3(camPosition.x - 0.1, camPosition.y, camPosition.z);
                camOrientation = glm::vec3(camOrientation.x - 0.1, camOrientation.y, camOrientation.z);
                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);
            }

        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;

            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camPosition = glm::vec3(camPosition.x + 0.1, camPosition.y, camPosition.z);
                camOrientation = glm::vec3(camOrientation.x + 0.1, camOrientation.y, camOrientation.z);
                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);

            }

        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {


            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;

            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camPosition = glm::vec3(camPosition.x, camPosition.y + 0.1, camPosition.z);
                camOrientation = glm::vec3(camOrientation.x, camOrientation.y + 0.1, camOrientation.z);
                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);

            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;

            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camPosition = glm::vec3(camPosition.x, camPosition.y - 0.1, camPosition.z);
                camOrientation = glm::vec3(camOrientation.x, camOrientation.y - 0.1, camOrientation.z);
                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);

            }
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {


            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;

            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camPosition = glm::vec3(camPosition.x, camPosition.y, camPosition.z + 0.1);
                camOrientation = glm::vec3(camOrientation.x, camOrientation.y, camOrientation.z + 0.1);


                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);

            }
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;

            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camPosition = glm::vec3(camPosition.x, camPosition.y, camPosition.z - 0.1);
                camOrientation = glm::vec3(camOrientation.x, camOrientation.y, camOrientation.z - 0.1);
                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);

            }
        }
        // Pomeranje kamere LEVO, DESNO, GORE, DOLE
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;

            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camOrientation = glm::vec3(camOrientation.x - 0.1, camOrientation.y, camOrientation.z);
                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);

            }
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;

            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camOrientation = glm::vec3(camOrientation.x + 0.1, camOrientation.y, camOrientation.z);
                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);

            }
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;

            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camOrientation = glm::vec3(camOrientation.x, camOrientation.y + 0.1, camOrientation.z);
                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);

            }
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;

            if (!speedKeyPressed && elapsedTime > 0.01) {
                speedKeyPressed = true;  // Set the flag to true to indicate key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                camOrientation = glm::vec3(camOrientation.x, camOrientation.y - 0.1, camOrientation.z);
                view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
                unifiedShader.setMat4("uV", view);

            }
        }
        // Zoom in na + i zoom out na -
        if (currentTimeZoom - lastZoomTime > 0.1) { 
            if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
                zoomLevel -= zoomSpeed;
                zoomLevel = glm::max(zoomLevel, minimalniZoom);
            }
            else if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
                zoomLevel += zoomSpeed;
                zoomLevel = glm::min(zoomLevel, maksimalniZoom);
            }
            lastZoomTime = currentTimeZoom;
        }
        // ESCAPE
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        // TACKE, STRANICE, POLIGONI
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glEnable(GL_PROGRAM_POINT_SIZE);
            glPointSize(4);
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        // Promena projekcija
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            camPosition = glm::vec3(2.8f, 5.6f, 8.6f);
            camOrientation = glm::vec3(2.1f, 4.1f, 6.6f);
            camRotation = glm::vec3(0.0f, 1.0f, 0.0f);

            view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)

            unifiedShader.setMat4("uV", view);

            unifiedShader.setMat4("uP", projectionP);
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        {
            camPosition = glm::vec3(10.0f, 10.0f, 8.6f);
            camOrientation = glm::vec3(0.0f, 0.0f, 0.0f);
            camRotation = glm::vec3(0.0f, 1.0f, 0.0f);

            view = glm::lookAt(camPosition, camOrientation, camRotation); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)


            unifiedShader.setMat4("uV", view);
            unifiedShader.setMat4("uP", projectionO);

        }
        //Testiranje dubine
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
        {
            glEnable(GL_DEPTH_TEST); //Ukljucivanje testiranja Z bafera
        }
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
        {
            glDisable(GL_DEPTH_TEST);
        }
        //Odstranjivanje lica (Prethodno smo podesili koje lice uklanjamo sa glCullFace)
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
        {
            glEnable(GL_CULL_FACE);
        }
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
        {
            glDisable(GL_CULL_FACE);
        }
        // Sakrivanje trave
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS) {
            seeFish = true;
        }
        // Prikazivanje trave
        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS) {
            seeFish = false;
        }
        // Ispis koordinata kamere
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        {
            std::cout << camPosition.x << " cam pos x " << camOrientation.x << " cam ori x" << std::endl;
            std::cout << camPosition.y << " cam pos y " << camOrientation.y << " cam ori y" << std::endl;
            std::cout << camPosition.z << " cam pos z " << camOrientation.z << " cam ori z" << std::endl;

        }
        // Smanjivanje i pojacavanje intenziteta crvenog reflektora L i J
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;
            if (elapsedTime > 0.01) {  // Debounce the key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                intensityChange += intensityIncrement;
                unifiedShader.setFloat("intensityChange", intensityChange);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
            clock_t currentTime = clock();
            double elapsedTime = static_cast<double>(currentTime - lastKeyPressTime) / CLOCKS_PER_SEC;
            if (elapsedTime > 0.01) {  // Debounce the key press
                lastKeyPressTime = currentTime;  // Update the last key press time
                intensityChange -= intensityIncrement;
                unifiedShader.setFloat("intensityChange", intensityChange);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
            speedKeyPressed = false;
        }
        // Promena Fongovog i Guroovog osvetljenja
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        {
            unifiedShader.setBool("useGourad", true);
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        {
            unifiedShader.setBool("useGourad", false);
        }

        glm::vec3 direction = glm::normalize(camPosition - camOrientation);
        camPosition = camOrientation + direction * zoomLevel;
        view = glm::lookAt(camPosition, camOrientation, camRotation);
        unifiedShader.setMat4("uV", view);

        double time = glfwGetTime();
        float spotlightRadius = 5;
        spotlightDir = glm::vec3(-2.0f , -0.2f, sin(time) * 0.5 + 0.5);
        unifiedShader.setVec3("spotLight.direction", spotlightDir);


        //sky and light
        float t = (cos(time * sunPosition) + 1.0f) / 2.0f;

        float dirLightPosX = sin(time * sunPosition);
        float dirLightPosY = cos(time * sunPosition);
        float dirLightPosZ = 0.0f;

        lerpedDirLightColor = mix(softYellow, softGrey, t);
        lerpedBackroundColor = mix(lightBlue, darkBlue, t);
        lerpedDirLightIntensity = mix(dirLightIntensityStart, dirLightIntensityEnd, t);

        unifiedShader.setVec3("dirLight.direction", dirLightPosX, dirLightPosY, dirLightPosZ);
        unifiedShader.setVec3("dirLight.color", lerpedDirLightColor);
        unifiedShader.setVec3("dirLight.intensity", lerpedDirLightIntensity);
        glClearColor(lerpedBackroundColor.x, lerpedBackroundColor.y, lerpedBackroundColor.z, 1.0);


        // PUSTINJA
        unifiedShader.setBool("useDesert", true);
       glm::mat4 translationMatrix = glm::mat4(
            glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
            glm::vec4(3.0f, -0.25f, 0.0f, 1.0f)
        );
        unifiedShader.setMat4("translationMatrix", translationMatrix);
        unifiedShader.setFloat("scale", 2.0);
        pustinja.Draw(unifiedShader);
        unifiedShader.setBool("useDesert", false);


        // OAZA
        unifiedShader.setBool("useWater", true);

        glm::mat4 translationMatrix2 = glm::mat4(
            glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.1f, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
            glm::vec4(0.0f, -0.1f, 2.0f, 0.5f)
        );
        unifiedShader.setMat4("translationMatrix", translationMatrix2);
        unifiedShader.setFloat("scale", 0.02);
        oaza.Draw(unifiedShader);
        unifiedShader.setBool("useWater", false);


        // INDEX
        unifiedShader.setBool("useTexture", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, indexTexture);
        glBindVertexArray(VAO[3]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        unifiedShader.setBool("useTexture", false);

        
        if (!seeFish) {
            // TRAVA
            unifiedShader.setBool("useGrass", true);
            glm::mat4 translationMatrix3 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 2.0f));
            unifiedShader.setMat4("translationMatrix", translationMatrix3);
            unifiedShader.setFloat("scale", 2.5);
            trava.Draw(unifiedShader);
            unifiedShader.setBool("useGrass", false);
        }
        else {
            // RIBA
            unifiedShader.setBool("useFish", true);
            unifiedShader.setMat4("uM", model);
            currentTime += 1;

            float horizontalMovement2 = sin(time) * 1.0f;
            float verticalMovement2 = cos(time ) * 1.0f;

            glm::mat4 translationMatrix4 = glm::translate(glm::mat4(1.0f), glm::vec3(horizontalMovement2, -0.2f, horizontalMovement2 +2.0));
            unifiedShader.setMat4("translationMatrix", translationMatrix4);

            float rotationAngle2 = atan2(cos(time ), sin(time));
            glm::mat4 rotationMatrix2 = glm::rotate(glm::mat4(1.0f), rotationAngle2, glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.setMat4("rotationMatrix", rotationMatrix2);

            unifiedShader.setFloat("scale", 0.2);
            unifiedShader.setMat4("uM", model);
            lija.Draw(unifiedShader);
            unifiedShader.setBool("useFish", false);
        }


        // PIRAMIDE 
        unifiedShader.setBool("usePyramidModel", true);

        float scaleFactors[3] = { 0.7f, 0.4f, 0.2f }; // Faktori skaliranja
        glm::vec3 positions[3] = {
            glm::vec3(5.5f, 0.0f, 1.0f),  // Pozicija prve piramide
            glm::vec3(-2.0f, 0.0f, -2.0f), // Pozicija druge piramide
            glm::vec3(2.0f, 0.0f, -2.0f)   // Pozicija trece piramide
        };
        
        glm::vec3 pointLightPositions[3] = {
        glm::vec3(5.51705, 3.20288, 1.28056),
        glm::vec3(-2.1852, 3.827, -1.34884),
        glm::vec3(1.98089f, 3.56757f, -1.32073f)
        };

        
        lightIntensity = mix(lightIntensityEnd, lightIntensityStart, 1.0);

        for (int i = 0; i < 3; ++i) {
            
            unifiedShader.setVec3("pointLight.position", pointLightPositions[i]);
            unifiedShader.setVec3("pointLight.color", softGreen); 
            unifiedShader.setVec3("pointLight.intensity", lightIntensity);

            glm::mat4 translationMatrix5 = glm::translate(glm::mat4(1.0f), positions[i]);
            translationMatrix5 = glm::scale(translationMatrix5, glm::vec3(scaleFactors[i], scaleFactors[i], scaleFactors[i]));
            unifiedShader.setMat4("translationMatrix", translationMatrix5);
            piramida.Draw(unifiedShader);
        }
        unifiedShader.setBool("usePyramidModel", false);
      

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
   
    glDeleteTextures(1, &indexTexture);
    glDeleteBuffers(1, VBO);
    glDeleteVertexArrays(1, VAO);
    glDeleteProgram(unifiedShader.ID);
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
unsigned int createShader(const char* vsSource, const char* fsSource) {

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
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 4);
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
void initializeTexture(unsigned int VAO, unsigned int VBO, float* vertices, int verticesCount, unsigned indexTexture) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesCount, vertices, GL_STATIC_DRAW);

    // Pozicija
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Teksturne koordinate
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Tekstura
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, indexTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}


