#ifdef __linux__
#include <GL/glut.h>
#elif WIN32
#include <windows.h>
#include <GL/glut.h>
#endif

#include <cmath>
#include <iostream>
#include<bits/stdc++.h>
#define pi (2 * acos(0.0))
using namespace std;
/* Global variables */
char title[] = "3D Shapes";
double increaseFactor = 1.0;
/* Initialize OpenGL Graphics */

// generate vertices for +X face only by intersecting 2 circular planes
// (longitudinal and latitudinal) at the given longitude/latitude angles

float radius = 1.0 / sqrt(3);
vector<float> buildUnitPositiveX(int subdivision)
{
    const float DEG2RAD = acos(-1) / 180.0f;

    std::vector<float> vertices;
    float n1[3];        // normal of longitudinal plane rotating along Y-axis
    float n2[3];        // normal of latitudinal plane rotating along Z-axis
    float v[3];         // direction vector intersecting 2 planes, n1 x n2
    float a1;           // longitudinal angle along Y-axis
    float a2;           // latitudinal angle along Z-axis

    // compute the number of vertices per row, 2^n + 1
    int pointsPerRow = (int)pow(2, subdivision) + 1;

    // rotate latitudinal plane from 45 to -45 degrees along Z-axis (top-to-bottom)
    for(unsigned int i = 0; i < pointsPerRow; ++i)
    {
        // normal for latitudinal plane
        // if latitude angle is 0, then normal vector of latitude plane is n2=(0,1,0)
        // therefore, it is rotating (0,1,0) vector by latitude angle a2
        a2 = DEG2RAD * (45.0f - 90.0f * i / (pointsPerRow - 1));
        n2[0] = -sin(a2);
        n2[1] = cos(a2);
        n2[2] = 0;

        // rotate longitudinal plane from -45 to 45 along Y-axis (left-to-right)
        for(unsigned int j = 0; j < pointsPerRow; ++j)
        {
            // normal for longitudinal plane
            // if longitude angle is 0, then normal vector of longitude is n1=(0,0,-1)
            // therefore, it is rotating (0,0,-1) vector by longitude angle a1
            a1 = DEG2RAD * (-45.0f + 90.0f * j / (pointsPerRow - 1));
            n1[0] = -sin(a1);
            n1[1] = 0;
            n1[2] = -cos(a1);

            // find direction vector of intersected line, n1 x n2
            v[0] = n1[1] * n2[2] - n1[2] * n2[1];
            v[1] = n1[2] * n2[0] - n1[0] * n2[2];
            v[2] = n1[0] * n2[1] - n1[1] * n2[0];

            // normalize direction vector
            float scale = 1 / sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

            v[0] *= (scale * radius);
            v[1] *= (scale * radius);
            v[2] *= (scale * radius);

            // add a vertex into array
            vertices.push_back(v[0]);
            vertices.push_back(v[1]);
            vertices.push_back(v[2]);
        }
    }

    return vertices;
}

class Point
{

public:
    float x, y, z;

