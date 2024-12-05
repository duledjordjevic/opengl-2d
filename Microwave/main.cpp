#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>  
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp> 
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <chrono>
#include "Timer.h"
#include "Verticies.h"

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath);
unsigned createTexture(const char* filePath, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT,
    GLint minFilter = GL_NEAREST, GLint magFilter = GL_NEAREST);
void loadTextures(unsigned(&textures)[10], std::string prefix, std::string sufix);
void initializeButtons();
void handleNumpadButtonClick(int buttonId);
void handleFuncButtonClick(int buttonId);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void decrementTimer(Timer& timer);
void cursorCallback(GLFWwindow* window, double xpos, double ypos);
void startTimer(Timer& timer);
void pauseTimer(Timer& timer);
void stopTimer(Timer& timer);

struct Button {
    float xMin, xMax, yMin, yMax;
    int id; 
};


Button numpadButtons[10];
Button funcButtons[3];
Timer timer;
bool isMicrowaveOpen = false;
bool isMicrowaveWorking = true;
bool fixingInProgress = false;
int main(void)
{
    if (!glfwInit())
    {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth = 1200;
    unsigned int wHeight = 700;
    const char wTitle[] = "Microwave";
    //window = glfwCreateWindow(wWidth, wHeight, wTitle, glfwGetPrimaryMonitor(), NULL);  
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    unsigned int basicShader = createShader("basic.vert", "basic.frag");
    unsigned int textureShader = createShader("texture.vert", "texture.frag");

    unsigned int stride = (2 + 4) * sizeof(float);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData(koji bafer, koliko podataka ima, adresa podataka, nacin upotrebe podataka (GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW;)
    glBufferData(GL_ARRAY_BUFFER, sizeof(microwaveVertices), microwaveVertices, GL_STATIC_DRAW); //Slanje podataka na memoriju graficke karte

    //Opisivanje pokazivaca na atribute: Pokazivac 0 ceo opisati poziciju (koordinate x i y), a pokazivac 1 boju (komponente r, g, b i a).
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int stride2 = (2 + 2) * sizeof(float);

    unsigned int VAO2;
    glGenVertexArrays(1, &VAO2);
    glBindVertexArray(VAO2);
    unsigned int VBO2;
    glGenBuffers(1, &VBO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride2, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride2, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    unsigned foodTexture = createTexture("res/burger.png");
    unsigned startButtonTexture = createTexture("res/on.png");
    unsigned pauseButtonTexture = createTexture("res/next.png");
    unsigned stopButtonTexture = createTexture("res/off.png");
    unsigned diodeTexture = createTexture("res/diode.png");
    unsigned diodeBlueTexture = createTexture("res/diode-blue.png");
    unsigned diodeRedTexture = createTexture("res/diode-red.png");
    unsigned openMicrowaveTexture = createTexture("res/microwave-opened.png");
    unsigned lightTexture = createTexture("res/light3.png");
    unsigned nameTexture = createTexture("res/name.png");
    unsigned charRTexture = createTexture("res/r.png");
    unsigned charETexture = createTexture("res/e.png");
    unsigned charOTexture = createTexture("res/o.png");
    unsigned smokeTexture = createTexture("res/cloud-smoke.png");
    unsigned smokeGrayTexture = createTexture("res/cloud-smoke-gray.png");
    unsigned smokeBlackTexture = createTexture("res/cloud-smoke-black.png");
    unsigned smokeTransparentTexture = createTexture("res/cloud-smoke-transparent.png");

    unsigned numbersTextures[10];
    loadTextures(numbersTextures, "", "");
    unsigned colon = createTexture("res/colon.png");
    unsigned numpadTextures[10];
    loadTextures(numpadTextures, "", "_numpad");

    //glUseProgram(textureShader);
    //unsigned uTexLoc = glGetUniformLocation(textureShader, "uTex");
    //glUniform1i(uTexLoc, 0); // Indeks teksturne jedinice (sa koje teksture ce se citati boje)


    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float alphaLoc = glGetUniformLocation(basicShader, "uAlpha");
    float scaleLoc = glGetUniformLocation(textureShader, "uScale");
    float scale = 0.0f;

    double lastFrameTime = 0.0;
    const double targetFrameRate = 1.0 / 60.0;
    double currentTime = 0.0;

    initializeButtons();
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorCallback);

    auto lastUpdateTime = std::chrono::steady_clock::now();
    bool isWindowTransparent = true;
    bool wasTransparentKeyPressed = false;
    float dimmingLight = 0.01;
    
    while (!glfwWindowShouldClose(window))
    {
        currentTime = glfwGetTime();
        if (currentTime - lastFrameTime >= targetFrameRate) {
            lastFrameTime = currentTime;

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }

            auto currentRealTime = std::chrono::steady_clock::now();
            std::chrono::duration<float> elapsed = currentRealTime - lastUpdateTime;

            if (timer.running && !timer.paused && elapsed.count() >= 1.0f) {
                decrementTimer(timer);
                lastUpdateTime = currentRealTime;
            }

            static double lastBlinkTime = 0.0;
            static bool showTexture = false;

            if (currentTime - lastBlinkTime >= 0.5) { 
                showTexture = !showTexture; 
                lastBlinkTime = currentTime; 
            }

            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(basicShader);
            if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            {
                if (!wasTransparentKeyPressed) {
                    if (isWindowTransparent) {
                        glUniform1f(alphaLoc, 0.5);
                        isWindowTransparent = false;
                    }
                    else {
                        glUniform1f(alphaLoc, 0);
                        isWindowTransparent = true;
                    }
                    wasTransparentKeyPressed = true;
                }
            }
            else {
                wasTransparentKeyPressed = false;
            }
            if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
            {
                    isMicrowaveOpen = true;
                    pauseTimer(timer);
            }
            if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
                    isMicrowaveOpen = false;
            }
            glBindVertexArray(VAO);
            if (!isMicrowaveOpen) {
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
                glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
                glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
                glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
            }
            glDrawArrays(GL_TRIANGLE_STRIP, 24, 4);
            glDrawArrays(GL_TRIANGLE_STRIP, 28, 4);

            glUseProgram(textureShader);
            glBindVertexArray(VAO2);
            glActiveTexture(GL_TEXTURE0);
         

            if (isMicrowaveWorking) {
                for (int i = 0; i < 4; i++) {
                    if (timer.counter == i) {
                        if (!showTexture && !timer.running && !timer.paused) continue;
                    }
                    glBindTexture(GL_TEXTURE_2D, numbersTextures[timer.time[i]]);
                    glDrawArrays(GL_TRIANGLE_STRIP, 4 * (i + 1), 4);
                }
                glBindTexture(GL_TEXTURE_2D, colon);
                glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
            }
            else {
                if (showTexture) {
                    glBindTexture(GL_TEXTURE_2D, charETexture);
                    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
                    glBindTexture(GL_TEXTURE_2D, charRTexture);
                    glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
                    glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
                    glBindTexture(GL_TEXTURE_2D, charOTexture);
                    glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
                    glBindTexture(GL_TEXTURE_2D, charRTexture);
                    glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
                }
            }
            


            glBindTexture(GL_TEXTURE_2D, numpadTextures[1]);
            glDrawArrays(GL_TRIANGLE_STRIP, 24, 4);
            glBindTexture(GL_TEXTURE_2D, numpadTextures[2]);
            glDrawArrays(GL_TRIANGLE_STRIP, 28, 4);
            glBindTexture(GL_TEXTURE_2D, numpadTextures[3]);
            glDrawArrays(GL_TRIANGLE_STRIP, 32, 4);
            glBindTexture(GL_TEXTURE_2D, numpadTextures[4]);
            glDrawArrays(GL_TRIANGLE_STRIP, 36, 4);
            glBindTexture(GL_TEXTURE_2D, numpadTextures[5]);
            glDrawArrays(GL_TRIANGLE_STRIP, 40, 4);
            glBindTexture(GL_TEXTURE_2D, numpadTextures[6]);
            glDrawArrays(GL_TRIANGLE_STRIP, 44, 4);
            glBindTexture(GL_TEXTURE_2D, numpadTextures[7]);
            glDrawArrays(GL_TRIANGLE_STRIP, 48, 4);
            glBindTexture(GL_TEXTURE_2D, numpadTextures[8]);
            glDrawArrays(GL_TRIANGLE_STRIP, 52, 4);
            glBindTexture(GL_TEXTURE_2D, numpadTextures[9]);
            glDrawArrays(GL_TRIANGLE_STRIP, 56, 4);
            glBindTexture(GL_TEXTURE_2D, numpadTextures[0]);
            glDrawArrays(GL_TRIANGLE_STRIP, 60, 4);
            glBindTexture(GL_TEXTURE_2D, startButtonTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 64, 4);
            glBindTexture(GL_TEXTURE_2D, pauseButtonTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 68, 4);
            glBindTexture(GL_TEXTURE_2D, stopButtonTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 72, 4);
            if (timer.running && !timer.paused && showTexture) {
                glBindTexture(GL_TEXTURE_2D, diodeTexture);
                glDrawArrays(GL_TRIANGLE_STRIP, 76, 4);
            }
            else if(timer.running && timer.paused) {
                glBindTexture(GL_TEXTURE_2D, diodeBlueTexture);
                glDrawArrays(GL_TRIANGLE_STRIP, 76, 4);
            }
            else if(!timer.running){
                glBindTexture(GL_TEXTURE_2D, diodeRedTexture);
                glDrawArrays(GL_TRIANGLE_STRIP, 76, 4);
            }


            if (isMicrowaveOpen) {
                glBindTexture(GL_TEXTURE_2D, openMicrowaveTexture);
                glDrawArrays(GL_TRIANGLE_STRIP, 80, 4);
            }
           
            glBindTexture(GL_TEXTURE_2D, foodTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            if (timer.running && !timer.paused) {
                glBindTexture(GL_TEXTURE_2D, lightTexture);
                glDrawArrays(GL_TRIANGLE_STRIP, 84, 4);
            }

            glBindTexture(GL_TEXTURE_2D, nameTexture);
            glDrawArrays(GL_TRIANGLE_STRIP, 88, 4);

            glBindTexture(GL_TEXTURE_2D, 0);
            glUseProgram(basicShader);
            glBindVertexArray(VAO);
            if (!isMicrowaveOpen) {
                glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
            }

            if (timer.running && glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            {
                isMicrowaveWorking = false;
                stopTimer(timer);
            }

            if (!isMicrowaveWorking && !fixingInProgress) {
                glUniform1f(alphaLoc, dimmingLight);
                glDrawArrays(GL_TRIANGLE_STRIP, 32, 4);

                if (dimmingLight < 0.8) {
                    dimmingLight += 0.01;
                }

                if (scale >= 0.0 && scale < 2.1) {
                    scale += 0.01;
                }
                else {
                    scale = -1.0;
                }
                glUniform1f(alphaLoc, 0);
            }

            if (!isMicrowaveWorking && glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
                fixingInProgress = true;
            }


            if (!isMicrowaveWorking && !fixingInProgress && scale != -1.0) {
                glUseProgram(textureShader);
                glBindVertexArray(VAO2);

                glUniform1f(scaleLoc, scale);
                if(scale < 0.5)
                    glBindTexture(GL_TEXTURE_2D, smokeBlackTexture);
                else if (scale > 0.5 && scale < 1.2)
                    glBindTexture(GL_TEXTURE_2D, smokeGrayTexture);
                else if(scale > 1.2 && scale < 1.6)
                    glBindTexture(GL_TEXTURE_2D, smokeTexture);
                else if (scale > 1.6 )
                    glBindTexture(GL_TEXTURE_2D, smokeTransparentTexture);

                glDrawArrays(GL_TRIANGLE_STRIP, 92, 4);
                glUniform1f(scaleLoc, 0.0);
            }

            glUseProgram(basicShader);
            glBindVertexArray(VAO);

            if (!isMicrowaveWorking && fixingInProgress) {
                glUniform1f(alphaLoc, dimmingLight);
                glDrawArrays(GL_TRIANGLE_STRIP, 32, 4);

                if (dimmingLight < 0.0) {
                    isMicrowaveWorking = true;
                    fixingInProgress = false;
                    scale = 0.0f;
                }
                dimmingLight -= 0.01;

                glUniform1f(alphaLoc, 0);
            }


            glBindVertexArray(0);
            glUseProgram(0);


            glfwSwapBuffers(window);
            glfwPollEvents();

        }
    }


    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(basicShader);
    glfwTerminate();
    return 0;
}

void startTimer(Timer& timer) {
    if (isMicrowaveWorking) {
        if (!timer.running) {
            if (!(timer.time[0] == 0 && timer.time[1] == 0 && timer.time[2] == 0 && timer.time[3] == 0) && !isMicrowaveOpen) {
                timer.running = true;
            }
        }
        if (!isMicrowaveOpen) {
            timer.paused = false;
            timer.counter = 0;
        }
    }
}
void pauseTimer(Timer& timer) {
    if (isMicrowaveWorking) {
        if (timer.running) {
            timer.paused = true;
        }
    }
}
void stopTimer(Timer& timer) {
    timer.running = false;
    timer.paused = false;
    timer.time[0] = timer.time[1] = timer.time[2] = timer.time[3] = 0;
    timer.counter = 0;
}
void decrementTimer(Timer& timer) {
    int hours = timer.time[0] * 10 + timer.time[1];
    int minutes = timer.time[2] * 10 + timer.time[3];
    int totalSeconds = hours * 3600 + minutes * 60;

    if (totalSeconds > 0) {
        totalSeconds--; 
    }
    else {
        timer.running = false; 
        return;
    }

    hours = totalSeconds / 3600;
    minutes = (totalSeconds % 3600) / 60;

    timer.time[0] = hours / 10;
    timer.time[1] = hours % 10;
    timer.time[2] = minutes / 10;
    timer.time[3] = minutes % 10;
}
void handleFuncButtonClick(int buttonId) {
    switch (buttonId) {
    case 0: 
        startTimer(timer);
        break;
    case 1: 
        pauseTimer(timer);
        break;
    case 2: 
        stopTimer(timer);
        break;
    default:
        break;
    }
}
void handleNumpadButtonClick(int buttonId) {
    timer.setNextValue(buttonId);
}
void initializeButtons() {
    numpadButtons[0] = { 0.61, 0.72, -0.16, -0.06, 0 };
    numpadButtons[1] = { 0.49, 0.60, 0.2, 0.3, 1 };
    numpadButtons[2] = { 0.61, 0.72, 0.2, 0.3, 2 };
    numpadButtons[3] = { 0.73, 0.84, 0.2, 0.3, 3 };
    numpadButtons[4] = { 0.49, 0.60, 0.08, 0.18, 4 };
    numpadButtons[5] = { 0.61, 0.72, 0.08, 0.18, 5 };
    numpadButtons[6] = { 0.73, 0.84, 0.08, 0.18, 6 };
    numpadButtons[7] = { 0.49, 0.60, -0.04, 0.06, 7 };
    numpadButtons[8] = { 0.61, 0.72, -0.04, 0.06, 8 };
    numpadButtons[9] = { 0.73, 0.84, -0.04, 0.06, 9 };

    funcButtons[0] = { 0.49, 0.60, -0.38, -0.24, 0 };
    funcButtons[1] = { 0.61, 0.72, -0.38, -0.24, 1 };
    funcButtons[2] = { 0.73, 0.84, -0.38, -0.24, 2 };
}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (isMicrowaveWorking) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            int width, height;
            glfwGetWindowSize(window, &width, &height);

            float xNormalized = (xpos / width) * 2.0f - 1.0f;
            float yNormalized = 1.0f - (ypos / height) * 2.0f; 


            for (const Button& button : numpadButtons) {
                if (xNormalized >= button.xMin && xNormalized <= button.xMax &&
                    yNormalized >= button.yMin && yNormalized <= button.yMax) {
                    std::cout << "Kliknut numpad taster: " << button.id << std::endl;
                    handleNumpadButtonClick(button.id);
                    break;
                }
            }
            for (const Button& button : funcButtons) {
                if (xNormalized >= button.xMin && xNormalized <= button.xMax &&
                    yNormalized >= button.yMin && yNormalized <= button.yMax) {
                    std::cout << "Kliknut func taster: " << button.id << std::endl;
                    handleFuncButtonClick(button.id);
                    break;
                }
            }
       
        }
    }
}
void cursorCallback(GLFWwindow* window, double xpos, double ypos) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    float xNormalized = (xpos / width) * 2.0f - 1.0f;
    float yNormalized = 1.0f - (ypos / height) * 2.0f;

    bool cursorOverButton = false;
    for (const Button& button : numpadButtons) {
        if (xNormalized >= button.xMin && xNormalized <= button.xMax &&
            yNormalized >= button.yMin && yNormalized <= button.yMax) {
            cursorOverButton = true;
            break;
        }
    }
    for (const Button& button : funcButtons) {
        if (xNormalized >= button.xMin && xNormalized <= button.xMax &&
            yNormalized >= button.yMin && yNormalized <= button.yMax) {
            cursorOverButton = true;
            break;
        }
    }

    if (cursorOverButton) {
        glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HAND_CURSOR));
    }
    else {
        glfwSetCursor(window, glfwCreateStandardCursor(GLFW_ARROW_CURSOR));
    }
   
}
void loadTextures(unsigned(&textures)[10], std::string prefix, std::string sufix) {
    for (int i = 0; i < 10; ++i) {
        std::string filePath = "res/" + prefix + std::to_string(i) + sufix  +".png";

        textures[i] = createTexture(filePath.c_str());

        if (textures[i] == 0) {
            std::cout << "Greska pri ucitavanju teksture za broj " << i
                << " iz fajla: " << filePath << std::endl;
        }
    }
}
unsigned createTexture(const char* filePath, GLint wrapS, GLint wrapT,GLint minFilter, GLint magFilter) {

    unsigned texture = loadImageToTexture(filePath);
    if (texture == 0) {
        std::cout << "Greska pri ucitavanju teksture: " << filePath << std::endl;
        return 0; // Vrati 0 ako nije uspelo
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glBindTexture(GL_TEXTURE_2D, 0); 

    return texture;
}
unsigned int compileShader(GLenum type, const char* source)
{
    //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
    //Citanje izvornog koda iz fajla
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
    const char* sourceCode = temp.c_str(); //Izvorni kod sejdera koji citamo iz fajla na putanji "source"

    int shader = glCreateShader(type); //Napravimo prazan sejder odredjenog tipa (vertex ili fragment)

    int success; //Da li je kompajliranje bilo uspjesno (1 - da)
    char infoLog[512]; //Poruka o gresci (Objasnjava sta je puklo unutar sejdera)
    glShaderSource(shader, 1, &sourceCode, NULL); //Postavi izvorni kod sejdera
    glCompileShader(shader); //Kompajliraj sejder

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //Provjeri da li je sejder uspjesno kompajliran
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog); //Pribavi poruku o gresci
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
    //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource

    unsigned int program; //Objedinjeni sejder
    unsigned int vertexShader; //Verteks sejder (za prostorne podatke)
    unsigned int fragmentShader; //Fragment sejder (za boje, teksture itd)

    program = glCreateProgram(); //Napravi prazan objedinjeni sejder program

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); //Napravi i kompajliraj vertex sejder
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); //Napravi i kompajliraj fragment sejder

    //Zakaci verteks i fragment sejdere za objedinjeni program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //Povezi ih u jedan objedinjeni sejder program
    glValidateProgram(program); //Izvrsi provjeru novopecenog programa

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); //Slicno kao za sejdere
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    //Posto su kodovi sejdera u objedinjenom sejderu, oni pojedinacni programi nam ne trebaju, pa ih brisemo zarad ustede na memoriji
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
        case 2: InternalFormat = GL_RG; break;
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
        std::cout << "STB Error: " << stbi_failure_reason() << "\n";
        stbi_image_free(ImageData);
        return 0;
    }
}