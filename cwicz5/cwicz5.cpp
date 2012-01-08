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
#include <vector>

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
   float ka; 
   float kd; 
   float ks; 
   float alpha; 

   void set_parameters(float ka, float kd, float ks, float alpha) {
  	this->ka = ka;
  	this->kd = kd;
  	this->ks = ks;
  	this->alpha = alpha;
   }
};

GLuint shader_swiatlo,
   	ShaderColor,
   	mvpMatrix_location_ShaderColor,
   	mvpMatrix_location,
   	mvMatrix_location,
   	vMatrix_location,
   	NormalMatrix_location,
   	intensity_ambient_component_location,
   	swiatlo_0_position_location,
   	swiatlo_0_intensity_diffuse_location,
   	swiatlo_0_intensity_specular_location,
   	swiatlo_0_attenuation_location,
   	material_0_ka_location,
   	material_0_kd_location,
   	material_0_ks_location,
   	material_0_alpha_location;
GLGeometryTransform geometry_pipeline;
GLMatrixStack p_stack;
GLMatrixStack mv_stack;
GLFrame camera_frame;
GLFrustum view_frustum;
float location[] = {1.0f, 0.5f, 1.0f}, target[] = {1.0f, 0.0f, 0.0f}, up_dir[] = {0.0f, 0.0f, 1.0f}, camera_matrix[16], intensity_ambient_component[] = {0.4f, 0.1f, 0.1f};
CStopWatch timer;
punktowe_swiatlo swiatlo_0;
Material material_0;
std::vector<float> vertices;
std::vector<unsigned int> faces;
GLuint vertex_buffer;
GLuint faces_buffer;

void load_vertices(const char * filename, std::vector<float> & out_vertices) {
   FILE * fvertices = fopen(filename, "r");
   char line[120];
   float x, y, z, magnitude;
   while (fgets(line, 120, fvertices) != NULL) {
  	sscanf(line,"%f %f %f", &x, &y, &z);
  	magnitude = sqrt(x * x + y * y + z * z);
  	out_vertices.push_back(x);
  	out_vertices.push_back(y);
  	out_vertices.push_back(z);
  	out_vertices.push_back(1.0f);
  	out_vertices.push_back(x / magnitude);
  	out_vertices.push_back(y / magnitude);
  	out_vertices.push_back(z / magnitude);
   }
}

void load_faces(const char * filename, std::vector<unsigned int> & out_faces) {
   FILE * ffaces = fopen(filename, "r");
   char line[120];
   int i, j, k;
   while (fgets(line, 120, ffaces) != NULL) {
  	sscanf(line, "%u %u %u", &i, &j, &k);
  	out_faces.push_back(i - 1);
  	out_faces.push_back(j - 1);
  	out_faces.push_back(k - 1);
   }
}

void set_up_frame(GLFrame & frame, const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
   frame.SetOrigin(origin);
   frame.SetForwardVector(forward);
   M3DVector3f side, o_up;
   m3dCrossProduct3(side, forward, up);
   m3dCrossProduct3(o_up, side, forward);
   frame.SetUpVector(o_up);
   frame.Normalize();
}

void look_at(GLFrame & frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
   M3DVector3f forward;
   m3dSubtractVectors3(forward, at, eye);
   set_up_frame(frame, eye, forward, up);
}

void change_size(int w, int h) {
   view_frustum.SetPerspective(65.0f, (float)w / h, 1.0f, 100.0f);
   glViewport(0, 0, w, h);
}

void setup_rc() {
   load_vertices("geode_vertices.dat", vertices);
   load_faces("geode_faces.dat", faces);
   glGenBuffers(1, &vertex_buffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
   glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 4 ,GL_FLOAT,GL_FALSE, sizeof(float) * 7, (const GLvoid *)0);
   glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const GLvoid *)(4*sizeof(float)) );
   glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
   glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
   //---
   glGenBuffers(1, &faces_buffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_buffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(GLuint), &faces[0], GL_STATIC_DRAW);
   //---
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   shader_swiatlo = gltLoadShaderPairWithAttributes("gouraud_shader.vp", "gouraud_shader.fp", 3, GLT_ATTRIBUTE_VERTEX, "vPosition", GLT_ATTRIBUTE_COLOR, "vColor", GLT_ATTRIBUTE_NORMAL, "vertex_normal");
   ShaderColor = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vPosition", GLT_ATTRIBUTE_COLOR, "vColor");
   mvpMatrix_location_ShaderColor = glGetUniformLocation(ShaderColor, "mvpMatrix");
   mvpMatrix_location = glGetUniformLocation(shader_swiatlo, "mvpMatrix");
   mvMatrix_location = glGetUniformLocation(shader_swiatlo, "mvMatrix");
   vMatrix_location = glGetUniformLocation(shader_swiatlo, "vMatrix");
   NormalMatrix_location = glGetUniformLocation(shader_swiatlo, "NormalMatrix");
   intensity_ambient_component_location = glGetUniformLocation(shader_swiatlo, "intensity_ambient_component");
   swiatlo_0_position_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.position");
   swiatlo_0_intensity_diffuse_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.intensity_diffuse");
   swiatlo_0_intensity_specular_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.intensity_specular");
   swiatlo_0_attenuation_location = glGetUniformLocation(shader_swiatlo, "swiatlo_0.attenuation");
   material_0_ka_location = glGetUniformLocation(shader_swiatlo, "material_0.ka");
   material_0_kd_location = glGetUniformLocation(shader_swiatlo, "material_0.kd");
   material_0_ks_location = glGetUniformLocation(shader_swiatlo, "material_0.ks");
   material_0_alpha_location = glGetUniformLocation(shader_swiatlo, "material_0.alpha");
   swiatlo_0.set_position(1.0f, 0.0f, 0.0f);
   swiatlo_0.set_intensity_diffuse(1.0f, 0.0f, 1.0f);
   swiatlo_0.set_intensity_specular(1.0f, 1.0f, 1.0f);
   swiatlo_0.set_attenuation(0.0f, 0.1f, 0.0f);
   material_0.set_parameters(0.5f, 0.5f, 0.5f, 100.0f);
   geometry_pipeline.SetMatrixStacks(mv_stack, p_stack);
   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);
   glFrontFace(GL_CCW);
}

