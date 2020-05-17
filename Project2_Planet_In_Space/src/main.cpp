#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility

//*************************************
// global constants
static const char*	window_name = "Project 2 - Planet in Space";
static const char*	vert_shader_path = "../bin/shaders/project2.vert";
static const char*	frag_shader_path = "../bin/shaders/project2.frag";
float				MAX_RADIUS = 1;
float				MIN_RADIUS = 0.1f;
float				SIZE_RADIUS = 1;
uint				NUM_TESS = 72;		// initial tessellation factor of the circle as a polygon

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

//time parameters to control the rotation
float	xf_stop_time = 0;
float	xf_blank_time = 0;
float	xr_stop_time = 0;
float	xr_blank_time = 0;
float	yf_stop_time = 0;
float	yf_blank_time = 0;
float	yr_stop_time = 0;
float	yr_blank_time = 0;
float	zf_stop_time = 0;
float	zf_blank_time = 0;
float	zr_stop_time = 0;
float	zr_blank_time = 0;

bool	b_wireframe = false;

//bool parameters for checking the rotation
bool	xf_rotate = false;
bool	xr_rotate = false;
bool	yf_rotate = false;
bool	yr_rotate = false;
bool	zf_rotate = false;
bool	zr_rotate = false;

struct {
	bool add = false, sub = false;
	operator bool() const { return add || sub; }
} b; // flags of keys for smooth changes

//*************************************
// scene objects
std::vector<vertex> unit_sphere_vertices;

//*************************************
int is_rotate() {//function to find the direction of rotation
	if (xf_rotate)
		return 1;
	else if (xr_rotate)
		return 2;
	else if (yf_rotate)
		return 3;
	else if (yr_rotate)
		return 4;
	else if (zf_rotate)
		return 5;
	else if (zr_rotate)
		return 6;
	return 0;
}

char return_char(int flag) {//function to return the alphabet for help
	switch (flag) {
	case 1:
		return 'f';
	case 2:
		return 'g';
	case 3:
		return 'v';
	case 4:
		return 'b';
	case 5:
		return 'r';
	case 6:
		return 't';
	default:
		return 'x';
	}
}

void update()
{
	// update projection matrix
	float aspect = window_size.x / float(window_size.y);
	mat4 aspect_matrix = 
	{
		min(1 / aspect,1.0f), 0, 0, 0,
		0, min(aspect,1.0f), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 view_projection_matrix = 
	{	0,	1,	0,	0, 
		0,	0,	1,	0,
		-1, 0,	0,	1, 
		0,	0,	0,	1 
	};


	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "view_projection_matrix");			if(uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, view_projection_matrix);		
	uloc = glGetUniformLocation(program, "aspect_matrix");					if(uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, aspect_matrix);
	uloc = glGetUniformLocation(program, "visualization");					if(uloc > -1) glUniform1i(uloc, visualization);
	
	// update vertex buffer by the pressed keys
	void update_tess(); // forward declaration
	if (b) update_tess();
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// notify GL that we use our own program
	glUseProgram( program );
	
	// bind vertex array object
	glBindVertexArray(vertex_array);
	
	// configure transformation parameters
	float xft = xf_rotate ? float(glfwGetTime()) - xf_blank_time : xf_stop_time - xf_blank_time;
	float xrt = xr_rotate ? float(glfwGetTime()) - xr_blank_time : xr_stop_time - xr_blank_time;

	float yft = yf_rotate ? float(glfwGetTime()) - yf_blank_time : yf_stop_time - yf_blank_time;
	float yrt = yr_rotate ? float(glfwGetTime()) - yr_blank_time : yr_stop_time - yr_blank_time;

	float zft = zf_rotate ? float(glfwGetTime()) - zf_blank_time : zf_stop_time - zf_blank_time;
	float zrt = zr_rotate ? float(glfwGetTime()) - zr_blank_time : zr_stop_time - zr_blank_time;

	// build the model matrix
	mat4 model_matrix = mat4::rotate(vec3(1, 0, 0), xft - xrt) *
						mat4::rotate(vec3(0, 1, 0), yft - yrt) *
						mat4::rotate(vec3(0, 0, 1), zft - zrt);

	// update the uniform model matrix and render
	glUniformMatrix4fv( glGetUniformLocation( program, "model_matrix" ), 1, GL_TRUE, model_matrix );
	glDrawElements( GL_TRIANGLES, NUM_TESS * NUM_TESS * 3, GL_UNSIGNED_INT, nullptr );

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
	printf( "- press 'd' to toggle (tc.xy,0) > (tc.xxx) > (tc.yyy)\n" );
	printf( "- press '+/-' to increase/decrease the radius of the sphere\n" );
	printf( "- press 'f' or 'g' to rotate the sphere on x axis\n");
	printf( "- press 'v' or 'b' to rotate the sphere on y axis\n");
	printf( "- press 'r' or 't' to rotate the sphere on z axis\n");
	printf( "\n" );
}