    Point(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Point()
    {
        this->x = 0;
        this->y = 0;
        this->z = 0;
    }

    Point operator+(Point b)
    {
        return Point(x + b.x, y + b.y, z + b.z);
    }
    Point operator-(Point b)
    {
        return Point(x - b.x, y - b.y, z - b.z);
    }
    Point operator-(double b)
    {
        return Point(x - b, y - b, z - b);
    }
    Point operator*(double b)
    {
        return Point(x * b, y * b, z * b);
    }
    Point operator/(double b)
    {
        return Point(x / b, y / b, z / b);
    }

};

Point normalized_point(Point p)
{
    float length = p.x * p.x + p.y * p.y + p.z * p.z;

    length = sqrt(length);

    Point result(p.x,p.y,p.z);

    result.x = p.x / length;
    result.y = p.y / length;
    result.z = p.z / length;

    return result;

}

Point center(0,0,0);

Point camera(3, 3, 3);

Point forward_vector;

Point right_vector;

Point world_up_vector(0,1,0);

Point camera_up_vector;


Point octahedron_point_3(0,0,1);
Point octahedron_point_2(0,1,0);
Point octahedron_point_1(1,0,0);


double angleOfRotation = 0;
int subdivision = 4;
int spherePointDimension = pow(2,subdivision) + 1;

Point spherePoints[20][20];

void drawCubeSphere()
{
    for(int i=0;i<spherePointDimension - 1;i++)
    {
        for(int j=0;j<spherePointDimension - 1;j++)
        {
            glBegin(GL_QUADS);

            glVertex3f(spherePoints[i][j].x,spherePoints[i][j].y,spherePoints[i][j].z);
            glVertex3f(spherePoints[i][j + 1].x,spherePoints[i][j + 1].y,spherePoints[i][j + 1].z);
            glVertex3f(spherePoints[i + 1][j + 1].x,spherePoints[i + 1][j + 1].y,spherePoints[i + 1][j + 1].z);
            glVertex3f(spherePoints[i + 1][j].x,spherePoints[i + 1][j].y,spherePoints[i + 1][j].z);

            glEnd();
        }
    }
}

vector<float>getUnitCircleVertices()
{

    int sectorCount = 20;

    const float PI = 3.1415926f;
    double theta_range = 70.5287794*PI/180.0;
    float sectorStep = PI / sectorCount;
    float sectorAngle;  // radian

    std::vector<float> unitCircleVertices;
    for(int i = 0; i <= sectorCount; ++i)
    {
        sectorAngle = -theta_range/2 + i * theta_range/sectorCount;
        unitCircleVertices.push_back(cos(sectorAngle)); // x
        unitCircleVertices.push_back(sin(sectorAngle)); // y
        unitCircleVertices.push_back(0);                // z
    }
    return unitCircleVertices;
}

vector<float>  buildVerticesSmooth()
{

    vector<float>vertices;
    vector<float>normals;
    vector<float>texCoords;
    // get unit circle vectors on XY-plane
    vector<float> unitVertices = getUnitCircleVertices();

    // put side vertices to arrays

    float height = (increaseFactor)*sqrt(2.26);
    int sectorCount = 20;
    float radius = increaseFactor/sqrt(3); //* sqrt(2);
    for(int i = 0; i <= 20; ++i)
    {
        float h = -height / 2.0f + i * height/20.0;


        for(int j = 0, k = 0; j <= sectorCount; ++j, k += 3)
        {
            float ux = unitVertices[k];
            float uy = unitVertices[k+1];
            float uz = unitVertices[k+2];
            // position vector
            vertices.push_back(ux * radius);             // vx
            vertices.push_back(uy * radius);             // vy
            vertices.push_back(h);                       // vz

        }
    }
    return vertices;

}

void drawCylinder()
{
        float radius = sqrt(1.0 / 3.0) * (1 - increaseFactor);
        float height = sqrt(2) * increaseFactor;

        double range = 70.5287794*M_PI/180.0;

        int segments = 100;

        struct Point cylinderPoints[segments+1];

        for (int i = 0; i <= segments; i++) {
            double theta = -range/2 +  i * range / segments;
            cylinderPoints[i].x = radius * cos(theta);
            cylinderPoints[i].y = radius * sin(theta);
        }

        glBegin(GL_QUADS);
            for (int i = 0; i < segments; i++)
            {
                glVertex3f(cylinderPoints[i].x, cylinderPoints[i].y, height/2);
                glVertex3f(cylinderPoints[i].x, cylinderPoints[i].y, -height/2);
                glVertex3f(cylinderPoints[i+1].x, cylinderPoints[i+1].y, -height/2);
                glVertex3f(cylinderPoints[i+1].x, cylinderPoints[i+1].y, height/2);
            }
        glEnd();
}
Point cross_product(Point p1, Point p2)
{
    return Point(p1.y * p2.z - p1.z * p2.y, p1.z * p2.x - p1.x * p2.z, p1.x * p2.y - p1.y * p2.x);

}
void calculateNecessaryVectors()
{
    forward_vector.x = center.x - camera.x;
    forward_vector.y = center.y - camera.y;
    forward_vector.z = center.z - camera.z;

    forward_vector = normalized_point(forward_vector);

    right_vector = cross_product(forward_vector, world_up_vector);
    right_vector = normalized_point(right_vector);

    camera_up_vector = cross_product(right_vector, forward_vector);
    camera_up_vector = normalized_point(camera_up_vector);

}
void initGL()
{
    calculateNecessaryVectors();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);              // Set background color to black and opaque
    glClearDepth(1.0f);                                // Set background depth to farthest
    glEnable(GL_DEPTH_TEST);                           // Enable depth testing for z-culling
    glDepthFunc(GL_LEQUAL);                            // Set the type of depth-test
    glShadeModel(GL_SMOOTH);                           // Enable smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nice perspective corrections
}

