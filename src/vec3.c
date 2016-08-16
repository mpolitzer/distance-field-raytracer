#include <math.h>
#include "vec3.h"

vec3 vec3_make(float x, float y, float z)
{
	return (vec3){ x, y, z };
}

vec3 vec3_add(vec3 u, vec3 v)
{
	return (vec3){ u.x + v.x, u.y + v.y, u.z + v.z };
}

vec3 vec3_sub(vec3 u, vec3 v)
{
	return (vec3){ u.x - v.x, u.y - v.y, u.z - v.z };
}

vec3 vec3_mul(vec3 u, vec3 v)
{
	return (vec3){ u.x * v.x, u.y * v.y, u.z * v.z };
}

vec3 vec3_div(vec3 u, vec3 v)
{
	return (vec3){ u.x / v.x, u.y / v.y, u.z / v.z };
}

vec3 vec3_divs(vec3 u, float s)
{
	return (vec3){ u.x/s, u.y/s, u.z/s };
}


bool vec3_gt(vec3 u, vec3 v)
{
	if (u.x > v.x) return true;
	if (u.y > v.y) return true;
	if (u.z > v.z) return true;
	return false;
}

vec3 vec3_max(vec3 u, vec3 v)
{
	return vec3_make(fmax(u.x, v.x), fmax(u.y, v.y), fmax(u.z, v.z));
}

vec3 vec3_min(vec3 u, vec3 v)
{
	return vec3_make(fmin(u.x, v.x), fmin(u.y, v.y), fmin(u.z, v.z));
}

float vec3_hmax(vec3 u)
{
	return fmax(fmax(u.x, u.y), u.z);
}

float vec3_length(vec3 u)
{
	return sqrt(u.x*u.x + u.y*u.y + u.z*u.z);
}

vec3 vec3_abs(vec3 u)
{
	return (vec3){ fabs(u.x), fabs(u.y), fabs(u.z) };
}

/* -------------------------------------------------------------------------- */

ivec3 ivec3_make(int x, int y, int z)
{
	return (ivec3){x, y, z};
}
