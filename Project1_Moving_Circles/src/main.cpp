#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility
#include "circle.h"		// circle class definition

//*************************************
// global constants
static const char*	window_name = "Project1 - Moving circles";
static const char*	vert_shader_path = "../bin/shaders/project1.vert";
static const char*	frag_shader_path = "../bin/shaders/project1.frag";
static const uint	MIN_CIRCLE = 20;	// minimum number of circle
static const uint	MAX_CIRCLE = 512;	// maximum number of circle
uint				NUM_CIRCLE = 25;	// initial number of circle
uint				NUM_TESS = 36;		// initial tessellation factor of the circle as a polygon

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint	program = 0;		// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

//*************************************
// global variables
int		frame = 0;						// index of rendering frames
float	t = 0.0f;						// current simulation parameter
bool	b_solid_color = false;			// use circle's color?
bool	b_index_buffer = true;			// use index buffering?
#ifndef GL_ES_VERSION_2_0
bool	b_wireframe = false;
#endif
auto	circles = std::move(create_circles(NUM_CIRCLE));
struct { 
	bool add=false, sub=false; 
	operator bool() const { return add||sub; } 
} b; // flags of keys for smooth changes

//*************************************
// holder of vertices and indices of a unit circle
std::vector<vertex>	unit_circle_vertices;	// host-side vertices

//function to change the angle of circle if it hits the wall
int hitTheWall(circle_t a) {
	float x = a.center.x;
	float y = a.center.y;
	float r = a.radius;
	float t = a.theta;

	if ((x + r >= 1.5f && (t < PI * 0.5f || t > PI * 1.5f)) || (x - r <= -1.5f && (t > PI * 0.5f && t < PI * 1.5f)))
		return 1;
	else if ((y - r <= -1.0f && t > PI) || (y + r >= 1.0f && t < PI))
		return 2;
	return 0;
}

//function to see whether two circles are collided or not
bool isCollided(circle_t c1, circle_t c2) {
	//position of circle that will be moved in the future if there's no change
	vec2 c1_future = vec2(c1.center.x + c1.velocity * cos(c1.theta), c1.center.y + c1.velocity * sin(c1.theta));
	vec2 c2_future = vec2(c2.center.x + c2.velocity * cos(c2.theta), c2.center.y + c2.velocity * sin(c2.theta));
	if(getDistance(c1.center, c2.center) <= c1.radius + c2.radius && getDistance(c1.center, c2.center) > getDistance(c1_future, c2_future))
		return true;
	else
		return false;
}

//function of elastic collision
void elasticCollision(circle_t &c1, circle_t &c2) {
	float v1 = c1.velocity; float v2 = c2.velocity;									//velocity
	float t1 = c1.theta; float t2 = c2.theta;										//angle
	float m1 = c1.radius * c1.radius * PI; float m2 = c2.radius * c2.radius * PI;	//mass
	float phi = atan2(c1.center.y - c2.center.y, c1.center.x - c2.center.x);		//contact angle

	//calculation of elastic collision
	float v1x_dot = (v1 * cos(t1 - phi) * (m1 - m2) + 2 * m2 * v2 * cos(t2 - phi)) / (m1 + m2) * cos(phi) 
						+ v1 * sin(t1 - phi) * cos(phi + PI / 2);
	float v1y_dot = (v1 * cos(t1 - phi) * (m1 - m2) + 2 * m2 * v2 * cos(t2 - phi)) / (m1 + m2) * sin(phi)
						+ v1 * sin(t1 - phi) * sin(phi + PI / 2);
	float v2x_dot = (v2 * cos(t2 - phi) * (m2 - m1) + 2 * m1 * v1 * cos(t1 - phi)) / (m1 + m2) * cos(phi)
						+ v2 * sin(t2 - phi) * cos(phi + PI / 2);
	float v2y_dot = (v2 * cos(t2 - phi) * (m2 - m1) + 2 * m1 * v1 * cos(t1 - phi)) / (m1 + m2) * sin(phi)
						+ v2 * sin(t2 - phi) * sin(phi + PI / 2);

	c1.theta = atan2(v1y_dot, v1x_dot);
	c1.velocity = sqrt(v1x_dot * v1x_dot + v1y_dot * v1y_dot);

	c2.theta = atan2(v2y_dot, v2x_dot);
	c2.velocity = sqrt(v2x_dot * v2x_dot + v2y_dot * v2y_dot);
}



