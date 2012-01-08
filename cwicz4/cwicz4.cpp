#include "stdafx.h"
#include <GLFrame.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLTools.h>
#include <GLFrustum.h>
#include <StopWatch.h>
#include <math3d.h>
#define FREEGLUT_STATIC
#include <GL/glut.h>

float vertices_icosahedron[3 * 12] = { 0.0,             	0.0,            	-0.9510565162951536,
                                   	0.0,             	0.0,             	0.9510565162951536,
                                  	-0.85065080835204,	0.0,            	-0.42532540417601994,
                                   	0.85065080835204,	0.0,             	0.42532540417601994,
                                   	0.6881909602355868, -0.5,            	-0.42532540417601994,
                                   	0.6881909602355868,  0.5,            	-0.42532540417601994,
                                  	-0.6881909602355868, -0.5,             	0.42532540417601994,
                                  	-0.6881909602355868,  0.5,             	0.42532540417601994,
                                  	-0.2628655560595668, -0.8090169943749475, -0.42532540417601994,
                                  	-0.2628655560595668,  0.8090169943749475, -0.42532540417601994,
                                   	0.2628655560595668, -0.8090169943749475,  0.42532540417601994,
                                   	0.2628655560595668,  0.8090169943749475,  0.42532540417601994
                                 	};
int faces_icosahedron[3 * 20] = {  1, 11,  7,
                               	1,  7,  6,
                               	1,  6, 10,
                               	1, 10,  3,
                               	1,  3, 11,
                               	4,  8,  0,
                               	5,  4,  0,
                               	9,  5,  0,
                               	2,  9,  0,
                               	8,  2,  0,
                              	11,  9,  7,
                               	7,  2,  6,
                               	6,  8, 10,
                              	10,  4,  3,
                               	3,  5, 11,
                               	4, 10,  8,
                               	5,  3,  4,
                               	9, 11,  5,
                               	2,  7,  9,
                               	8,  6,  2 };

struct punktowe_swiatlo {
   float position[3];
   float intensity_diffuse[3];
   float intensity_specular[3];
   float attenuation[3];

   void set_position(float x, float y, float z) {
  	position[0] = x;
  	position[1] = y;
  	position[2] = z;
   }

   void set_intensity_diffuse(float r, float g, float b) {
  	intensity_diffuse[0] = r;
  	intensity_diffuse[1] = g;
  	intensity_diffuse[2] = b;
   }

   void set_intensity_specular(float r, float g, float b) {
  	intensity_specular[0] = r;
  	intensity_specular[1] = g;
  	intensity_specular[2] = b;
   }

     void set_attenuation(float attenuation_0, float attenuation_1, float attenuation_2) {
  	attenuation[0] = attenuation_0;
  	attenuation[1] = attenuation_1;
  	attenuation[2] = attenuation_2;
   }
};

struct Material {
   float r_ambient;//ka; 
   float r_diffuse;//kd; 
   float r_spectacular;//ks;  
   float alpha; // shininess constant, which is larger for surfaces that are smoother and more mirror-like. When this constant is large the specular highswiatlo is small

   void set_parameters(float r_ambient, float r_diffuse, float r_spectacular, float alpha) {
  	this->r_ambient = r_ambient;
  	this->r_diffuse = r_diffuse;
  	this->r_spectacular = r_spectacular;
  	this->alpha = alpha;
   }
};
GLuint shader_swiatlo,
   	shader_color,
   	MVPMatrixLocation_shader_color,
   	MVPMatrixLocation,
   	mvMatrix_location,
   	vMatrix_location,
   	NormalMatrix_location,
   	intensity_ambient_component_location,
   	swiatlo_0_position_location,
   	swiatlo_0_intensity_diffuse_location,
   	swiatlo_0_intensity_specular_location,
   	swiatlo_0_attenuation_location,
   	material_0_r_ambient_location,
   	material_0_r_diffuse_location,
   	material_0_r_spectacular_location,
   	material_0_alpha_location;
GLGeometryTransform geometry_pipeline;
GLMatrixStack p_stack;
GLMatrixStack mv_stack;
GLFrame cameraFrame;
GLFrustum viewFrustum;

//ustawienie poczatkowe kamery
float location[] = {1.5f, -1.0f, -1.5f};
float target[] = {0.0f, 0.0f, 0.0f};
float up_dir[] = {0.0f, 0.0f, 1.0f};
float camera_matrix[16];
float intensity_ambient_component[] = {0.2f, 0.2f, 0.2f};