void draw_trinagle_face_no_normal(const float A[], const float B[], const float C[], float r = 1.0f, float g = 1.0f, float b = 1.0f) {
   glBegin(GL_TRIANGLES);
  	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, r, g, b);
  	glVertex3fv(A);
  	glVertex3fv(B);
  	glVertex3fv(C);
   glEnd();
}

void fill_array_3(float out_array[], float a0, float a1,float a2) {
   out_array[0] = a0;
   out_array[1] = a1;
   out_array[2] = a2;
}
void rysuj_swiatlo() {
   float A[3], B[3], C[3], normal[3];

   fill_array_3(A, 1.0f, -1.0f, 0.0f);
   fill_array_3(B, -1.0f, -1.0f, 0.0f);
   fill_array_3(C, 1.0f, 1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.5f, 1.0f, 0.0f);
   fill_array_3(A, -1.0f, 1.0f, 0.0f);
   fill_array_3(B, 1.0f, 1.0f, 0.0f);
   fill_array_3(C, -1.0f, -1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.5f, 1.0f, 0.0f);
  
   fill_array_3(A, 1.0f, -1.0f, 0.0f);
   fill_array_3(B, 1.0f, 1.0f, 0.0f);
   fill_array_3(C, 0.0f, 0.0f, 2.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.5f, 1.0f, 0.0f);

   fill_array_3(A, 1.0f, 1.0f, 0.0f);
   fill_array_3(B, -1.0f, 1.0f, 0.0f);
   fill_array_3(C, 0.0f, 0.0f, 2.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.5f, 1.0f, 0.0f);

   fill_array_3(A, -1.0f, -1.0f, 0.0f);
   fill_array_3(B, 0.0f, 0.0f, 2.0f);
   fill_array_3(C, -1.0f, 1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.5f, 1.0f, 0.0f);

   fill_array_3(A, 1.0f, -1.0f, 0.0f);
   fill_array_3(B, 0.0f, 0.0f, 2.0f);
   fill_array_3(C, -1.0f, -1.0f, 0.0f);
   draw_trinagle_face_no_normal(A, B, C, 1.5f, 1.0f, 0.0f);
}

void render_scene(void) {
   float angle = timer.GetElapsedSeconds() * 3.14f / 5.0f;
   location[0] = -11.0f * cos(angle / 2.0f);// ustawienie kamery
   location[1] = -11.0f * sin(angle / 2.0f);
   location[2] = 4.0f;
   swiatlo_0.position[0] = -6.0f * cos(-angle);
   swiatlo_0.position[1] = -6.0f * sin(-angle);
   swiatlo_0.position[2] = 4.0f;
   look_at(camera_frame, location, target, up_dir);
   camera_frame.GetCameraMatrix(camera_matrix);
   p_stack.LoadMatrix(view_frustum.GetProjectionMatrix());
   mv_stack.LoadMatrix(camera_matrix);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   glUseProgram(ShaderColor);
   mv_stack.PushMatrix();
   mv_stack.Translate(swiatlo_0.position[0], swiatlo_0.position[1], swiatlo_0.position[2]);
   mv_stack.Scale(0.25f, 0.25f, 0.25f);
   glUniformMatrix4fv(mvpMatrix_location_ShaderColor, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
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
   glUniform1f(material_0_ka_location, material_0.ka);
   glUniform1f(material_0_kd_location, material_0.kd);
   glUniform1f(material_0_ks_location, material_0.ks);
   glUniform1f(material_0_alpha_location, material_0.alpha);
   // liczba kulek
   for(int i = -4; i <= 4; i += 2)
   for(int j = -5; j <= 5; j += 2) {
mv_stack.PushMatrix();
mv_stack.Translate(i, j, 0.0f);
  	glUniformMatrix4fv(mvpMatrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewProjectionMatrix());
  	glUniformMatrix4fv(mvMatrix_location, 1, GL_FALSE, geometry_pipeline.GetModelViewMatrix());
  	glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, 0);
  	mv_stack.PopMatrix();
   }

   glUseProgram(0);
   glutSwapBuffers();
   glutPostRedisplay();
}

int main(int argc, char* argv[]) {
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
   glutInitWindowSize(600, 600);
   glutCreateWindow("cwiczenie 5");
   glutReshapeFunc(change_size);
   glutDisplayFunc(render_scene);
   GLenum err = glewInit();
   if(GLEW_OK != err) {
  	return 1;
   }
   setup_rc();
   glutMainLoop();
   return 0;
}