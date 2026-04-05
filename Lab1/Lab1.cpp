#define GLEW_DLL
#define GLFW_DLL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>

// === ВАРИАНТ 19: Треугольник ===
// Цвет фигуры: (0.3, 1.0, 1.0) - голубовато-бирюзовый
// Цвет фона: (1.0, 1.0, 1.0) - белый
float figureColor[3] = { 0.3f, 1.0f, 1.0f };
float bgColor[3] = { 1.0f, 1.0f, 1.0f };

void drawFigure() {
    // Рисуем треугольник
    glBegin(GL_TRIANGLES);
    glColor3f(figureColor[0], figureColor[1], figureColor[2]);

    // Вершины треугольника
    glVertex2f(0.0f, 0.5f);   // верхняя вершина
    glVertex2f(-0.5f, -0.3f);  // левая нижняя вершина
    glVertex2f(0.5f, -0.3f);   // правая нижняя вершина
    glEnd();
}

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Ошибка инициализации GLFW\n");
        return -1;
    }

    // Настройка версии OpenGL 1.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Создание окна (800x600, заголовок на латинице)
    GLFWwindow* window = glfwCreateWindow(800, 600,
        "Pototskaya Alina - ASUb-24-1 - Variant 19", NULL, NULL);
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

    // Вывод информации о версии OpenGL
    printf("OpenGL версия: %s\n", glGetString(GL_VERSION));
    printf("Устройство: %s\n", glGetString(GL_RENDERER));
    printf("GLEW версия: %s\n", glewGetString(GLEW_VERSION));
    printf("Вариант 19: Треугольник (цвет: 0.3,1.0,1.0 | фон: белый)\n");

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        // Очистка экрана цветом фона
        glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Рисуем треугольник
        drawFigure();

        // Смена буферов и обработка событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Завершение работы
    glfwTerminate();
    return 0;
}