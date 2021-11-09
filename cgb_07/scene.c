#include <stdlib.h>
#include "scene.h"
#include "camera.h"
#include "texture.h"

static const color thmGray = {0.29f, 0.36f, 0.4f, 1.0f};
static const color thmGreen = {0.5f, 0.73f, 0.14f, 1.0f};
static const color thmYellow = {0.96f, 0.67f, 0.0f, 1.0f};
static const color thmRed = {0.61f, 0.07f, 0.18f, 1.0f};
static const color sunLight = {1.0f, 1.0f, 1.0f, 1.0f};
static const color ambientLight = {0.01f, 0.01f, 0.01f, 1.0f};

static mesh greenCube;
static mesh yellowCube;
static mesh redSphere;

static GLuint thmTexture;
static GLuint earthTexture;

void loadScene(GLFWwindow* window)
{
    setViewportSize(window);

    glClearColor(thmGray.r, thmGray.g, thmGray.b, thmGray.a);

    greenCube = createCubeMesh(thmGreen);
    redSphere = createSphereMesh(thmRed);
    yellowCube = createCubeMesh(thmYellow);

    thmTexture = loadTexture("res/thm2k.png");
    earthTexture = loadTexture("res/earth8k.jpg");

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, &sunLight);
    glLightfv(GL_LIGHT1, GL_AMBIENT, &ambientLight);
}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    loadCameraViewMatrix();

    vector4 lightPosition = {50000, 5000, 50000, 0};
	glLightfv(GL_LIGHT1, GL_POSITION, &lightPosition);

    renderMesh(yellowCube, matrixTranslate(-3,0,0), thmTexture);
    renderMesh(redSphere, matrixTranslate(0,0,0), earthTexture);
    renderMesh(greenCube, matrixTranslate(3,0,0), thmTexture);
}

void unloadScene()
{
    free(greenCube.vertices);
    free(yellowCube.vertices);
    free(redSphere.vertices);
}

void setViewportSize(GLFWwindow* window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    loadCameraProjectionMatrix((float)width / (float) height);
}

static void renderMesh(mesh m, matrix transform, GLuint texture)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glMultMatrixf(&transform);
    glBegin(GL_QUADS);
    for (int i = 0; i < m.vcount; i++)
    {
        glNormal3fv(&(m.vertices[i].norm));
        glTexCoord2fv(&(m.vertices[i].texcoord));
        glVertex3fv(&(m.vertices[i].pos));
    }
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

static mesh createCubeMesh(color col)
{
    vertex* vertices = (vertex *)calloc(24, sizeof(vertex));

    vector3 norm = { 0, 0, 1 };

    vertices[0] = (vertex){ { -1, -1,  1 }, norm, col, { 0, 0 } };
    vertices[1] = (vertex){ {  1, -1,  1 }, norm, col, { 1, 0 } };
    vertices[2] = (vertex){ {  1,  1,  1 }, norm, col, { 1, 1 } };
    vertices[3] = (vertex){ { -1,  1,  1 }, norm, col, { 0, 1 } };
    for (int i = 1; i < 6; i++)
    {
        matrix rotationMatrix;
        if (i <= 3) rotationMatrix = matrixRotateY(deg2rad(90.0f * i));
        if (i == 4) rotationMatrix = matrixRotateX(deg2rad(90.0f));
        if (i == 5) rotationMatrix = matrixRotateX(deg2rad(-90.0f));
        vector3 normr = matrixVector3Multiply(rotationMatrix, norm);
        vertices[i * 4 + 0] = (vertex){ matrixVector3Multiply(rotationMatrix, vertices[0].pos), normr, col, { 0, 0 } };
        vertices[i * 4 + 1] = (vertex){ matrixVector3Multiply(rotationMatrix, vertices[1].pos), normr, col, { 1, 0 } };
        vertices[i * 4 + 2] = (vertex){ matrixVector3Multiply(rotationMatrix, vertices[2].pos), normr, col, { 1, 1 } };
        vertices[i * 4 + 3] = (vertex){ matrixVector3Multiply(rotationMatrix, vertices[3].pos), normr, col, { 0, 1 } };
    }
    return (mesh) {
        vcount: 24,
        vertices: vertices
    };
}

static mesh createSphereMesh(color col)
{
    const int segments = 64;
    const int rings = segments / 2;
    
    int vcount = segments * rings * 4;
    vertex* vertices = (vertex *)calloc(vcount, sizeof(vertex));

    vector3 **vectors = (vector3 **)calloc(segments+1, sizeof(vector3 *));
    for (int i = 0; i <= segments; i++)
    {
        vectors[i] = (vector3 *)calloc(rings+1, sizeof(vector3));
    }
    
    for (int y = 0; y <= rings; y++)
    {
        float deg = 180.0f / rings * (y - rings * 0.5f);
        matrix rotationMatrixX = matrixRotateX(deg2rad(deg));
        vector3 startVector = matrixVector3Multiply(rotationMatrixX, (vector3){0,0,1});
        vectors[0][y] = startVector;
        vectors[segments][y] = startVector;
        for (int x = 1; x < segments; x++)
        {
            float deg2 = 360.0f / (float)segments * (float)x;
            matrix rotationMatrixY = matrixRotateY(deg2rad(deg2));
            vectors[x][y] = matrixVector3Multiply(rotationMatrixY, vectors[0][y]);
        }
    }
    int i = 0;
    for (int y = 0; y < rings; y++)
    {
        for (int x = 0; x < segments; x++)
        {
            float tw = 1.0f / segments;
            float th = 1.0f / rings;
            float ty = rings - y;
            vertices[i++] = (vertex){ vectors[ x ][y+1], vectors[ x ][y+1], col, {   x   * tw, (ty-1) * th } };
            vertices[i++] = (vertex){ vectors[x+1][y+1], vectors[x+1][y+1], col, { (x+1) * tw, (ty-1) * th } };
            vertices[i++] = (vertex){ vectors[x+1][ y ], vectors[x+1][ y ], col, { (x+1) * tw, ( ty ) * th } };
            vertices[i++] = (vertex){ vectors[ x ][ y ], vectors[ x ][ y ], col, {   x   * tw, ( ty ) * th } };
        }
    }
    for (int i = 0; i <= segments; i++)
    {
        free(vectors[i]);
    }
    free(vectors);
    return (mesh) {
        vcount: vcount,
        vertices: vertices
    };
}