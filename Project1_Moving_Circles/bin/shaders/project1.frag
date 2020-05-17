#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif
// output of the fragment shader
out vec4 fragColor;

// shader's global variables, called the uniform variables
uniform vec4 solid_color;

void main()
{
	//fragColor = b_solid_color ? solid_color : vec4(vec2(tc), 0, 1);
	fragColor = solid_color;
}