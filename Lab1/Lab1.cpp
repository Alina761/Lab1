#define GLEW_DLL
// #define GLFW_DLL

#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

// Подключаем GLM для матриц и векторов
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// БИБЛИОТЕКА ДЛЯ РАБОТЫ С ШЕЙДЕРАМИ

std::string readShaderFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        fprintf(stderr, "Не удалось открыть файл: %s\n", filename);
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

unsigned int compileShaderFromFile(unsigned int type, const char* filename) {
    std::string source = readShaderFile(filename);
    if (source.empty()) return 0;

    const char* sourceCStr = source.c_str();
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &sourceCStr, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        const char* shaderType = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        fprintf(stderr, "ОШИБКА компиляции шейдера (%s) из файла %s:\n%s\n", shaderType, filename, infoLog);
        return 0;
    }
    return shader;
}

unsigned int createShaderProgramFromFiles(const char* vertexPath, const char* fragmentPath) {
    unsigned int vertexShader = compileShaderFromFile(GL_VERTEX_SHADER, vertexPath);
    unsigned int fragmentShader = compileShaderFromFile(GL_FRAGMENT_SHADER, fragmentPath);

    if (vertexShader == 0 || fragmentShader == 0) return 0;

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[1024];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 1024, NULL, infoLog);
        fprintf(stderr, "ОШИБКА линковки:\n%s\n", infoLog);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

// ПЕРЕМЕННЫЕ КАМЕРЫ (глобальные для callback)

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;   // начальный угол рыскания
float pitch = 0.0f;     // начальный угол тангажа
float lastX = 400.0f;   // центр окна (ширина/2)
float lastY = 300.0f;   // центр окна (высота/2)
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// CALLBACK ДЛЯ МЫШИ

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Ограничение pitch (чтобы не переворачивать камеру)
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Вычисляем новый вектор направления
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// ГЛАВНАЯ ФУНКЦИЯ

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    // Инициализация GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Ошибка инициализации GLFW\n");
        return -1;
    }

    // Настройка OpenGL 4.6
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 600,
        "Lab4 Pototskaya Alina ASUb-24-1", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Ошибка создания окна\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    // Инициализация GLEW
    GLenum ret = glewInit();
    if (ret != GLEW_OK) {
        fprintf(stderr, "Ошибка GLEW: %s\n", glewGetErrorString(ret));
        return -1;
    }

    // Скрываем курсор и захватываем его в окне
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    printf("Управление: WASD - движение, Мышь - поворот камеры\n");

    // ДАННЫЕ ТРЕУГОЛЬНИКА
    float vertices[] = {
         0.0f,  0.5f,   // верхняя (0)
        -0.5f, -0.3f,   // левая нижняя (1)
         0.5f, -0.3f    // правая нижняя (2)
    };

    unsigned int indices[] = { 0, 1, 2 };

    // VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ШЕЙДЕРНАЯ ПРОГРАММА
    unsigned int shaderProgram = createShaderProgramFromFiles("vertex.glsl", "fragment.glsl");
    if (shaderProgram == 0) {
        fprintf(stderr, "Не удалось создать шейдерную программу\n");
        glfwTerminate();
        return -1;
    }

    // Включаем глубину
    glEnable(GL_DEPTH_TEST);

    // ОСНОВНОЙ ЦИКЛ
    while (!glfwWindowShouldClose(window)) {
        // Выход по Esc
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        // Время для анимации цвета
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Цвет от времени (как в Lab2)
        float red = (sin(currentFrame) + 1.0f) / 2.0f;
        float green = (sin(currentFrame + 2.0f) + 1.0f) / 2.0f;
        float blue = (sin(currentFrame + 4.0f) + 1.0f) / 2.0f;

        // УПРАВЛЕНИЕ КАМЕРОЙ
        float cameraSpeed = 2.5f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

        // МАТРИЦЫ
        // Матрица проекции 
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // Матрица вида (камера)
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Матрица модели
        glm::mat4 model = glm::mat4(1.0f);

        // ОТРИСОВКА
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);  // тёмно-серый фон
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Передаём матрицы в шейдер
        int projLoc = glGetUniformLocation(shaderProgram, "projection");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Передаём цвет
        int colorLoc = glGetUniformLocation(shaderProgram, "ourColor");
        glUniform4f(colorLoc, red, green, blue, 1.0f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}