//*************************************
void update()
{
	// update global simulation parameter
	t = float(glfwGetTime())*0.4f;

	// tricky aspect correction matrix for non-square window
	float aspect = window_size.x/float(window_size.y);
	mat4 aspect_matrix = 
	{
		min(1/aspect,1.0f), 0, 0, 0,
		0, min(aspect,1.0f), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	// update common uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "b_solid_color" );	if(uloc>-1) glUniform1i( uloc, b_solid_color );
	uloc = glGetUniformLocation( program, "aspect_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, aspect_matrix );

	//void update_circles();
	//if (b) update_circles();
	// update vertex buffer by the pressed keys
	//void update_tess(); // forward declaration
	//if(b) update_tess(); 
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// notify GL that we use our own program
	glUseProgram( program );

	// bind vertex array object
	glBindVertexArray( vertex_array );

	// render circles: trigger shader program to process vertex data
	for(auto& c : circles)
	{
		// per-circle update
		c.update();

		// set the theta between 0 and 2 * PI
		if (c.theta >= 2 * PI) {
			while (c.theta >= 2 * PI)
				c.theta -= 2 * PI;
		}
		if (c.theta < 0.0f) {
			while (c.theta < 0.0f)
				c.theta += 2 * PI;
		}
		
		// change the angle of circle if it hits the wall
		if (hitTheWall(c)) {
			switch (hitTheWall(c)) {
			case 1: //right, left
				if (c.theta < PI)
					c.theta = PI - c.theta;
				else
					c.theta = 3 * PI - c.theta;
				break;
			case 2: // up, down
				c.theta = 2 * PI - c.theta;
			}
		}	
		
		// calculate and change the angle and velocity if two circles are collided
		for (auto& tmp : circles) {
			if (tmp.center == c.center)//except the same circle
				continue;

			if (isCollided(c, tmp)) {
				elasticCollision(c, tmp);
			}
		}

		// update per-circle uniforms
		GLint uloc;
		uloc = glGetUniformLocation( program, "solid_color" );		if(uloc>-1) glUniform4fv( uloc, 1, c.color );	// pointer version
		uloc = glGetUniformLocation( program, "model_matrix" );		if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, c.model_matrix );

		// per-circle draw calls
		if(b_index_buffer)	glDrawElements( GL_TRIANGLES, NUM_TESS*3, GL_UNSIGNED_INT, nullptr );
		else				glDrawArrays( GL_TRIANGLES, 0, NUM_TESS*3 ); // NUM_TESS = N, Simple Vertex Buffering 때 사용
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
	printf( "- press '+/-' to increase/decrease circle number (min=%d, max=%d)\n", MIN_CIRCLE, MAX_CIRCLE );
	printf( "- press 'i' to toggle between index buffering and simple vertex buffering\n" );
#ifndef GL_ES_VERSION_2_0
	printf( "- press 'w' to toggle wireframe\n" );
#endif
	printf( "\n" );
}

std::vector<vertex> create_circle_vertices( uint N )
{
	std::vector<vertex> v = {{ vec3(0), vec3(0,0,-1.0f), vec2(0.5f) }}; // origin
	for( uint k=0; k <= N; k++ )
	{
		float t=PI*2.0f*k/float(N), c=cos(t), s=sin(t);
		v.push_back( { vec3(c,s,0), vec3(0,0,-1.0f), vec2(c,s)*0.5f+0.5f } );
	}
	return v;
}

void update_vertex_buffer( const std::vector<vertex>& vertices, uint N )
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	// clear and create new buffers
	if(vertex_buffer)	glDeleteBuffers( 1, &vertex_buffer );	vertex_buffer = 0;
	if(index_buffer)	glDeleteBuffers( 1, &index_buffer );	index_buffer = 0;

	// check exceptions
	if(vertices.empty()){ printf("[error] vertices is empty.\n"); return; }

	// create buffers
	if(b_index_buffer)//vertex + index
	{
		std::vector<uint> indices;
		for( uint k=0; k < N; k++ ) // front-facing을 위해 0,1,2 순서대로, 반대로 할시 glCullFace로 인해 출력되지 않는다
		{
			indices.push_back(0);	// the origin
			indices.push_back(k+1);
			indices.push_back(k+2);
		}

		// generation of vertex buffer: use vertices as it is
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

		// geneation of index buffer
		//GL_ELEMENT_ARRAY_BUFFER == INDEX_BUFFER
		glGenBuffers( 1, &index_buffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*indices.size(), &indices[0], GL_STATIC_DRAW );
	}
	else//simple vertex buffering
	{
		std::vector<vertex> v; // triangle vertices
		for( uint k=0; k < N; k++ )
		{
			v.push_back(vertices.front());	// the origin
			v.push_back(vertices[k+1]);
			v.push_back(vertices[k+2]);
		}

		// generation of vertex buffer: use triangle_vertices instead of vertices
		glGenBuffers( 1, &vertex_buffer );
		glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*v.size(), &v[0], GL_STATIC_DRAW );
	}

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if(vertex_array) glDeleteVertexArrays(1,&vertex_array);
	vertex_array = cg_create_vertex_array( vertex_buffer, index_buffer );
	if(!vertex_array){ printf("%s(): failed to create vertex aray\n",__func__); return; }
}

void update_circles()
{
	uint n = NUM_CIRCLE; if(b.add) n++; if(b.sub) n--;
	if(n==NUM_CIRCLE||n<MIN_CIRCLE||n>MAX_CIRCLE) return;
	
	circles = std::move(create_circles(NUM_CIRCLE = n));
	printf( "> NUM_CIRCLE = % -4d\r", NUM_CIRCLE );
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_KP_ADD || (key == GLFW_KEY_EQUAL && (mods & GLFW_MOD_SHIFT))) {
			b.add = true;
			update_circles();
		}
		else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS) {
			b.sub = true;
			update_circles();
		}
		else if(key==GLFW_KEY_I)
		{
			b_index_buffer = !b_index_buffer;
			update_vertex_buffer( unit_circle_vertices,NUM_TESS );
			printf( "> using %s buffering\n", b_index_buffer?"index":"vertex" );
		}
#ifndef GL_ES_VERSION_2_0
		else if(key==GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode( GL_FRONT_AND_BACK, b_wireframe ? GL_LINE:GL_FILL );
			printf( "> using %s mode\n", b_wireframe ? "wireframe" : "solid" );
		}
#endif
	}
	else if(action==GLFW_RELEASE)
	{
		if(key==GLFW_KEY_KP_ADD||(key==GLFW_KEY_EQUAL&&(mods&GLFW_MOD_SHIFT)))	b.add = false;
		else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS) b.sub = false;
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
	glLineWidth( 1.0f );
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	
	// define the position of four corner vertices
	unit_circle_vertices = std::move(create_circle_vertices( NUM_TESS ));

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer( unit_circle_vertices, NUM_TESS );

	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

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
