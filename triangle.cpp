// triangle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <GLTools.h>            // OpenGL toolkit


#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif


GLuint shader;

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.

void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);
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
		glVertex3f(0.3f, 0.2f, -1.0f);//gorne prawe zaczepienie
		glVertex3f(-0.3f, 0.2f, -1.0f);//gorne lewe zaczepinie
		glVertex3f(0.3f, -0.5f, 1.0f);//prawe zaczepienie
		glVertex3f(-0.3f, -0.5f, 1.0f);//lewe zaczepienie
glEnd();

    glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.0, 0.0);

	//tylna œciana (trójk¹t)
   glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f(0.3f, 0.2f, -1.0f);//prawe zaczepinie
		glVertex3f(-0.3f, 0.2f, -1.0f);//lewe zaczepinie
		glVertex3f(0.0f, 0.0f, 0.0f);// gora

//prawa œciana
   glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.3f, -0.5f, -1.0f);// prawy
		glVertex3f(0.3f, 0.2f, 1.0f);//lewy (patrz¹c na trójk¹t)
		glVertex3f(0.0f, 0.0f, 0.0f);//gorny

//lewa œciana
   glColor3f(0.0f, 0.0f, 0.4f);
		glVertex3f(-0.3f, 0.2f, 1.0f);//prawy
		glVertex3f(-0.3f, -0.5f, -1.0f);//lewy (patrz¹c na trójk¹t)
		glVertex3f(0.0f, 0.0f, 0.0f);// gora


//przednia œciana

   glColor4f(0.05f, 1.0f, 0.3f, 1.0f);
		glVertex3f(0.3f, -0.5f, 1.0f);//prawy
		glVertex3f(-0.3f, -0.5f, 1.0f);//lewy
		glVertex3f(0.0f, 0.0f, 0.0f);// gorny

    glEnd();



    // Perform the buffer swap to display back buffer
    glutSwapBuffers();
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