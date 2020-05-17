#ifndef __TRACKBALL_H__
#define __TRACKBALL_H__
#include "cgmath.h"

struct trackball
{
	bool	b_tracking = false;
	float	scale;			// controls how much rotation is applied
	float	move;			// controls the movement of panning and zooming
	mat4	view_matrix0;	// initial view matrix
	vec2	m0;				// the last mouse position
	int		button = 0;
	int		mods = 0;

	trackball(float rot_scale = 1.0f, float move_scale = 100.0f) : scale(rot_scale), move(move_scale) {}
	bool is_tracking() const { return b_tracking; }
	void begin(const mat4& view_matrix, vec2 m);
	void end() { b_tracking = false; }
	mat4 update(vec2 m) const;
	mat4 update_pan(vec2 m) const;
	mat4 update_zoom(vec2 m) const;
};

struct camera
{
	vec3	eye = vec3(0, 100, 200);//camera 위치
	vec3	at = vec3(0, 0, 0);//target
	vec3	up = vec3(0, 1, 0);//다른 베이스
	mat4	view_matrix = mat4::look_at(eye, at, up);

	float	fovy = PI / 4.0f; // must be in radian
	float	aspect_ratio = 0.0f;
	float	dNear = 1.0f;
	float	dFar = 1000.0f;
	mat4	projection_matrix;
};

camera	cam;

inline void trackball::begin(const mat4& view_matrix, vec2 m)
{
	b_tracking = true;			// enable trackball tracking
	m0 = m;			  			// save current mouse position
	view_matrix0 = view_matrix;	// save current view matrix
}

inline mat4 trackball::update(vec2 m) const
{
	// project a 2D mouse position to a unit sphere
	static const vec3 p0 = vec3(0, 0, 1.0f);	// reference position on sphere
	vec3 p1 = vec3(m - m0, 0);					// displacement
	if (!b_tracking || length(p1) < 0.0001f) return view_matrix0;		// ignore subtle movement
	p1 *= scale;														// apply rotation scale
	p1 = vec3(p1.x, p1.y, sqrtf(max(0, 1.0f - length2(p1)))).normalize();	// back-project z=0 onto the unit sphere

	// find rotation axis and angle in world space
	// - trackball self-rotation should be done at first in the world space
	// - mat3(view_matrix0): rotation-only view matrix
	// - mat3(view_matrix0).transpose(): inverse view-to-world matrix
	vec3 v = mat3(view_matrix0).transpose() * p0.cross(p1);
	float theta = asin(min(v.length(), 1.0f));

	// resulting view matrix, which first applies
	// trackball rotation in the world space
	return view_matrix0 * mat4::rotate(v.normalize(), theta);
}

inline mat4 trackball::update_pan(vec2 m) const
{
	// project a 2D mouse position to a unit sphere
	//static const vec3 p0 = vec3(0);	// reference position on sphere
	vec3 u = vec3(view_matrix0[0], view_matrix0[1], view_matrix0[2]);
	vec3 v = vec3(view_matrix0[4], view_matrix0[5], view_matrix0[6]);
	vec2 dis = vec2(m - m0) * move;										// displacement
	if (!b_tracking || length(dis) < 0.0001f) return view_matrix0;		// ignore subtle movement												

	// resulting view matrix, which first applies
	// trackball rotation in the world space
	return view_matrix0 * mat4::translate(u * dis.x + v * dis.y);
}

inline mat4 trackball::update_zoom(vec2 m) const
{
	vec3 n = vec3(view_matrix0[8], view_matrix0[9], view_matrix0[10]);
	float dis = (m - m0).y * move;
	if (!b_tracking || abs(dis) < 0.0001f) return view_matrix0;			// ignore subtle movement

	return view_matrix0 * mat4::translate(n * dis);
}

// utility function
inline vec2 cursor_to_ndc(dvec2 cursor, ivec2 window_size)
{
	// normalize window pos to [0,1]^2
	vec2 npos = vec2(float(cursor.x) / float(window_size.x - 1),
		float(cursor.y) / float(window_size.y - 1));

	// normalize window pos to [-1,1]^2 with vertical flipping
	// vertical flipping: window coordinate system defines y from
	// top to bottom, while the trackball from bottom to top
	return vec2(npos.x * 2.0f - 1.0f, 1.0f - npos.y * 2.0f);
}

#endif // __TRACKBALL_H__
