#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility
#include "planet.h"		// planet class definition
#include "trackball.h"

//*************************************
// global constants
static const char*	window_name = "Project 3 - Moving Planets";
static const char*	vert_shader_path = "../bin/shaders/project3.vert";
static const char*	frag_shader_path = "../bin/shaders/project3.frag";
uint				NUM_TESS = 72;		// initial tessellation factor of the circle as a polygon

//*************************************
// common structures


//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2(1280, 720); // initial window size
GLuint		vertex_array = 0;	// ID holder for vertex array object

//*************************************
// OpenGL objects
GLuint	program	= 0;	// ID holder for GPU program

//*************************************
// global variables
int		frame = 0;		// index of rendering frames
int		visualization = 0; //for text coordination visualization

bool	b_solid_color = false;
bool	b_wireframe = false;
auto	planets = std::move(create_planets());


//*************************************
// scene objects
std::vector<vertex> unit_sphere_vertices;
trackball	tb;

//*************************************
void update()
{
	// update projection matrix
	cam.aspect_ratio = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dNear, cam.dFar);

	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "b_solid_color");		if (uloc > -1) glUniform1i(uloc, b_solid_color);
	uloc = glGetUniformLocation(program, "view_matrix");		if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);		// update the view matrix
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);	// update the projection matrix
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// notify GL that we use our own program
	glUseProgram( program );
	
	// bind vertex array object
	glBindVertexArray(vertex_array);
	float t = float(glfwGetTime());
	for (auto& p: planets)
	{
		float pr = p.planetRadius;
		float rr = p.revRadius;

		mat4 model_matrix =	mat4::translate(rr * sin(t * p.revSpeed), 0, rr * cos(t * p.revSpeed)) *
							mat4::translate(cam.at) *
							mat4::rotate(vec3(0, 1, 0), t * p.rotSpeed) *
							mat4::translate(-cam.at) *
							mat4::scale(pr);

		// update the uniform model matrix and render
		glUniform4fv(glGetUniformLocation(program, "solid_color"), 1, p.rgb);	// pointer version
		glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_matrix);
		glDrawElements(GL_TRIANGLES, NUM_TESS * NUM_TESS * 3, GL_UNSIGNED_INT, nullptr);
	}
	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'w' to toggle wireframe\n" );
	printf( "- press 'd' to toggle between solid color and texture coordinates\n" );
	printf( "\n" );
}

std::vector<vertex> create_sphere_vertices(uint N)
{
	std::vector<vertex> v = { { vec3(0), vec3(0,0,-1.0f), vec2(0.5f) } };
	for (uint i = 0; i <= N; i++)
	{
		for (uint j = 0; j <= N / 2; j++) 
		{
			float t = PI * 2.0f * j / float(N), tc = cos(t), ts = sin(t);
			float p = PI * 2.0f * i / float(N), pc = cos(p), ps = sin(p);
			v.push_back({ vec3(ts * ps, tc, ts * pc), vec3(ts * pc, ts * ps, tc), vec2(p / (2.0f * PI), 1.0f - (t / PI)) });
		}
	}
	return v;
}

void update_vertex_buffer(const std::vector<vertex>& vertices, uint N)
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if (vertex_buffer)	glDeleteBuffers(1, &vertex_buffer);	vertex_buffer = 0;
	if (index_buffer)	glDeleteBuffers(1, &index_buffer);	index_buffer = 0;

	// check exceptions
	if (vertices.empty()) { printf("[error] vertices is empty.\n"); return; }

	// create buffers
	std::vector<uint> indices;
	for (uint i = 0; i < N; i++) 
	{
		for (uint j = 0; j < N / 2; j++)
		{
			//set the order for front-facing, not back-facing
			indices.push_back(i * (N / 2 + 1) + j + 1);
			indices.push_back(i * (N / 2 + 1) + j + 2);
			indices.push_back((i + 1) * (N / 2 + 1) + j + 2);

			indices.push_back(i * (N / 2 + 1) + j + 1);	
			indices.push_back((i + 1) * (N / 2 + 1) + j + 2);
			indices.push_back((i + 1) * (N / 2 + 1) + j + 1);
		}
	}

	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	//GL_ELEMENT_ARRAY_BUFFER == INDEX_BUFFER
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);
	

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if (vertex_array) glDeleteVertexArrays(1, &vertex_array);
	vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_D)
		{
			b_solid_color = !b_solid_color;
			printf("> using %s\n", b_solid_color ? "solid color" : "texture coordinates as color");
		}
#ifndef GL_ES_VERSION_2_0
		else if (key == GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, b_wireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", b_wireframe ? "wireframe" : "solid");
		}
#endif
	}
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{	
	dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
	vec2 npos = cursor_to_ndc(pos, window_size);
	tb.button = button;
	tb.mods = mods;

	if (action == GLFW_PRESS) tb.begin(cam.view_matrix, npos);//start view change
	else if (action == GLFW_RELEASE) tb.end();//stop view change
}

void motion(GLFWwindow* window, double x, double y)
{
	if (!tb.is_tracking()) return;
	vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);//커서 위치를 정규화된 좌표로 옮김
	if(tb.button==GLFW_MOUSE_BUTTON_LEFT&&tb.mods==0)
		cam.view_matrix = tb.update(npos);
	else if(tb.button == GLFW_MOUSE_BUTTON_MIDDLE || (tb.button==GLFW_MOUSE_BUTTON_LEFT&&(tb.mods & GLFW_MOD_CONTROL)))
		cam.view_matrix = tb.update_pan(npos);
	else if (tb.button == GLFW_MOUSE_BUTTON_RIGHT || (tb.button == GLFW_MOUSE_BUTTON_LEFT && (tb.mods & GLFW_MOD_SHIFT))) {
		cam.view_matrix = tb.update_zoom(npos);
	}
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests

	unit_sphere_vertices = create_sphere_vertices(NUM_TESS);

	update_vertex_buffer(unit_sphere_vertices, NUM_TESS);
	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// version and extensions

	// initializations and validations
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movement

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