CStopWatch timer;
punktowe_swiatlo swiatlo_0;
Material material_0;
void SetUpFrame(GLFrame & frame, const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
   frame.SetOrigin(origin);
   frame.SetForwardVector(forward);
   M3DVector3f side, oUp;
   m3dCrossProduct3(side, forward, up);
   m3dCrossProduct3(oUp, side, forward);
   frame.SetUpVector(oUp);
   frame.Normalize();
}
void LookAt(GLFrame & frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
   M3DVector3f forward;
   m3dSubtractVectors3(forward, at, eye);
   SetUpFrame(frame, eye, forward, up);
}

void change_size(int w, int h) {
   viewFrustum.SetPerspective(70.0f, (float)w / h, 1.0f, 150.0f);
   glViewport(0, 0, w, h);
}
void SetupRC() {
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   shader_swiatlo = gltLoadShaderPairWithAttributes("gouraud_shader.vp", "gouraud_shader.fp", 3, GLT_ATTRIBUTE_VERTEX, "vPosition", GLT_ATTRIBUTE_COLOR, "vColor", GLT_ATTRIBUTE_NORMAL, "vertex_normal");
   shader_color = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vPosition", GLT_ATTRIBUTE_COLOR, "vColor");
   MVPMatrixLocation_shader_color = glGetUniformLocation(shader_color, "mvpMatrix");
   MVPMatrixLocation = glGetUniformLocation(shader_swiatlo, "mvpMatrix");
   mvMatrix_location = glGetUniformLocation(shader_swiatlo, "mvMatrix");
   vMatrix_location = glGetUniformLocation(shader_swiatlo, "vMatrix");
   NormalMatrix_location = glGetUniformLocation(shader_swiatlo, "NormalMatrix");
   intensity_ambient_component_location = glGetUniformLocation(shader_swiatlo, "intensity_ambient_component");
   swiatlo_0_position_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.position");
   swiatlo_0_intensity_diffuse_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.intensity_diffuse");
   swiatlo_0_intensity_specular_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.intensity_specular");
   swiatlo_0_attenuation_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.attenuation");
   material_0_r_ambient_location = glGetUniformLocation(shader_swiatlo, "material_0.r_ambient");
   material_0_r_diffuse_location = glGetUniformLocation(shader_swiatlo, "material_0.r_diffuse");
   material_0_r_spectacular_location = glGetUniformLocation(shader_swiatlo, "material_0.r_spectacular");
   material_0_alpha_location = glGetUniformLocation(shader_swiatlo, "material_0.alpha");
   swiatlo_0.set_position(0.0f, 0.0f, 0.0f);
   swiatlo_0.set_intensity_diffuse(1.0f, 1.0f, 1.0f);
   swiatlo_0.set_intensity_specular(0.0f, 0.0f, 0.0f);
   swiatlo_0.set_attenuation(0.1f, 0.1f, 0.0f);// Jasnoœæ œwiat³a od obiektu
   material_0.set_parameters(1.0f, 1.0f, 1.0f, 200.0f);
   geometry_pipeline.SetMatrixStacks(mv_stack, p_stack);
   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);
   glFrontFace(GL_CCW);
}

void draw_trinagle_face(const float A[], const float B[], const float C[], float normal[3], float r = 1.0f, float g = 1.0f, float b = 1.0f) {
   glBegin(GL_TRIANGLES);
  	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, r, g, b);
  	glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
  	glVertex3fv(A);
  	glVertex3fv(B);
  	glVertex3fv(C);
   glEnd();
}

void draw_trinagle_face_no_normal(const float A[], const float B[], const float C[], float r = 1.0f, float g = 1.0f, float b = 1.0f) {
   glBegin(GL_TRIANGLES);
  	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, r, g, b);
  	glVertex3fv(A);
  	glVertex3fv(B);
  	glVertex3fv(C);
   glEnd();
}

void calculate_normal_vector(float result[], const float A[], const float B[], const float C[]) {
   float C_min_B[3], A_min_B[3];
   m3dSubtractVectors3(C_min_B, C, B);
   m3dSubtractVectors3(A_min_B, A, B);
   m3dCrossProduct3(result, C_min_B, A_min_B);
   m3dNormalizeVector3(result);
}

void fill_array_3(float out_array[], float a0, float a1,float a2) {
   out_array[0] = a0;
   out_array[1] = a1;
   out_array[2] = a2;
}

