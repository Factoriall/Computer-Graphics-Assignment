// vertex attributes
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

// matrices
uniform mat4 model_matrix;
uniform mat4 view_projection_matrix;
uniform mat4 aspect_matrix;

//out vec3 norm;
out vec2 tc;

void main()
{
	vec4 pos_in_hc = vec4(position, 1);//local frame
	vec4 wpos = model_matrix * pos_in_hc;//world frame
	gl_Position = aspect_matrix * view_projection_matrix * wpos;//NDC or canonical view volume [-1,1]

	// pass eye-coordinate normal to fragment shader
	tc = texcoord;
}
