// vertex attributes
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

// matrices
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 norm;
out vec2 tc;


void main()
{
	vec4 pos_in_hc = vec4(position, 1);//local frame
	vec4 wpos = model_matrix * pos_in_hc;//world frame
	vec4 epos = view_matrix * wpos;//eye-space frame
	gl_Position = projection_matrix * epos;//NDC or canonical view volume [-1,1]

	norm = normal;
	tc = texcoord;
}