void rysuj_piramide() {
   float A[3], B[3], C[3], normal[3];
   //// prawa sciana
   fill_array_3(A, 1.5f, -1.5f, 0.0f);
   fill_array_3(B, 1.5f, 1.5f, 0.0f);
   fill_array_3(C, 0.0f, 0.0f, 2.0f);
   calculate_normal_vector(normal, A, B, C);
   draw_trinagle_face(A, B, C, normal, 0.4f, 0.8f, 1.5f);
   //-- tyl
   fill_array_3(A, 1.5f, 1.5f, 0.0f);
   fill_array_3(B, -1.5f, 1.5f, 0.0f);
   fill_array_3(C, 0.0f, 0.0f, 2.0f);
   calculate_normal_vector(normal, A, B, C);
   draw_trinagle_face(A, B, C, normal, 0.0f, 0.0f, 0.0f);
   //-- lewa
   fill_array_3(A, -1.5f, -1.5f, 0.0f);
   fill_array_3(B, 0.0f, 0.0f, 2.0f);
   fill_array_3(C, -1.5f, 1.5f, 0.0f);
   calculate_normal_vector(normal, A, B, C);
   draw_trinagle_face(A, B, C, normal, 1.0f, 0.0f, 0.0f);
   //-- przod
   fill_array_3(A, 1.5f, -1.5f, 0.0f);
   fill_array_3(B, 0.0f, 0.0f, 2.0f);
   fill_array_3(C, -1.5f, -1.5f, 0.0f);
   calculate_normal_vector(normal, A, B, C);
   draw_trinagle_face(A, B, C, normal, 1.5f, 1.5f, 1.5f);
}

void rysuj_swiatlo() {
   float A[3], B[3], C[3], normal[3];
   // podstawa
	fill_array_3(A, 1.0f, -1.0f, 0.0f);
   fill_array_3(B, -1.0f, -1.0f, 0.0f);
   fill_array_3(C,  1.0f,  1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 0.5f, 0.2f);
  fill_array_3(A, -1.0f, 1.0f, 0.0f);
   fill_array_3(B, 1.0f, 1.0f, 0.0f);
   fill_array_3(C,-1.0f,-1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 0.5f, 0.2f);
   //// sciany
   fill_array_3(A, 1.0f,-1.0f, 0.0f);
   fill_array_3(B, 1.0f, 1.0f, 0.0f);
   fill_array_3(C, 0.0f, 0.0f, 3.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 0.5f, 0.2f);

   fill_array_3(A, 1.0f, 1.0f, 0.0f);
   fill_array_3(B,-1.0f, 1.0f, 0.0f);
   fill_array_3(C, 0.0f, 0.0f, 3.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.0f, 0.5f, 0.2f);

   fill_array_3(A,-1.0f, -1.0f, 0.0f);
   fill_array_3(B, 0.0f, 0.0f, 3.0f);
   fill_array_3(C, -1.0f, 1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C,  1.0f, 0.5f, 0.2f);
 
   fill_array_3(A, 1.0f, -1.0f, 0.0f);
   fill_array_3(B, 0.0f, 0.0f, 3.0f);
   fill_array_3(C, -1.0f, -1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C,  1.0f, 0.5f, 0.2f);
}

void draw_icosahedron(int n_faces, float * vertices, int * faces) {
   float normal[3];
   for(int i = 0; i < n_faces; ++i) {
  	calculate_normal_vector(normal, vertices + 3 * faces[3 * i], vertices + 3 * faces[3 * i + 1], vertices + 3 * faces[3 * i + 2]);
  	draw_trinagle_face(vertices + 3 * faces[3 * i], vertices + 3 * faces[3 * i + 1], vertices + 3 * faces[3 * i + 2], normal, 0.8f, 0.8f, 0.7f);
   }
}

void draw_icosahedron_smooth(int n_faces, float *vertices, int *faces) {
   float normal[3];
   for(int i = 0; i < n_faces; ++i) {
  	for(int j=0 ; j < 3 ; ++j) {
     	m3dCopyVector3(normal, vertices + 3 * faces[i * 3 + j]);
     	m3dNormalizeVector3(normal);
     	draw_trinagle_face(vertices + 3 * faces[3 * i], vertices + 3 * faces[3 * i + 1], vertices + 3 * faces[3 * i + 2], normal, 1.0f, 0.0f, 0.0f);
  	}
   }
}

