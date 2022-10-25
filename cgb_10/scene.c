#include <stdlib.h>
#include "scene.h"
#include "camera.h"
#include "texture.h"
#include <math.h>

static const color thmGray = {0.29f, 0.36f, 0.4f, 1.0f};
static const color white = {1.0f, 1.0f, 1.0f, 1.0f};

static const color sunLight = {0.95f, 0.95f, 0.95f, 1.0f};
static const color ambientLight = {0.05f, 0.05f, 0.05f, 1.0f};
static const color noLight = {0.0f, 0.0f, 0.0f, 1.0f};

static mesh earthMesh;
static mesh satelliteMesh;
static mesh cubeMap;

static GLuint earthTexture;
static GLuint earthNightTexture;
static GLuint earthCloudTexture;
static GLuint satelliteTexture;
static GLuint cubeMapTexture;

void loadScene(GLFWwindow* window)
{
    setViewportSize(window);

    glClearColor(0, 0, 0, 0);

    earthMesh = createSphereMesh(white);
    satelliteMesh = createCubeMesh(white);
    cubeMap = createCubeMap(white);

    earthTexture = loadTexture("res/land_ocean_ice_8192.jpg");
    earthNightTexture = loadTexture("res/lights_8192.jpg");
    earthCloudTexture = loadTexture("res/cloud_combined_8192.jpg");
    satelliteTexture = loadTexture("res/thm2k.png");
    cubeMapTexture = loadTexture("res/cubemap8k.jpg");

    glLightfv(GL_LIGHT1, GL_DIFFUSE, &sunLight);
    glLightfv(GL_LIGHT1, GL_AMBIENT, &ambientLight);
    glLightfv(GL_LIGHT1, GL_SPECULAR, &white);

    glLightfv(GL_LIGHT2, GL_DIFFUSE, &noLight);
    glLightfv(GL_LIGHT2, GL_AMBIENT, &sunLight);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &noLight);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    loadCameraViewMatrix(0);
    glDisable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glBlendFunc(GL_ONE, GL_ZERO);
    renderMesh(cubeMap, matrixScale(1), cubeMapTexture);
    glClear(GL_DEPTH_BUFFER_BIT);

    loadCameraViewMatrix(1);

    vector4 lightPosition = {0, 0, 50000, 0};
    glLightfv(GL_LIGHT1, GL_POSITION, &lightPosition);

    glDisable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    renderMesh(earthMesh, calculateEarthRotation(1), earthNightTexture);
    glClear(GL_DEPTH_BUFFER_BIT);

    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
    renderMesh(earthMesh, calculateEarthRotation(0.08), earthCloudTexture);
    glClear(GL_DEPTH_BUFFER_BIT);

    glDisable(GL_LIGHT2);
    glEnable(GL_LIGHT1);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
    renderMesh(earthMesh, calculateEarthRotation(1), 0);
    glClear(GL_DEPTH_BUFFER_BIT);

    glBlendFunc(GL_ONE, GL_ONE);
    renderMesh(earthMesh, calculateEarthRotation(1), earthTexture);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    renderMesh(earthMesh, calculateEarthRotation(0.08), earthCloudTexture);

    glDisable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glBlendFunc(GL_ONE, GL_ZERO);
    renderMesh(satelliteMesh, calculateSatellitePosition(), satelliteTexture);
}

void unloadScene()
{
    free(earthMesh.vertices);
    free(satelliteMesh.vertices);
    free(cubeMap.vertices);
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
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, &(m.vertices[i].color));
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(m.vertices[i].color));
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30.0f);
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

