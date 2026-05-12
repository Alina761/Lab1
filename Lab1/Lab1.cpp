#define GLEW_DLL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"

//КАМЕРА
glm::vec3 cameraPos = glm::vec3(3.0f, 3.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos; lastY = ypos; firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    xoffset *= 0.1f; yoffset *= 0.1f;
    yaw += xoffset; pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(dir);
}

//ШЕЙДЕРЫ
std::string readFile(const char* name) {
    std::ifstream f(name);
    return f.is_open() ? std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()) : "";
}

unsigned int compileShader(unsigned int type, const char* src) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, NULL, log);
        printf("Ошибка шейдера: %s\n", log);
    }
    return shader;
}

unsigned int createShaderProgram(const char* vpath, const char* fpath) {
    std::string vsrc = readFile(vpath);
    std::string fsrc = readFile(fpath);
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vsrc.c_str());
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fsrc.c_str());
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

//КООРДИНАТЫ ЦЕНТРОВ
glm::vec3 centerBase = glm::vec3(0.000000f, -0.486263f, 1.326860f);
glm::vec3 centerArm = glm::vec3(0.2279f, 0.130384f, 0.51433f);
glm::vec3 centerTube = glm::vec3(0.000000f, 0.536150f, 2.096110f);

float baseRotate = 0.0f;
float armPitch = 0.0f;          
float tubeShiftZ = 0.0f;        

int main() {
    SetConsoleOutputCP(1251);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Lab7 Variant 19", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glEnable(GL_DEPTH_TEST);

    unsigned int shaderProgram = createShaderProgram("vertex.glsl", "fragment.glsl");
    glUseProgram(shaderProgram);

    Model ourModel("models/my_model.obj");

    // Освещение
    glm::vec3 lightPos = glm::vec3(1.2f, 3.0f, 2.0f);
    glm::vec3 lightAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDiffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 lightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::vec3 materialAmbient = glm::vec3(0.3f, 1.0f, 1.0f);
    glm::vec3 materialDiffuse = glm::vec3(0.3f, 1.0f, 1.0f);
    glm::vec3 materialSpecular = glm::vec3(0.5f, 0.5f, 0.5f);
    float shininess = 64.0f;

    while (!glfwWindowShouldClose(window)) {
        float now = glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;
        float speed = 2.5f * deltaTime;
        float angleSpeed = 50.0f * deltaTime;

        // Камера
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) cameraPos += speed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) cameraPos -= speed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

        // Управление кинематикой
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) baseRotate += angleSpeed;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) baseRotate -= angleSpeed;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) armPitch += angleSpeed;
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) armPitch -= angleSpeed;
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) tubeShiftZ += speed;
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) tubeShiftZ -= speed;

        // Ограничения, чтобы не отрывалось
        if (armPitch > 30.0f) armPitch = 30.0f;
        if (armPitch < -10.0f) armPitch = -10.0f;
        if (tubeShiftZ < -0.5f) tubeShiftZ = -0.5f;
        if (tubeShiftZ > 0.0f) tubeShiftZ = 0.0f;

        // --- Кинематическая цепочка ---
        // Основание
        glm::mat4 baseMatrix = glm::mat4(1.0f);
        baseMatrix = glm::translate(baseMatrix, centerBase);
        baseMatrix = glm::rotate(baseMatrix, glm::radians(baseRotate), glm::vec3(0, 1, 0));
        baseMatrix = glm::translate(baseMatrix, -centerBase);

        // Вторая деталь
        glm::mat4 armMatrix = baseMatrix;
        armMatrix = glm::translate(armMatrix, centerArm);
        armMatrix = glm::rotate(armMatrix, glm::radians(armPitch), glm::vec3(0, 1, 0));
        armMatrix = glm::translate(armMatrix, -centerArm);

        // Третья деталь
        glm::mat4 tubeMatrix = armMatrix;
        tubeMatrix = glm::translate(tubeMatrix, centerTube);
        tubeMatrix = glm::translate(tubeMatrix, glm::vec3(0, 0, tubeShiftZ));
        tubeMatrix = glm::translate(tubeMatrix, -centerTube);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "light.ambient"), 1, glm::value_ptr(lightAmbient));
        glUniform3fv(glGetUniformLocation(shaderProgram, "light.diffuse"), 1, glm::value_ptr(lightDiffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, "light.specular"), 1, glm::value_ptr(lightSpecular));
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.ambient"), 1, glm::value_ptr(materialAmbient));
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.diffuse"), 1, glm::value_ptr(materialDiffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.specular"), 1, glm::value_ptr(materialSpecular));
        glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), shininess);

        ourModel.DrawPart(0, shaderProgram, baseMatrix);
        ourModel.DrawPart(1, shaderProgram, armMatrix);
        ourModel.DrawPart(2, shaderProgram, tubeMatrix);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}