/* Handler for window-repaint event. Called back when the window first appears and
   whenever the window needs to be re-painted. */
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW);                         // To operate on model-view matrix

    // Render a color-cube consisting of 6 quads with different colors
    glLoadIdentity();
    // Reset the model-view matrix
    // glTranslatef(1.5f, 0.0f, -7.0f);  // Move right and into the screen


     gluLookAt(camera.x, camera.y, camera.z,
              center.x,center.y,center.z,
              camera_up_vector.x,camera_up_vector.y,camera_up_vector.z);

    // Begin drawing the color cube with 6 quads
    // Top face (y = 1.0f)
    // Define vertices in counter-clockwise (CCW) order with normal pointing out

    octahedron_point_1.x =  (1.0/3) + increaseFactor * (2.0/3);
    octahedron_point_1.y =  (1.0/3) * (1 - increaseFactor);
    octahedron_point_1.z =  (1.0/3) * (1 - increaseFactor);


    octahedron_point_2.y = (1.0/3) + increaseFactor * (2.0/3);
    octahedron_point_2.x = (1.0/3) * (1 - increaseFactor);
    octahedron_point_2.z = (1.0/3) * (1 - increaseFactor);

    octahedron_point_3.z = (1.0/3) + increaseFactor * (2.0/3);
    octahedron_point_3.x = (1.0/3) * (1 - increaseFactor);
    octahedron_point_3.y = (1.0/3) * (1 - increaseFactor);


    glRotatef(angleOfRotation, world_up_vector.x, world_up_vector.y, world_up_vector.z);



    for (int i = 0; i < 4; i++)
    {
        glPushMatrix();

        glRotatef(i * 90, 0, 1, 0);

        glBegin(GL_TRIANGLES);

        if(i%2)
            glColor3f(1, 0, 1);
        else
            glColor3f(0, 1, 1);

        glVertex3f(octahedron_point_1.x, octahedron_point_1.y, octahedron_point_1.z);

        glVertex3f(octahedron_point_2.x, octahedron_point_2.y, octahedron_point_2.z);

        glVertex3f(octahedron_point_3.x, octahedron_point_3.y, octahedron_point_3.z);

        glEnd(); // End of drawing color-cube

        glPopMatrix();
    }

    glScalef(1, -1, 1);

    for (int i = 0; i < 4; i++)
    {
        glPushMatrix();

        glRotatef(i * 90, 0, 1, 0);

        glBegin(GL_TRIANGLES);

        if(i%2 == 0)
            glColor3f(1, 0, 1);
        else
            glColor3f(0, 1, 1);

        glVertex3f(octahedron_point_1.x, octahedron_point_1.y, octahedron_point_1.z);

        glVertex3f(octahedron_point_2.x, octahedron_point_2.y, octahedron_point_2.z);

        glVertex3f(octahedron_point_3.x, octahedron_point_3.y, octahedron_point_3.z);

        glEnd(); // End of drawing color-cube

        glPopMatrix();
    }

    vector<float> allPoints = buildUnitPositiveX(4);

    int pointsPerRow = (int)pow(2, 4) + 1;

    int divider = pointsPerRow * 3;



    int ind = 0;
    for(int i=0;i<spherePointDimension;i++)
    {
        for(int j=0;j<spherePointDimension;j++)
        {
            spherePoints[i][j].x = allPoints[ind++];
            spherePoints[i][j].y = allPoints[ind++];
            spherePoints[i][j].z = allPoints[ind++];
        }
    }


    for(int i=0;i<4;i++)
    {
        glPushMatrix();
        if(i%2)
            glColor3f(0,0,1);
        else
            glColor3f(0,1,0);
        glRotatef(i*90,0,1,0);
        glTranslatef(increaseFactor,0,0);
        glScalef(1-increaseFactor,1-increaseFactor,1-increaseFactor);
        drawCubeSphere();
        glPopMatrix();
    }

    glPushMatrix();
    glColor3f(1,0,0);
    glRotatef(1*90,0,0,1);
    glTranslatef(increaseFactor,0,0);
    glScalef(1-increaseFactor,1-increaseFactor,1-increaseFactor);
    drawCubeSphere();
    glPopMatrix();

    glPushMatrix();
    glColor3f(1,0,0);
    glRotatef(3*90,0,0,1);
    glTranslatef(increaseFactor,0,0);
    glScalef(1-increaseFactor,1-increaseFactor,1-increaseFactor);
    drawCubeSphere();
    glPopMatrix();


    for(int i=0;i<=4;i++)
    {

        glPushMatrix();

        glColor3f(1,1,0);
        glRotatef(90 * i,0,1,0);
        glRotatef(45,0,0,-1);

        glRotatef(90,1,0,0);
        glTranslatef(increaseFactor/sqrt(2),0,0);
        drawCylinder();



        glPopMatrix();
    }
    for(int i=0;i<=4;i++)
    {

        glPushMatrix();
        glRotatef(90 * i,0,1,0);
        glRotatef(45,0,1,0);
        glTranslatef(increaseFactor/sqrt(2),0,0);
        drawCylinder();

        glPopMatrix();
    }

    for(int i=0;i<=4;i++)
    {

        glPushMatrix();
        glRotatef(90 * i,0,1,0);
        glRotatef(45,0,0,1);

        glRotatef(90,1,0,0);
        glTranslatef(increaseFactor/sqrt(2),0,0);

        drawCylinder();

        glPopMatrix();
    }

    glutSwapBuffers(); // Swap the front and back frame buffers (double buffering)
}