static mesh createCubeMap(color col)
{
    vertex* vertices = (vertex *)calloc(24, sizeof(vertex));

    // +y
    vertices[ 0] = (vertex){ {  1,  1, -1 }, {  0, -1,  0 }, col, { 2/3.f, 1/2.f } };
    vertices[ 1] = (vertex){ {  1,  1,  1 }, {  0, -1,  0 }, col, { 2/3.f, 2/2.f } };
    vertices[ 2] = (vertex){ { -1,  1,  1 }, {  0, -1,  0 }, col, { 1/3.f, 2/2.f } };
    vertices[ 3] = (vertex){ { -1,  1, -1 }, {  0, -1,  0 }, col, { 1/3.f, 1/2.f } };

    // +z
    vertices[ 4] = (vertex){ {  1, -1,  1 }, {  0,  0, -1 }, col, { 2/3.f, 1/2.f } };
    vertices[ 5] = (vertex){ { -1, -1,  1 }, {  0,  0, -1 }, col, { 3/3.f, 1/2.f } };
    vertices[ 6] = (vertex){ { -1,  1,  1 }, {  0,  0, -1 }, col, { 3/3.f, 2/2.f } };
    vertices[ 7] = (vertex){ {  1,  1,  1 }, {  0,  0, -1 }, col, { 2/3.f, 2/2.f } };

    // -x
    vertices[ 8] = (vertex){ { -1, -1,  1 }, {  1,  0,  0 }, col, { 0/3.f, 0/2.f } };
    vertices[ 9] = (vertex){ { -1, -1, -1 }, {  1,  0,  0 }, col, { 1/3.f, 0/2.f } };
    vertices[10] = (vertex){ { -1,  1, -1 }, {  1,  0,  0 }, col, { 1/3.f, 1/2.f } };
    vertices[11] = (vertex){ { -1,  1,  1 }, {  1,  0,  0 }, col, { 0/3.f, 1/2.f } };

    // -y
    vertices[12] = (vertex){ { -1, -1, -1 }, {  0,  1,  0 }, col, { 1/3.f, 1/2.f } };
    vertices[13] = (vertex){ { -1, -1,  1 }, {  0,  1,  0 }, col, { 1/3.f, 0/2.f } };
    vertices[14] = (vertex){ {  1, -1,  1 }, {  0,  1,  0 }, col, { 2/3.f, 0/2.f } };
    vertices[15] = (vertex){ {  1, -1, -1 }, {  0,  1,  0 }, col, { 2/3.f, 1/2.f } };

    // +x
    vertices[16] = (vertex){ {  1, -1, -1 }, { -1,  0,  0 }, col, { 0/3.f, 1/2.f } };
    vertices[17] = (vertex){ {  1, -1,  1 }, { -1,  0,  0 }, col, { 1/3.f, 1/2.f } };
    vertices[18] = (vertex){ {  1,  1,  1 }, { -1,  0,  0 }, col, { 1/3.f, 2/2.f } };
    vertices[19] = (vertex){ {  1,  1, -1 }, { -1,  0,  0 }, col, { 0/3.f, 2/2.f } };

    // -z
    vertices[20] = (vertex){ { -1, -1, -1 }, { 0,  0,  1 }, col, { 2/3.f, 0/2.f } };
    vertices[21] = (vertex){ {  1, -1, -1 }, { 0,  0,  1 }, col, { 3/3.f, 0/2.f } };
    vertices[22] = (vertex){ {  1,  1, -1 }, { 0,  0,  1 }, col, { 3/3.f, 1/2.f } };
    vertices[23] = (vertex){ { -1,  1, -1 }, { 0,  0,  1 }, col, { 2/3.f, 1/2.f } };

    return (mesh){
        vcount: 24,
        vertices: vertices
    };
}

static matrix calculateEarthRotation(float multiplier)
{
    float earthRotation = timePeriod(86400 * multiplier, 0) * 2.f * M_PI;

    float earthEcliptic = cosf(timePeriod(31557600, 864000) * 2.f * M_PI) * deg2rad(-23.4f);
    
    return matrixMultiply(matrixRotateX(earthEcliptic), matrixRotateY(earthRotation));
}

static matrix calculateSatellitePosition()
{
    float scale = 25.0 / 6370.0;

    int orbitTime = 5400;
    float orbitRadius = 6770.0 / 6370.0;

    matrix orbit = matrixRotateY(timePeriod(orbitTime, 0) * 2.f * M_PI);

    matrix translation = matrixMultiply(matrixTranslate(orbitRadius, 0, 0), matrixScale(scale));

    static float tu = 0.1f;
    tu += 0.1f;
    matrix tumbling = matrixMultiply(matrixRotateY(deg2rad(tu)), matrixRotateZ(deg2rad(tu)));

    return matrixMultiply(orbit, matrixMultiply(translation, tumbling));
}