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
    /*Point operator*(Point b)
    {
        return Point(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
    }*/

};

Point center(0,0,0);

Point camera(5, 5, 5);

Point forward_vector;

Point right_vector;

Point world_up_vector(0,0,1);

Point camera_up_vector;


Point ball_center(0,0,0);
Point ball_forward(1,0,0);
Point ball_upward(0,0,1);
Point ball_left(0,1,0);

Point ball_rotation;

float radius = 0.5;

float rotation_angle = 0;
double angleRotationZ = 0;

bool isSimulation = false;

float arc_distance = 1;

//float movement_Factor = 0.1;

float ball_rotation_angle = arc_distance/radius;

float ball_rotation_angle_degree = 0;

float movement_Factor = (ball_rotation_angle / (2 *acos(-1))) * arc_distance;


float simulationTime = 100;
float collisionTime = 1e9;

float forward_vector_angle = 0;

void drawCone(double radius,double height,int segments)
{
    int i;
    double shade;
    struct Point points[100];
    //generate points
    for(i=0; i<=segments; i++)
    {
        points[i].x=radius*cos(((double)i/(double)segments)*2*pi);
        points[i].y=radius*sin(((double)i/(double)segments)*2*pi);
    }
    //draw triangles using generated points
    for(i=0; i<segments; i++)
    {
        //create shading effect
        if(i<segments/2)shade=2*(double)i/(double)segments;
        else shade=2*(1.0-(double)i/(double)segments);
        //glColor3f(shade,shade,shade);

        glBegin(GL_TRIANGLES);
        {
            glVertex3f(0,0,height);
            glVertex3f(points[i].x,points[i].y,0);
            glVertex3f(points[i+1].x,points[i+1].y,0);
        }
        glEnd();
    }
}

