#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <cstdlib>

struct Vertex {
    float x, y, z;    //Chaque sommet de la surface est défini par trois coordonnées x,y,z.
};
// Contrôle de la caméra
float orbitAngleX = 20.0f;    
float orbitAngleY = 30.0f;    
float lastMouseX, lastMouseY;
int mouseButton = -1;
bool isDragging = false;

float camDistance = 6.0f;
float camXOffset = 0.0f;
float camYOffset = 0.0f;

std::vector<std::vector<Vertex>> surfaceMesh;

const int rows = 100;
const int cols = 100;
const float u_min = -2.0f;
const float u_max = 2.0f;
const float v_min = -2.0f;
const float v_max = 2.0f;

/// @brief 
void generateEnneperSurface() {
    surfaceMesh.clear();

    for (int i = 0; i <= rows; ++i) {
        std::vector<Vertex> row;
        float u = u_min + (u_max - u_min) * i / (float) rows;
        for (int j = 0; j <= cols; ++j) {
            float v = v_min + (v_max - v_min) * j / (float) cols;
/*
 Enneper parametric equations:
    x = u - (u³)/3 + u*v²
    y = v - (v³)/3 + v*u²
    z = u² - v²
*/
            float u2 = u * u;
            float v2 = v * v;
            float u3 = u2 * u;
            float v3 = v2 * v;

            Vertex vert;
            vert.x = u - (u3 / 3.0f) + u * v2;
            vert.y = v - (v3 / 3.0f) + v * u2;
            vert.z = u2 - v2;

            row.push_back(vert);
        }
        surfaceMesh.push_back(row);
    }
}
//  Calcul des normales
void computeNormal(const Vertex& p1, const Vertex& p2, const Vertex& p3, float& nx, float& ny, float& nz) {
    float ux = p2.x - p1.x;
    float uy = p2.y - p1.y;
    float uz = p2.z - p1.z;

    float vx = p3.x - p1.x;
    float vy = p3.y - p1.y;
    float vz = p3.z - p1.z;

    nx = uy * vz - uz * vy;
    ny = uz * vx - ux * vz;
    nz = ux * vy - uy * vx;

    float length = sqrt(nx * nx + ny * ny + nz * nz);
    if (length > 0.00001f) {
        nx /= length;
        ny /= length;
        nz /= length;
    } else {
        nx = ny = nz = 0.0f;
    }
}
// Affichage de la surface De Enneper
void drawEnneperSurface() {
    glColor3f(0.4f, 0.75f, 1.0f);

    for (int i = 0; i < rows; ++i) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= cols; ++j) {
            Vertex& v1 = surfaceMesh[i][j];
            Vertex& v2 = surfaceMesh[i + 1][j];

            // Compute normales pour v1
            float nx1, ny1, nz1;
            if (j < cols && i < rows - 1) {
                computeNormal(surfaceMesh[i][j], surfaceMesh[i + 1][j], surfaceMesh[i][j + 1], nx1, ny1, nz1);
            } else {
                nx1 = ny1 = nz1 = 0.0f;
            }

            // Compute normales pour v2
            float nx2, ny2, nz2;
            if (j < cols && i < rows - 1) {
                computeNormal(surfaceMesh[i + 1][j], surfaceMesh[i + 1][j + 1], surfaceMesh[i][j + 1], nx2, ny2, nz2);
            } else {
                nx2 = ny2 = nz2 = 0.0f;
            }

            glNormal3f(nx1, ny1, nz1);
            glVertex3f(v1.x, v1.y, v1.z);

            glNormal3f(nx2, ny2, nz2);
            glVertex3f(v2.x, v2.y, v2.z);
        }
        glEnd();
    }
}

void applyCameraTransform() {
    // Translate la camera pan offsets
    glTranslatef(-camXOffset, -camYOffset, -camDistance);

    //  rotation orbite autours de l'origin
    glRotatef(orbitAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(orbitAngleY, 0.0f, 1.0f, 0.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    applyCameraTransform();

    drawEnneperSurface();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = (float) w / (float) h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        mouseButton = button;
        lastMouseX = (float)x;
        lastMouseY = (float)y;
        isDragging = true;
    } else if (state == GLUT_UP) {
        isDragging = false;
        mouseButton = -1;
    }

    
    if (button == 3) {  // scroll up
        camDistance -= 0.3f;
        if (camDistance < 1.0f) camDistance = 1.0f;
        glutPostRedisplay();
    } else if (button == 4) {  // scroll down
        camDistance += 0.3f;
        glutPostRedisplay();
    }
}

void motion(int x, int y) {
    if (!isDragging) return;

    float dx = (float)(x - lastMouseX);
    float dy = (float)(y - lastMouseY);

    if (mouseButton == GLUT_LEFT_BUTTON) {
        // controls orbite
        orbitAngleY += dx * 0.5f;
        orbitAngleX += dy * 0.5f;

        // Clamp vertical orbit angle pour ne pas flip
        if (orbitAngleX < -90.0f) orbitAngleX = -90.0f;
        if (orbitAngleX > 90.0f) orbitAngleX = 90.0f;

    } else if (mouseButton == GLUT_MIDDLE_BUTTON) {
        float panScale = camDistance * 0.015f;
        camXOffset -= dx * panScale;
        camYOffset += dy * panScale;

    }

    lastMouseX = (float)x;
    lastMouseY = (float)y;

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    float moveStep = camDistance * 0.05f;

    switch(key) {
        case 27:  // ESC
            exit(0);
            break;
        case 's':
        case 'S':
            camYOffset -= moveStep;
            break;
        case 'w':
        case 'W':
            camYOffset += moveStep;
            break;
        case 'a':
        case 'A':
            camXOffset -= moveStep;
            break;
        case 'd':
        case 'D':
            camXOffset += moveStep;
            break;
        case 'q':
        case 'Q':
            camDistance -= 0.3f;
            if (camDistance < 1.0f) camDistance = 1.0f;
            break;
        case 'e':
        case 'E':
            camDistance += 0.3f;
            break;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    float moveStep = camDistance * 0.05f;
    switch(key) {
        case GLUT_KEY_UP:
            camYOffset -= moveStep;
            break;
        case GLUT_KEY_DOWN:
            camYOffset += moveStep;
            break;
        case GLUT_KEY_LEFT:
            camXOffset -= moveStep;
            break;
        case GLUT_KEY_RIGHT:
            camXOffset += moveStep;
            break;
    }
    glutPostRedisplay();
}

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_pos[] = {6.0f, 6.0f, 6.0f, 1.0f};
    GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, light_specular);
    glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 64);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Modelisation surface Enneper surface minimale - Realisation Enzo Hakim ");

    glEnable(GL_DEPTH_TEST);
    initLighting();  //Lighting shaders pour la figure (Enlever car mon laptop est de la chie )

    generateEnneperSurface();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}