std::vector<vertex> create_sphere_vertices(uint N)
{
	std::vector<vertex> v = { { vec3(0), vec3(0,0,-1.0f), vec2(0.5f) } };
	for (uint i = 0; i <= N; i++)
	{
		for (uint j = 0; j <= N / 2; j++) 
		{
			float r = SIZE_RADIUS;
			float t = PI * 2.0f * j / float(N), tc = cos(t), ts = sin(t);
			float p = PI * 2.0f * i / float(N), pc = cos(p), ps = sin(p);
			v.push_back({ vec3(r * ts * pc, r * ts * ps, r * tc), vec3(ts * pc, ts * ps, tc), vec2(p / (2.0f * PI), 1.0f - (t / PI)) });
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

void update_tess()
{
	float n = SIZE_RADIUS; if (b.add) n = n + 0.001f; if (b.sub) n = n - 0.001f;
	if (n == SIZE_RADIUS || n<MIN_RADIUS || n>MAX_RADIUS) return;

	SIZE_RADIUS = n;
	unit_sphere_vertices = create_sphere_vertices(NUM_TESS);
	update_vertex_buffer(unit_sphere_vertices, NUM_TESS);
	printf("> SIZE_RADIUS = %.4f\n", SIZE_RADIUS);
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		int flag = is_rotate();
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_KP_ADD || (key == GLFW_KEY_EQUAL && (mods & GLFW_MOD_SHIFT))) {
			
			b.add = true;
			
		}
		else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS) {
			
			b.sub = true;
			
		}
		else if (key == GLFW_KEY_R)
		{
			if (flag == 5 || flag == 0) {
				if (zf_rotate)
					zf_stop_time = float(glfwGetTime());
				else
					zf_blank_time += (float(glfwGetTime()) - zf_stop_time);
				zf_rotate = !zf_rotate;
			}
			else 
				printf("> Stop rotation first!(Press '%c' to stop)\n", return_char(flag));
		}
		else if (key == GLFW_KEY_T)
		{
			if (flag == 6 || flag == 0) {
				if (zr_rotate)
					zr_stop_time = float(glfwGetTime());
				else
					zr_blank_time += (float(glfwGetTime()) - zr_stop_time);
				zr_rotate = !zr_rotate;
			}
			else
				printf("> Stop rotation first!(Press '%c' to stop)\n", return_char(flag));
		}
		else if (key == GLFW_KEY_F)
		{
			if (flag == 1 || flag == 0) {
				if (xf_rotate)
					xf_stop_time = float(glfwGetTime());
				else
					xf_blank_time += (float(glfwGetTime()) - xf_stop_time);
				xf_rotate = !xf_rotate;
			}
			else
				printf("> Stop rotation first!(Press '%c' to stop)\n", return_char(flag));
		}
		else if (key == GLFW_KEY_G)
		{
			if (flag == 2 || flag == 0) {
				if (xr_rotate)
					xr_stop_time = float(glfwGetTime());
				else
					xr_blank_time += (float(glfwGetTime()) - xr_stop_time);
				xr_rotate = !xr_rotate;
			}
			else
				printf("> Stop rotation first!(Press '%c' to stop)\n", return_char(flag));
		}
		else if (key == GLFW_KEY_V)
		{
			if (flag == 3 || flag == 0) {
				if (yf_rotate)
					yf_stop_time = float(glfwGetTime());
				else
					yf_blank_time += (float(glfwGetTime()) - yf_stop_time);
				yf_rotate = !yf_rotate;
			}
			else
				printf("> Stop rotation first!(Press '%c' to stop)\n", return_char(flag));
		}
		else if (key == GLFW_KEY_B)
		{
			if (flag == 4 || flag == 0) {
				if (yr_rotate)
					yr_stop_time = float(glfwGetTime());
				else
					yr_blank_time += (float(glfwGetTime()) - yr_stop_time);
				yr_rotate = !yr_rotate;
			}
			else
				printf("> Stop rotation first!(Press '%c' to stop)\n", return_char(flag));
		}
		else if (key == GLFW_KEY_D)
		{
			visualization = (visualization + 1) % 3;
			if (visualization == 0)
				printf("> using (textcoord.xy, 0) as color\n");
			else if(visualization == 1)
				printf("> using (textcoord.xxx) as color\n");
			else if (visualization == 2)
				printf("> using (textcoord.yyy) as color\n");
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
	else if (action == GLFW_RELEASE)
	{
	if (key == GLFW_KEY_KP_ADD || (key == GLFW_KEY_EQUAL && (mods & GLFW_MOD_SHIFT)))	b.add = false;
	else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS) b.sub = false;
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS )
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		printf( "> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y) );
	}
}

void motion( GLFWwindow* window, double x, double y )
{
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
