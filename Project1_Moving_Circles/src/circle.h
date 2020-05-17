#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__

struct circle_t
{
	vec2	center=vec2(0);		// 2D position for translation
	float	radius=1.0f;		// radius
	float	velocity = 1.0f;
	float	theta = 0.0f;		//movement direction 
	vec4	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update();	
};

float getDistance(vec2 a, vec2 b) {
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

inline std::vector<circle_t> create_circles(uint N)
{
	std::vector<circle_t> circles;
	
	circle_t c;
	srand((uint)time(NULL));

	for (uint i = 0; i < N; i++) {
		//set the radius, position, velocity, angle, and color randomly 
		float r = (float(rand()) / float(RAND_MAX) * sqrt(3.0f / (N * PI))) + sqrt(1.0f / (N * PI));
		float x = (float(rand()) / float(RAND_MAX) * (3.0f - 2 * r)) - (1.5f - r);
		float y = (float(rand()) / float(RAND_MAX) * (2.0f - 2 * r)) - (1.0f - r);
		float v = float(rand()) / float(RAND_MAX) * 0.002f;
		float theta = float(rand()) / float(RAND_MAX) * PI * 2;
		float red = float(rand()) / float(RAND_MAX);
		float green = float(rand()) / float(RAND_MAX);
		float blue = float(rand()) / float(RAND_MAX);
		
		c = { vec2(x, y), r, v, theta, vec4(red, green, blue, 1.0f) };

		bool flag = false;

		//set the circle not to be overlapped 
		for (auto& circle : circles) {
			if (getDistance(circle.center, c.center) < circle.radius + c.radius) {
				flag = true;
				break;
			}
		}
		if (flag) {
			i--;
			continue;
		}
			
		circles.emplace_back(c);
	}
	
	return circles;
}

inline void circle_t::update()
{
	float c	= cos(theta), s=sin(theta);
	center.x = center.x + velocity * c;
	center.y = center.y + velocity * s;

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	
	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

#endif