/* Handler for window re-size event. Called back when the window first appears and
   whenever the window is re-sized with its new width and height */
void reshape(GLsizei width, GLsizei height)
{ // GLsizei for non-negative integer
    // Compute aspect ratio of the new window
    if (height == 0)
        height = 1; // To prevent divide by 0
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);

    // Set the aspect ratio of the clipping volume to match the viewport
    glMatrixMode(GL_PROJECTION); // To operate on the Projection matrix
    glLoadIdentity();            // Reset
    // Enable perspective projection with fovy, aspect, zNear and zFar
    gluPerspective(45.0f, aspect, 0.1f, 100.0f);
}

float getLength(Point p)
{
    return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

void keyboardListener(unsigned char key, int x, int y)
{
    double v = 0.1;
    double rate = 0.1;
    // Point oldEye = eye;
    float theta = 1 * acos(-1)/180.0;
    double s;
    // float v = 0.1;
    switch (key)
    {
        case 'a':

            angleOfRotation -= 10;

            break;
        case 'd':

            angleOfRotation += 10;

            break;
    case 'w':

        camera.x += rate * camera_up_vector.x;
        camera.y += rate * camera_up_vector.y;
        camera.z += rate * camera_up_vector.z;

        forward_vector.x = center.x - camera.x;
        forward_vector.y = center.x - camera.y;
        forward_vector.z = center.x - camera.z;

        forward_vector = normalized_point(forward_vector);

        right_vector = cross_product(forward_vector,camera_up_vector);

        right_vector = normalized_point(right_vector);

        break;
    case 's':

        camera.x -= rate * camera_up_vector.x;
        camera.y -= rate * camera_up_vector.y;
        camera.z -= rate * camera_up_vector.z;

        forward_vector.x = center.x - camera.x;
        forward_vector.y = center.x - camera.y;
        forward_vector.z = center.x - camera.z;

        forward_vector = normalized_point(forward_vector);

        right_vector = cross_product(forward_vector,camera_up_vector);

        right_vector = normalized_point(right_vector);

        break;

        case '1':


            right_vector = right_vector * cos(theta) + forward_vector * sin(theta);

            forward_vector = cross_product(camera_up_vector,right_vector);

            forward_vector = normalized_point(forward_vector);

            right_vector = normalized_point(right_vector);

            center = forward_vector * getLength(center - camera);



            break;

        case '2':

            forward_vector = forward_vector * cos(theta) + right_vector * sin(theta);

            forward_vector = normalized_point(forward_vector);

            right_vector = cross_product(forward_vector , camera_up_vector);

            right_vector = normalized_point(right_vector);

            center = forward_vector * getLength(center - camera);

            break;

        case '3':

            forward_vector = forward_vector * cos(theta) + camera_up_vector * sin(theta);

            forward_vector = normalized_point(forward_vector);

            camera_up_vector = cross_product(right_vector, forward_vector);

            camera_up_vector = normalized_point(camera_up_vector);

            center = forward_vector * getLength(center - camera);


            break;

        case '4':

            camera_up_vector = camera_up_vector * cos(theta) + forward_vector * sin(theta);

            camera_up_vector = normalized_point(camera_up_vector);

            forward_vector = cross_product(camera_up_vector, right_vector);

            forward_vector = normalized_point(forward_vector);

            center = forward_vector * getLength(center - camera);

            break;

        case '5':


            camera_up_vector = camera_up_vector * cos(theta) + right_vector * sin(theta);

            camera_up_vector = normalized_point(camera_up_vector);

            right_vector = cross_product(forward_vector,camera_up_vector);

            right_vector = normalized_point(right_vector);

            //center = forward_vector * getLength(center - camera);


            break;

        case '6':

            right_vector = right_vector * cos(theta) + camera_up_vector * sin(theta);

            right_vector = normalized_point(right_vector);

            camera_up_vector = cross_product(right_vector,forward_vector);

            camera_up_vector = normalized_point(camera_up_vector);

            break;

            // Control center (location where the eye is looking at)
            // control centerx
            // Control what is shown

        case ',':

            if(increaseFactor > 0)
                increaseFactor -= 0.05;
            break;
        case '.':
            if(increaseFactor < 1)
                increaseFactor += 0.05;
            break;
            // Control exit
        case 27:     // ESC key
            exit(0); // Exit window
            break;
    }

    // look = look - eye + oldEye;

    glutPostRedisplay(); // Post a paint request to activate display()
}

void specialKeyListener(int key, int x, int y)
{
    double r = 0.08;
    switch (key)
    {
    case GLUT_KEY_UP: // down arrow key
        //pos = pos + l;
        camera.x = camera.x + r * forward_vector.x;
        camera.y = camera.y + r * forward_vector.y;
        camera.z = camera.z + r * forward_vector.z;


        center.x = center.x + r * forward_vector.x;
        center.y = center.y + r * forward_vector.y;
        center.z = center.z + r * forward_vector.z;

        break;
    case GLUT_KEY_DOWN: // up arrow key
       // pos = pos - l;
        camera.x = camera.x - r * forward_vector.x;
        camera.y = camera.y - r * forward_vector.y;
        camera.z = camera.z - r * forward_vector.z;


        center.x = center.x - r * forward_vector.x;
        center.y = center.y - r * forward_vector.y;
        center.z = center.z - r * forward_vector.z;

        break;

    case GLUT_KEY_RIGHT:
        //pos = pos + r;
        camera.x = camera.x + r * right_vector.x;
        camera.y = camera.y + r * right_vector.y;
        camera.z = camera.z + r * right_vector.z;


        center.x = center.x + r * right_vector.x;
        center.y = center.y + r * right_vector.y;
        center.z = center.z + r * right_vector.z;

        break;
    case GLUT_KEY_LEFT:
        //pos = pos - r;
        camera.x = camera.x - r * right_vector.x;
        camera.y = camera.y - r * right_vector.y;
        camera.z = camera.z - r * right_vector.z;


        center.x = center.x - r * right_vector.x;
        center.y = center.y - r * right_vector.y;
        center.z = center.z - r * right_vector.z;
        break;

    case GLUT_KEY_PAGE_UP:
        //pos = pos + u;
        camera.x = camera.x + r * camera_up_vector.x;
        camera.y = camera.y + r * camera_up_vector.y;
        camera.z = camera.z + r * camera_up_vector.z;


        center.x = center.x + r * camera_up_vector.x;
        center.y = center.y + r * camera_up_vector.y;
        center.z = center.z + r * camera_up_vector.z;
        break;
    case GLUT_KEY_PAGE_DOWN:
        camera.x = camera.x - r * camera_up_vector.x;
        camera.y = camera.y - r * camera_up_vector.y;
        camera.z = camera.z - r * camera_up_vector.z;


        center.x = center.x - r * camera_up_vector.x;
        center.y = center.y - r * camera_up_vector.y;
        center.z = center.z - r * camera_up_vector.z;
        break;

    default:
        break;
    }
    glutPostRedisplay();
}

/* Main function: GLUT runs as a console application starting at main() */
int main(int argc, char **argv)
{
    glutInit(&argc, argv);            // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE); // Enable double buffered mode
    glutInitWindowSize(640, 480);     // Set the window's initial width & height
    glutInitWindowPosition(50, 50);   // Position the window's initial top-left corner
    glutCreateWindow(title);          // Create window with the given title
    glutDisplayFunc(display);         // Register callback handler for window re-paint event
    glutReshapeFunc(reshape);         // Register callback handler for window re-size event
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);
    initGL();       // Our own OpenGL initialization
    glutMainLoop(); // Enter the infinite event-processing loop
    return 0;
}
