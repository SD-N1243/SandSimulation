#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "palette.h"

#define SCREEN_W 800
#define SCREEN_H 600
#define FPS 16
#define MAX_COLOURS 256 // 8-bit palette

// TYPE DEFINES
#define TYPE_EMPTY  0
#define TYPE_SAND   1
#define TYPE_WATER  2
#define TYPE_STONE  3
#define TYPE_FIRE   4
// TYPE DEFINES

typedef struct
{
    uint8_t type; // determines both behavior and color
} Particle;

Particle World[SCREEN_W][SCREEN_H];

double mouseX, mouseY;
int brushSize = 64;
bool isSimulationRunning = true;

void drawBrushBox(GLFWwindow *window, int brushSize)
{
    double mx_d, my_d;
    glfwGetCursorPos(window, &mx_d, &my_d);

    int mx = (int)mx_d;
    int my = (int)my_d; // top-left origin

    int half = brushSize / 2;

    int left   = mx - half;
    int right  = mx + half;
    int top    = my + half;
    int bottom = my - half;

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_LINE_LOOP);
        glVertex2i(left, bottom);
        glVertex2i(right, bottom);
        glVertex2i(right, top);
        glVertex2i(left, top);
    glEnd();
}

void renderScene(GLFWwindow *window)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);

    glBegin(GL_POINTS);
    for (size_t x = 0; x < SCREEN_W; x++)
    {
        for (size_t y = 0; y < SCREEN_H; y++)
        {
            uint8_t type = World[x][y].type;

            // Skip empty cells if desired
            if (type == TYPE_EMPTY)
                continue;

            uint8_t colorIndex = type;
            unsigned char r = palette_rgb[colorIndex][0];
            unsigned char g = palette_rgb[colorIndex][1];
            unsigned char b = palette_rgb[colorIndex][2];

            glColor3f(r / 255.0f, g / 255.0f, b / 255.0f);
            glVertex2i(x, y);
        }
    }
    glEnd();

    drawBrushBox(window, brushSize + 1);

    Sleep(FPS);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // yoffset = scroll up (+) or down (-)
    printf("Scroll: %.2f\n", yoffset);

    // Example: zoom, change brush size, or switch materials
    if (yoffset > 0)
        brushSize++;
    else if (yoffset < 0 && brushSize > 1)
        brushSize--;

    printf("Brush size: %d\n", brushSize);
}

void spawnParticles(GLFWwindow *window, uint8_t particleType)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    int x = (int)mouseX;
    int y = (int)mouseY; // top-left origin

    Particle TmpParticle;
    TmpParticle.type = particleType;

    int half = brushSize / 2;

    for (int i = -half; i <= half; i++)
    {
        for (int j = -half; j <= half; j++)
        {
            int px = x + i;
            int py = y + j;

            if (px >= 0 && px < SCREEN_W && py >= 0 && py < SCREEN_H)
            {
                World[px][py] = TmpParticle;
            }
        }
    }
}

void processInput(GLFWwindow *window)
{

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (isSimulationRunning == true)
        {
            isSimulationRunning = false;
        } else
        {
            isSimulationRunning = true;
        }
        
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        spawnParticles(window, TYPE_SAND);
    }
}

void updateWorld()
{
    for (int y = SCREEN_H - 2; y >= 0; y--) // bottom-up, avoid bottom edge
    {
        for (int x = 0; x < SCREEN_W; x++)
        {
            switch (World[x][y].type)
            {
                case TYPE_SAND:
                {
                    // Check below
                    if (World[x][y + 1].type == TYPE_EMPTY)
                    {
                        World[x][y + 1].type = TYPE_SAND;
                        World[x][y].type = TYPE_EMPTY;
                    }
                    else
                    {
                        // Try diagonals
                        int rng_dir = rand() % 2; // 0 or 1

                        if (rng_dir == 0)
                        {
                            // down-right
                            if (x < SCREEN_W - 1 && World[x + 1][y + 1].type == TYPE_EMPTY)
                            {
                                World[x + 1][y + 1].type = TYPE_SAND;
                                World[x][y].type = TYPE_EMPTY;
                            }
                        }
                        else
                        {
                            // down-left
                            if (x > 0 && World[x - 1][y + 1].type == TYPE_EMPTY)
                            {
                                World[x - 1][y + 1].type = TYPE_SAND;
                                World[x][y].type = TYPE_EMPTY;
                            }
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }
}

int main(int argc, char **argv)
{
    // Initialize GLUT first (needed for fonts/text)
    glutInit(&argc, argv);

    if (!glfwInit()) return -1;

    GLFWwindow *window = glfwCreateWindow(SCREEN_W, SCREEN_H, "Sand Simulation", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Flip Y so (0,0) is top-left
    glOrtho(0, SCREEN_W, SCREEN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    for (size_t y = 0; y < SCREEN_W; y++)
    {
        for (size_t x = 0; x < SCREEN_H; x++)
        {
            World[x][y].type = TYPE_EMPTY;
        }
    }

    while (!glfwWindowShouldClose(window))
    {
        glLoadIdentity();
        processInput(window);
        glfwSetScrollCallback(window, scroll_callback);

        if (isSimulationRunning)
        {
            updateWorld();
        }

        renderScene(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