void drawCylinder()
{
    float radius = 0.05;
    float height = 1;

    double offset = 2 * acos(-1);

    int segments = 100;

    struct Point cylinderPoints[segments+1];

    for (int i = 0; i <= segments; i++)
    {
        double theta = -offset/2 +  i * offset / segments;
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

    ball_rotation = cross_product(ball_forward,ball_upward);

    ball_rotation = normalized_point(ball_rotation);

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

Point gridPoints[101][101];
void generateGridPoints()
{

    for(int i=0,row = -50; row<=50; row++,i++)
    {
        for(int j=0,col = -50; col <= 50; col++,j++)
        {
            Point point(col,row,0);
            gridPoints[i][j] = point;
        }
    }
}

void drawBoundary()
{
    glBegin(GL_QUADS);
    glVertex3f(5,5,1);
    glVertex3f(5,5,0);
    glVertex3f(-5,5,0);
    glVertex3f(-5,5,1);


    glVertex3f(-5,-5,1);
    glVertex3f(-5,-5,0);
    glVertex3f(-5,5,0);
    glVertex3f(-5,5,1);


    glVertex3f(5,-5,1);
    glVertex3f(5,-5,0);
    glVertex3f(-5,-5,0);
    glVertex3f(-5,-5,1);

    glVertex3f(5,-5,1);
    glVertex3f(5,-5,0);
    glVertex3f(5,5,0);
    glVertex3f(5,5,1);



    glEnd();
}
void drawGrid()
{

    generateGridPoints();

    for(int i=0; i<99; i++)
    {
        for(int j=0; j<99; j++)
        {
            glBegin(GL_QUADS);
            if((i+j) % 2 == 0)
            {
                glColor3f(0,0,0);
            }
            else
                glColor3f(1,1,1);



            glVertex3f(gridPoints[i][j].x,gridPoints[i][j].y,gridPoints[i][j].z);
            glVertex3f(gridPoints[i][j+1].x,gridPoints[i][j+1].y,gridPoints[i][j+1].z);
            glVertex3f(gridPoints[i+1][j+1].x,gridPoints[i+1][j+1].y,gridPoints[i+1][j+1].z);
            glVertex3f(gridPoints[i+1][j].x,gridPoints[i+1][j].y,gridPoints[i+1][j].z);
            glEnd();


        }
    }


}
void drawSphere(double radius,int slices,int stacks)
{
    struct Point points[100][100];
    int i,j;
    double h,r;
    //generate points
    for(i=0; i<=stacks; i++)
    {
        h=radius*sin(((double)i/(double)stacks)*(pi/2));
        r=radius*cos(((double)i/(double)stacks)*(pi/2));
        for(j=0; j<=slices; j++)
        {
            points[i][j].x=r*cos(((double)j/(double)slices)*2*pi);
            points[i][j].y=r*sin(((double)j/(double)slices)*2*pi);
            points[i][j].z=h;
        }
    }
    //draw quads using generated points
    for(i=0; i<stacks; i++)
    {
        //glColor3f((double)i/(double)stacks,(double)i/(double)stacks,(double)i/(double)stacks);

        for(j=0; j<slices; j++)
        {

            glBegin(GL_QUADS);
            {
                //upper hemisphere
                if( j % 2 == 0) glColor3f(1,0,0);
                else
                    glColor3f(0,1,0);
                glVertex3f(points[i][j].x,points[i][j].y,points[i][j].z);
                glVertex3f(points[i][j+1].x,points[i][j+1].y,points[i][j+1].z);
                glVertex3f(points[i+1][j+1].x,points[i+1][j+1].y,points[i+1][j+1].z);
                glVertex3f(points[i+1][j].x,points[i+1][j].y,points[i+1][j].z);
                //lower hemisphere
                if( j % 2 == 1) glColor3f(1,0,0);
                else
                    glColor3f(0,1,0);
                glVertex3f(points[i][j].x,points[i][j].y,-points[i][j].z);
                glVertex3f(points[i][j+1].x,points[i][j+1].y,-points[i][j+1].z);
                glVertex3f(points[i+1][j+1].x,points[i+1][j+1].y,-points[i+1][j+1].z);
                glVertex3f(points[i+1][j].x,points[i+1][j].y,-points[i+1][j].z);
            }
            glEnd();
        }
    }
}

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





    drawGrid();

    glColor3f(1,0,0);
    drawBoundary();

    glColor3f(1,0,0);

    //drawBallAxis();

    glPushMatrix();

    glTranslatef(ball_center.x,ball_center.y,ball_center.z);


    glTranslatef(0,0,radius);

    glRotatef(ball_rotation_angle_degree,ball_rotation.x,ball_rotation.y,ball_rotation.z);
    drawSphere(radius,10,10);
    glPopMatrix();

    glPushMatrix();

    glTranslatef(0,0,radius);
    glLineWidth(5);
    glBegin(GL_LINES);
    glColor3f(0,0,1);
    glVertex3f(ball_center.x,ball_center.y,ball_center.z);
    glVertex3f(ball_forward.x + ball_center.x,ball_forward.y + ball_center.y,ball_forward.z + ball_center.z);
    glColor3f(1,0,1);
    glVertex3f(ball_center.x,ball_center.y,ball_center.z);
    glVertex3f(ball_center.x + ball_upward.x,ball_center.y + ball_upward.y,ball_center.z + ball_upward.z);
    glEnd();
    glPopMatrix();

    /*glPushMatrix();


        glTranslatef(ball_forward.x,ball_forward.y,ball_forward.z);

        glTranslatef(0,0,radius);
        glRotatef(90,0,1,0);
        glColor3f(1,0,1);
        glTranslatef(0,0,1);
        drawCylinder();
        glColor3f(1,1,0);

        glTranslatef(0,0,0.5);
        drawCone(0.1,0.5,20);

    glPopMatrix();*/


    glutSwapBuffers(); // Swap the front and back frame buffers (double buffering)
}

/* Handler for window re-size event. Called back when the window first appears and
   whenever the window is re-sized with its new width and height */
void reshape(GLsizei width, GLsizei height)
{
    // GLsizei for non-negative integer
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

Point ball_velocity_direction;

void do_something(int val)
{


    ball_center.x = ball_center.x + movement_Factor * ball_forward.x;
    ball_center.y = ball_center.y + movement_Factor * ball_forward.y;
    ball_center.z = ball_center.z + movement_Factor* ball_forward.z;


    ball_rotation_angle_degree += (ball_rotation_angle * (180 / acos(-1)));

    ball_rotation = cross_product(ball_forward,ball_upward);

    ball_rotation = normalized_point(ball_rotation);

    if(ball_center.x + radius >= 5)
    {

        ball_forward.x = -ball_forward.x;

        ball_center.x = ball_center.x + movement_Factor * ball_forward.x;

    }

    if(ball_center.x - radius  <= -5)
    {
        ball_forward.x = -ball_forward.x;

        ball_center.x = ball_center.x + movement_Factor * ball_forward.x;

    }

    if(ball_center.y + radius >= 5)
    {

        ball_forward.y = -ball_forward.y;

        ball_center.y = ball_center.y + movement_Factor * ball_forward.y;

    }

    if(ball_center.y - radius <= -5)
    {

        ball_forward.y = -ball_forward.y;

        ball_center.y = ball_center.y + movement_Factor * ball_forward.y;

    }

    glutPostRedisplay();

    collisionTime -= simulationTime;

    if(isSimulation)
    {


        if(collisionTime < simulationTime && collisionTime > 0)
        {

            glutTimerFunc(collisionTime,do_something,0);


        }

        else if(collisionTime <= 0)
        {


            collisionTime = 1e9;

            if(ball_forward.x > 0)
            {
                collisionTime = min(collisionTime, abs(5 - radius  - ball_center.x) * simulationTime / abs(movement_Factor * ball_forward.x));

            }
            if(ball_forward.x < 0)
            {
                collisionTime = min(collisionTime, abs(5 - radius  + ball_center.x) * simulationTime / abs(movement_Factor * ball_forward.x));

            }
            if(ball_forward.y > 0)
            {
                collisionTime = min(collisionTime, abs(5 - radius  - ball_center.y) * simulationTime / abs(movement_Factor * ball_forward.y));
            }
            if(ball_forward.y < 0)
            {
                collisionTime = min(collisionTime, abs(5 - radius  + ball_center.y) * simulationTime / abs(movement_Factor * ball_forward.y));
            }


            if(collisionTime < simulationTime  && collisionTime > 0)
            {
                glutTimerFunc(collisionTime,do_something,collisionTime);

            }
            else
            {
                glutTimerFunc(simulationTime,do_something,simulationTime);
            }


        }

        else
        {
            glutTimerFunc(simulationTime,do_something,simulationTime);
        }









    }
}
void keyboardListener(unsigned char key, int x, int y)
{
    double v = 0.1;
    double rate = 0.1;
    // Point oldEye = eye;

    double s;

    float theta = 10 * acos(-1) / 180.0;



    double r;

    float th;


    // float v = 0.1;
    switch (key)
    {
    case '1':


        right_vector = right_vector * cos(theta) + forward_vector * sin(theta);

        right_vector = normalized_point(right_vector);

        forward_vector = cross_product(camera_up_vector,right_vector);

        forward_vector = normalized_point(forward_vector);

        center = forward_vector * getLength(center - camera);

        break;

    case '2':

        forward_vector = forward_vector * cos(theta) + right_vector * sin(theta);

        forward_vector = normalized_point(forward_vector);

        right_vector = cross_product(forward_vector, camera_up_vector);

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



        break;

    case '6':

        right_vector = right_vector * cos(theta) + camera_up_vector * sin(theta);

        right_vector = normalized_point(right_vector);

        camera_up_vector = cross_product(right_vector,forward_vector);

        camera_up_vector = normalized_point(camera_up_vector);

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

    case 'k':

        ball_center.x = ball_center.x - movement_Factor * ball_forward.x;
        ball_center.y = ball_center.y - movement_Factor * ball_forward.y;
        ball_center.z = ball_center.z - movement_Factor* ball_forward.z;



        if(ball_center.x + radius >= 5)
        {

            ball_forward.x = -ball_forward.x;



            ball_center.x = ball_center.x - movement_Factor * ball_forward.x;

        }

        if(ball_center.x - radius  <= -5)
        {



            ball_forward.x = -ball_forward.x;



            ball_center.x = ball_center.x - movement_Factor * ball_forward.x;

        }
        if(ball_center.y + radius >= 5)
        {

            ball_forward.y = -ball_forward.y;

            ball_center.y = ball_center.y - movement_Factor * ball_forward.y;
        }
        if(ball_center.y - radius <= -5)
        {

            ball_forward.y = -ball_forward.y;

            ball_center.y = ball_center.y - movement_Factor * ball_forward.y;

        }


        ball_rotation_angle_degree -= (ball_rotation_angle * (180 / acos(-1)));

        ball_rotation = cross_product(ball_forward,ball_upward);

        ball_rotation = normalized_point(ball_rotation);

        break;

    case 'i':

        ball_center.x = ball_center.x + movement_Factor * ball_forward.x;
        ball_center.y = ball_center.y + movement_Factor * ball_forward.y;
        ball_center.z = ball_center.z + movement_Factor* ball_forward.z;


        if(ball_center.x + radius >= 5)
        {

            ball_forward.x = -ball_forward.x;

            ball_center.x = ball_center.x + movement_Factor * ball_forward.x;

        }

        if(ball_center.x - radius  <= -5)
        {



            ball_forward.x = -ball_forward.x;

            ball_center.x = ball_center.x + movement_Factor * ball_forward.x;

        }

        if(ball_center.y + radius >= 5)
        {

            ball_forward.y = -ball_forward.y;

            ball_center.y = ball_center.y + movement_Factor * ball_forward.y;

        }

        if(ball_center.y - radius <= -5)
        {

            ball_forward.y = -ball_forward.y;

            ball_center.y = ball_center.y + movement_Factor * ball_forward.y;

        }


        ball_rotation_angle_degree += (ball_rotation_angle * (180 / acos(-1)));

        ball_rotation = cross_product(ball_forward,ball_upward);

        ball_rotation = normalized_point(ball_rotation);



        break;
    case 'j':

        forward_vector_angle += 10;

        ball_forward = ball_forward * cos(theta) + ball_left*sin(theta);
        ball_forward = normalized_point(ball_forward);
        ball_left = cross_product(ball_upward,ball_forward);
        ball_left = normalized_point(ball_left);

        collisionTime = 1e9;

        if(ball_forward.x > 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  - ball_center.x) * simulationTime / abs(movement_Factor * ball_forward.x));

        }
        if(ball_forward.x < 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  + ball_center.x) * simulationTime / abs(movement_Factor * ball_forward.x));

        }
        if(ball_forward.y > 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  - ball_center.y) * simulationTime / abs(movement_Factor * ball_forward.y));
        }
        if(ball_forward.y < 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  + ball_center.y) * simulationTime / abs(movement_Factor * ball_forward.y));
        }



        break;

    case 'l':

        collisionTime = 1e9;

        ball_forward = ball_forward * cos(theta) + ball_left* sin(-theta);

        ball_forward = normalized_point(ball_forward);

        ball_left = cross_product(ball_upward,ball_forward);

        ball_left = normalized_point(ball_left);


        if(ball_forward.x > 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  - ball_center.x) * simulationTime / abs(movement_Factor * ball_forward.x));

        }
        if(ball_forward.x < 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  + ball_center.x) * simulationTime / abs(movement_Factor * ball_forward.x));

        }
        if(ball_forward.y > 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  - ball_center.y) * simulationTime / abs(movement_Factor * ball_forward.y));
        }
        if(ball_forward.y < 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  + ball_center.y) * simulationTime / abs(movement_Factor * ball_forward.y));
        }

        break;

    case ' ':


        // collisionTime = min((5.0 - radius - ball_center.x) * simulationTime / abs(movement_Factor * ball_velocity_direction.x) , (5.0 - radius - abs(ball_center.y)) * simulationTime / abs(movement_Factor * ball_velocity_direction.y));

        if(ball_forward.x > 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  - ball_center.x) * simulationTime / abs(movement_Factor * ball_forward.x));

        }
        if(ball_forward.x < 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  + ball_center.x) * simulationTime / abs(movement_Factor * ball_forward.x));

        }
        if(ball_forward.y > 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  - ball_center.y) * simulationTime / abs(movement_Factor * ball_forward.y));
        }
        if(ball_forward.y < 0)
        {
            collisionTime = min(collisionTime, abs(5 - radius  + ball_center.y) * simulationTime / abs(movement_Factor * ball_forward.y));
        }

        if(!isSimulation)
        {
            isSimulation = true;
            glutTimerFunc(simulationTime,do_something,0);
        }
        else
            isSimulation = false;

        break;

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