void rysuj_podloge() {
   float A[3], B[3], C[3], normal[] = {0.0f, 0.0f, 1.0f};
   for(int i = -5; i < 10; i += 2) {
  	for(int j = -5; j < 10; j += 2) {
     	fill_array_3(A, (float)i + -1.0f, (float)j + -1.0f, -0.1f);
     	fill_array_3(B, (float)i + 1.0f, (float)j + -1.0f, -0.1f);
     	fill_array_3(C, (float)i + 1.0f, (float)j + 1.0f, -0.1f);
     	draw_trinagle_face(A, B, C, normal,0.05f, 0.8f, 0.0f);
     	fill_array_3(A, (float)i + 1.0f, (float)j + 1.0f, -0.1f);
     	fill_array_3(B, (float)i + -1.0f, (float)j + 1.0f, -0.1f);
     	fill_array_3(C, (float)i + -1.0f, (float)j + -1.0f, -0.1f);
     	draw_trinagle_face(A, B, C, normal, 0.05f, 0.8f, 0.0f);
  	}
   }
}



void render_scene(void) {
   float angle = timer.GetElapsedSeconds() * 3.14f / 5.0f;
   location[0] = 10.0f * cos(angle / 2.0f);//ustawienie kamery
   location[1] = 10.0f * sin(angle / 2.0f);
   location[2] = 8.0f;
   swiatlo_0.position[0] = 5.0f * cos(-angle);
   swiatlo_0.position[1] = 5.0f * sin(-angle);
   swiatlo_0.position[2] = 9.0f;
   LookAt(cameraFrame, location, target, up_dir);
   cameraFrame.GetCameraMatrix(camera_matrix);
   p_stack.LoadMatrix(viewFrustum.GetProjectionMatrix());
   mv_stack.LoadMatrix(camera_matrix);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
 
   glUseProgram(shader_color);
   mv_stack.PushMatrix();
   mv_stack.Translate(swiatlo_0.position[0], swiatlo_0.position[1], swiatlo_0.position[2]);
   mv_stack.Scale(0.25f, 0.25f, 0.25f);
   glUniformMatrix4fv(MVPMatrixLocation_shader_color, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   rysuj_swiatlo();
   mv_stack.PopMatrix();
 
   glUseProgram(shader_swiatlo);
   glUniformMatrix3fv(NormalMatrix_location, 1, GL_FALSE, geometry_pipeline.GetNormalMatrix());
   glUniformMatrix4fv(vMatrix_location, 1, GL_FALSE, camera_matrix);
   glUniform3fv(intensity_ambient_component_location, 1, intensity_ambient_component);
   glUniform3fv(swiatlo_0_position_location, 1, swiatlo_0.position);
   glUniform3fv(swiatlo_0_intensity_diffuse_location, 1, swiatlo_0.intensity_diffuse);
   glUniform3fv(swiatlo_0_intensity_specular_location, 1, swiatlo_0.intensity_specular);
   glUniform3fv(swiatlo_0_attenuation_location, 1, swiatlo_0.attenuation);
   glUniform1f(material_0_r_ambient_location, material_0.r_ambient);
   glUniform1f(material_0_r_diffuse_location, material_0.r_diffuse);
   glUniform1f(material_0_r_spectacular_location, material_0.r_spectacular);
   glUniform1f(material_0_alpha_location, material_0.alpha);
 
   glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   glUniformMatrix4fv(mvMatrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewMatrix());
 
   rysuj_podloge();
  //ilosc piramid
    for(int i = -2; i < 4; i +=3) {
  	for(int j = -2; j < 4; j +=3) {
     	mv_stack.PushMatrix();
     	mv_stack.Translate((float)i, (float)j, 0.0f);
     	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
     	glUniformMatrix4fv(mvMatrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewMatrix());
     	rysuj_piramide();
   	    mv_stack.PopMatrix();
  	}
   }
   
   mv_stack.PushMatrix();
   mv_stack.Translate(2.0f, 0.0f, 4.0f);
   glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   glUniformMatrix4fv(mvMatrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewMatrix());
   draw_icosahedron(20, vertices_icosahedron, faces_icosahedron);
   mv_stack.PopMatrix();
 
   mv_stack.PushMatrix();
   mv_stack.Translate(-2.0f, 0.0f, 4.0f);
   glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
   glUniformMatrix4fv(mvMatrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewMatrix());
   draw_icosahedron_smooth(20, vertices_icosahedron, faces_icosahedron);
   mv_stack.PopMatrix();
   //--
   glUseProgram(0);
   glutSwapBuffers();
   glutPostRedisplay();
}

int main(int argc, char* argv[]) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(600, 600);
   glutCreateWindow("cwiczenie 4");
   glutReshapeFunc(change_size);
   glutDisplayFunc(render_scene);
   GLenum err = glewInit();
   if(GLEW_OK != err) {
  	return 1;
   }
   SetupRC();
   glutMainLoop();
   return 0;
}