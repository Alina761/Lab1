#define GLEW_DLL
// #define GLFW_DLL  // закомментировано для статической версии

#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

// БИБЛИОТЕКА ДЛЯ РАБОТЫ С ШЕЙДЕРАМИ (Задание 2)

// Чтение файла шейдера
std::string readShaderFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        fprintf(stderr, "Не удалось открыть файл: %s\n", filename);
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

// Компиляция шейдера из файла
unsigned int compileShaderFromFile(unsigned int type, const char* filename) {
    std::string source = readShaderFile(filename);
    if (source.empty()) return 0;

    const char* sourceCStr = source.c_str();
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &sourceCStr, NULL);
    glCompileShader(shader);

    // Проверка ошибок компиляции
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

// Создание шейдерной программы из файлов
unsigned int createShaderProgramFromFiles(const char* vertexPath, const char* fragmentPath) {
    unsigned int vertexShader = compileShaderFromFile(GL_VERTEX_SHADER, vertexPath);
    unsigned int fragmentShader = compileShaderFromFile(GL_FRAGMENT_SHADER, fragmentPath);

    if (vertexShader == 0 || fragmentShader == 0) {
        fprintf(stderr, "Не удалось скомпилировать один из шейдеров\n");
        return 0;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Проверка ошибок линковки
    int success;
    char infoLog[1024];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 1024, NULL, infoLog);
        fprintf(stderr, "ОШИБКА линковки шейдерной программы:\n%s\n", infoLog);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

// УТИЛИТЫ ДЛЯ UNIFORM (одна строка)

// Установка uniform vec4 (цвет)
void setUniformVec4(unsigned int program, const char* name, float x, float y, float z, float w) {
    glUseProgram(program);
    int location = glGetUniformLocation(program, name);
    glUniform4f(location, x, y, z, w);
}

// Установка uniform float
void setUniformFloat(unsigned int program, const char* name, float value) {
    glUseProgram(program);
    int location = glGetUniformLocation(program, name);
    glUniform1f(location, value);
}

// ГЛАВНАЯ ФУНКЦИЯ

int main() {
    // 1. Инициализация GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Ошибка инициализации GLFW\n");
        return -1;
    }

    // 2. Настройка версии OpenGL 4.6 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 600,
        "Pototskaya Alina - ASUb-24-1 - Variant 19", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Ошибка создания окна\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    // 4. Инициализация GLEW
    GLenum ret = glewInit();
    if (ret != GLEW_OK) {
        fprintf(stderr, "Ошибка GLEW: %s\n", glewGetErrorString(ret));
        return -1;
    }

    // Вывод информации о системе
    SetConsoleOutputCP(1251);
    printf("Вариант 19: Треугольник, цвет меняется от времени\n");
    printf("Шейдеры загружаются из файлов: vertex.glsl, fragment.glsl\n");

    // 5. ДАННЫЕ ДЛЯ ФИГУРЫ (ТРЕУГОЛЬНИК)

    // Вершины треугольника (координаты x, y)
    float vertices[] = {
         0.0f,  0.5f,   // верхняя вершина (0)
        -0.5f, -0.3f,   // левая нижняя (1)
         0.5f, -0.3f    // правая нижняя (2)
    };

    // Индексы вершин (порядок отрисовки)
    unsigned int indices[] = {
        0, 1, 2   // один треугольник из трёх вершин
    };

    // 6. СОЗДАНИЕ VAO, VBO, EBO
    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Привязываем VAO (все настройки ниже будут сохранены в нём)
    glBindVertexArray(VAO);

    // Настройка VBO (буфер вершин)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Настройка EBO (буфер индексов)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Указываем, как интерпретировать данные вершин
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Отвязываем VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 7. СОЗДАНИЕ ШЕЙДЕРНОЙ ПРОГРАММЫ ИЗ ФАЙЛОВ (Задание 2)
    unsigned int shaderProgram = createShaderProgramFromFiles("vertex.glsl", "fragment.glsl");
    if (shaderProgram == 0) {
        fprintf(stderr, "Не удалось создать шейдерную программу\n");
        glfwTerminate();
        return -1;
    }

    // 8. ОСНОВНОЙ ЦИКЛ РЕНДЕРИНГА
    while (!glfwWindowShouldClose(window)) {
        // Очистка экрана (белый фон)
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ЗАДАНИЕ 1: ИЗМЕНЕНИЕ ЦВЕТА ОТ ВРЕМЕНИ
        double currentTime = glfwGetTime();

        // Используем синусоидальный закон для плавного изменения цветов
        float red = (sin(currentTime) + 1.0f) / 2.0f;
        float green = (sin(currentTime + 2.0f) + 1.0f) / 2.0f;
        float blue = (sin(currentTime + 4.0f) + 1.0f) / 2.0f;

        // Передаём цвет в шейдер (ОДНА СТРОКА!)
        setUniformVec4(shaderProgram, "ourColor", red, green, blue, 1.0f);

        // Рисуем треугольник
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        // Смена буферов и обработка событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 9. ОЧИСТКА РЕСУРСОВ
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}