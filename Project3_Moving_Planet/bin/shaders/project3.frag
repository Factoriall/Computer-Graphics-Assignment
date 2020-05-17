#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
precision highp float; // default precision needs to be defined
#endif

// input
in vec3 norm;
in vec2 tc;

// input from vertex shader
uniform bool b_solid_color;
uniform vec4 solid_color;

// the only output variable
out vec4 fragColor;

void main()
{
	fragColor = b_solid_color ? solid_color : vec4(tc.xy, 0, 1);
}
