// triangle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <GLFrame.h>
#include <GLTools.h>            // OpenGL toolkit
#include <GLFrustum.h>
#include <StopWatch.h>
#include <math3d.h>
#include <gl/gl.h>
#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif


GLuint shader;
GLuint MVPMatrixLocation;
GLFrustum viewFrustum;
GLFrame cameraFrame;

//camera settings
M3DMatrix44f mCamera;
	
//cameraFrame.GetCameraMatrix(mCamera);

void setOrigin(GLfloat *p);
void setForwardVector(GLfloat *v);
void setUpVector(GLfloat *v);

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.

void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(45,w/h,10,20);
}


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context.
// This is the first opportunity to do any OpenGL related tasks.

void SetupRC() {
    // background
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp",
            2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");
    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \n",
            GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);


	MVPMatrixLocation=glGetUniformLocation(shader,"MVPMatrix");
if(MVPMatrixLocation==-1)
{
	fprintf(stderr,"uniform MVPMatrix could not be found\n");
}
 
}

void SetUpFrame(GLFrame &frame,const M3DVector3f origin,
				const M3DVector3f forward,
				const M3DVector3f up) {
					frame.SetOrigin(origin);
					frame.SetForwardVector(forward);
M3DVector3f side,oUp;
m3dCrossProduct3(side,forward,up);
m3dCrossProduct3(oUp,side,forward);
frame.SetUpVector(oUp);
frame.Normalize();
};

void LookAt(GLFrame &frame, const M3DVector3f eye,
        const M3DVector3f at,
        const M3DVector3f up) {
    M3DVector3f forward;
    m3dSubtractVectors3(forward, at, eye);
    SetUpFrame(frame, eye, forward, up);
}

///////////////////////////////////////////////////////////////////////////////
// Called to draw scene

void RenderScene(void) {
    // Clear the window with current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   
    glUseProgram(shader);

glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glBegin(GL_QUADS);
//podstawa piramidy
   glColor3f(0.8f, 0.8f, 0.7f);
		glVertex3f(1.0f, 1.0f, 0.0f);//gorne prawe zaczepienie
		glVertex3f(-1.0f, 1.0f, 0.0f);//gorne lewe zaczepinie
		glVertex3f(1.0f, -1.0f, 0.0f);//prawe zaczepienie
		glVertex3f(-1.0f, -1.0f, 0.0f);//lewe zaczepienie
glEnd();

    glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.0, 0.0);

	//tylna �ciana (tr�jk�t)
   glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f(1.0f, 1.0f, 0.0f);//prawe zaczepinie
		glVertex3f(-1.0f, 1.0f, 0.0f);//lewe zaczepinie
		glVertex3f(0.0f, 0.0f, 1.0f);// gora

//prawa �ciana
   glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, -1.0f, 0.0f);// prawy
		glVertex3f(1.0f, 1.0f, 0.0f);//lewy (patrz�c na tr�jk�t)
		glVertex3f(0.0f, 0.0f, 1.0f);//gorny

//lewa �ciana
   glColor3f(0.0f, 0.0f, 0.4f);
		glVertex3f(-1.0f, 1.0f, 0.0f);//prawy
		glVertex3f(-1.0f, -1.0f, 0.0f);//lewy (patrz�c na tr�jk�t)
		glVertex3f(0.0f, 0.0f, 1.0f);// gora


//przednia �ciana

   glColor4f(0.05f, 1.0f, 0.3f, 0.4f);
		glVertex3f(1.0f, -1.0f, 0.0f);//prawy
		glVertex3f(-1.0f, -1.0f, 0.0f);//lewy
		glVertex3f(0.0f, 0.0f, 1.0f);// gorny

    glEnd();

// animacja
CStopWatch timer;
M3DVector3f at={0,0,0};
M3DVector3f up={0,0,1};
M3DVector3f eye;
float angle = timer.GetElapsedSeconds()* M3D_PI;

eye[0]=4.0f*cos(angle);
eye[1]=3.0f*sin(angle);
eye[2]=5.0f;
//LookAt(cameraFrame,eye,at,up);

glRotatef(0.2,0,-0.3, 0);

    // Perform the buffer swap to display back buffer
    glutSwapBuffers();
glutPostRedisplay();
}


///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs

int main(int argc, char* argv[]) {
  

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Triangle");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();

    glutMainLoop();
    return 0;
}