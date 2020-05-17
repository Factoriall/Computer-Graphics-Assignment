#pragma once
#ifndef __PLANET_H__
#define __PLANET_H__

struct planet_t
{
	float	revRadius = 0.0f;			//revolution radius from sun
	float	planetRadius = 1.0f;		//radius of planet
	float	rotSpeed = 1.0f;			//rotation speed
	float	revSpeed = 1.0f;			//revolution speed
	vec3	rgb = vec3(0);				//color of planet 
	mat4	model_matrix;				//modeling transformation
};


planet_t sun = { 0, 15, 1, 0, {1, 1, 0} };
planet_t mercury = { 20, 1, 0.5, 0.5, {211.0f / 255.0f, 211.0f / 255.0f, 211.0f / 255.0f} };
planet_t venus = { 25, 2, 2, 1, {1, 215.0f/255.0f, 0} };
planet_t earth = { 29, 2, 3, 1.5, {0, 1, 0} };
planet_t mars = { 35, 1.5f, 4, 2.5, {1, 0, 0} };
planet_t jupiter = { 50, 8, 6, 3, {1, 2.0f / 3.0f, 0} };
planet_t saturn = { 70, 9, 4, 4.5, {165.0f / 255.0f, 42.0f / 255.0f, 42.0f / 255.0f} };
planet_t uranus = {80, 3, 3, 3.5, {13.0f / 255.0f, 152.0f / 255.0f, 186.0f / 255.0f} };
planet_t neptune = { 90, 3, 2, 4, {0,0,1} };


inline std::vector<planet_t> create_planets()
{
	std::vector<planet_t> planets;

	planet_t p;

	planets.emplace_back(sun);
	planets.emplace_back(mercury);
	planets.emplace_back(venus);
	planets.emplace_back(earth);
	planets.emplace_back(mars);
	planets.emplace_back(jupiter);
	planets.emplace_back(saturn);
	planets.emplace_back(uranus);
	planets.emplace_back(neptune);

	return planets;
}

#endif