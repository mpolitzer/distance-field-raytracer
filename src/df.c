#include <math.h>
#include <stdlib.h>
#include "df.h"

void df_build(df *o,
              ivec3 n, float *p,      // canvas / output buffer
              vec3 center, vec3 half, // AABB
              is_gen fn, void *arg)   // implicit surface generator
{
	o->p = p;
	o->n = n;
	o->center = center;
	o->half = half;

	for (int k=0; k<n.z; ++k) {
		for (int j=0; j<n.y; ++j) {
			for (int i=0; i<n.x; ++i) {
				int index = k*n.z*n.y + j*n.y + i;
				float x = center.x + (i-n.x/2)/(half.x*n.x),
				      y = center.y + (j-n.y/2)/(half.y*n.y),
				      z = center.z + (k-n.z/2)/(half.z*n.z);
				p[index] = fn(x, y, z, arg);
			}
		}
	}
}

float df_value_at(const df *in, float x, float y, float z)
{
	vec3 p   = vec3_make(x, y, z);
	vec3 n   = vec3_make(in->n.x, in->n.y, in->n.z);
	vec3 c   = in->center;
	vec3 h   = in->half;
	vec3 pc  = vec3_sub(p, c);

	if (vec3_gt(vec3_abs(pc), h))
		return -NAN;

	vec3 nr = vec3_div(pc, h);
	int i = nr.x * in->n.x + in->n.x/2;
	int j = nr.y * in->n.y + in->n.y/2;
	int k = nr.z * in->n.z + in->n.z/2;
	int index = k*n.z*n.y + j*n.y + i;
	return in->p[index];
}

static float gen_is_sphere(float x, float y, float z, void *arg)
{
	return sqrt(x*x + y*y + z*z)-*(float *)arg;
}

void df_build_sphere(df *o, float x, float y, float z, float r)
{
	//vec3  half = vec3_make(.5, .5, .5);
	vec3  half = vec3_make(1, 1, 1);

	int   sq   = 128;
	ivec3 n    =ivec3_make(sq, sq, sq);

	df_build(o,
	         ivec3_make(sq, sq, sq),
	         malloc(sizeof(float)*n.x*n.y*n.z),
	         vec3_make(x,y,z), half,
	         gen_is_sphere, &r);
}

// GLSL
//float vmax(vec3 v) {
//	return max(max(v.x, v.y), v.z);
//}
//
//float box(vec3 p, vec3 b) {
//	vec3 d = abs(p) - b;
//	return length(max(d, vec3(0))) + vmax(min(d, vec3(0)));
//}

static float gen_is_box(float x, float y, float z, void *arg)
{
	vec3 p    = vec3_make(x, y, z);
	vec3 zero = vec3_make(0, 0, 0);
	vec3 d    = vec3_sub(vec3_abs(p), *(vec3 *)arg);
	return vec3_length(vec3_max(d, zero)) + vec3_hmax(vec3_min(d, zero));
}

void df_build_box(df *o, float x, float y, float z,
                         float w, float h, float d)
{
	vec3 center = vec3_make(x,y,z),
	     half   = vec3_make(1,1,1),
	     dim    = vec3_make(w,h,d);

	int   sq = 128;
	ivec3 n  = ivec3_make(sq, sq, sq);

	df_build(o, n, malloc(sizeof(float)*n.x*n.y*n.z),
	         center, half,
	         gen_is_box, &dim);
}

/* -------------------------------------------------------------------------- */

struct df_pair_t { const df *a, *b; };
static float gen_df_unite(float x, float y, float z, void *arg)
{
	struct df_pair_t *pair = (struct df_pair_t *)arg;
	return fmin(
			df_value_at(pair->a, x, y, z),
			df_value_at(pair->b, x, y, z));
}

void df_unite(df *o, const df *a, const df *b)
{
	struct df_pair_t pair = { a, b };
	int   sq = 128;
	ivec3 n  =ivec3_make(sq, sq, sq);

	vec3 center = vec3_divs(vec3_add(a->center, b->center), 2);
	vec3 half   = vec3_max(a->half, b->half);

	df_build(o, n, malloc(sizeof(float)*n.x*n.y*n.z),
	         center, half,
	         gen_df_unite, &pair);
}

static float gen_df_unite_chamfer(float x, float y, float z, void *arg)
{
	struct df_pair_t *pair = (struct df_pair_t *)arg;
	float a = df_value_at(pair->a, x, y, z),
	      b = df_value_at(pair->b, x, y, z),
	      r = 0.05;
	return fmin(fmin(a, b), (a - r + b)*sqrt(0.5));
}

void df_unite_chamfer(df *o, const df *a, const df *b)
{
	struct df_pair_t pair = { a, b };
	int   sq = 128;
	ivec3 n  =ivec3_make(sq, sq, sq);

	vec3 center = vec3_divs(vec3_add(a->center, b->center), 2);
	vec3 half   = vec3_max(a->half, b->half);

	df_build(o, n, malloc(sizeof(float)*n.x*n.y*n.z),
	         center, half,
	         gen_df_unite_chamfer, &pair);
}

static float gen_df_unite_rounded(float x, float y, float z, void *arg)
{
	struct df_pair_t *pair = (struct df_pair_t *)arg;

	float r = 0.1;
	float a = df_value_at(pair->a, x, y, z),
	      b = df_value_at(pair->b, x, y, z);

	float ux= fmax(r-a, 0);
	float uy= fmax(r-b, 0);

	return fmax(r, fmin(a,b)) - sqrtf(ux*ux + uy*uy);
}

void df_unite_rounded(df *o, const df *a, const df *b)
{
	struct df_pair_t pair = { a, b };
	int   sq = 128;
	ivec3 n  =ivec3_make(sq, sq, sq);

	vec3 center = vec3_divs(vec3_add(a->center, b->center), 2);
	vec3 half   = vec3_max(a->half, b->half);

	df_build(o, n, malloc(sizeof(float)*n.x*n.y*n.z),
	         center, half,
	         gen_df_unite_rounded, &pair);
}

static float gen_df_intersect(float x, float y, float z, void *arg)
{
	struct df_pair_t *pair = (struct df_pair_t *)arg;
	return fmax(
			df_value_at(pair->a, x, y, z),
			df_value_at(pair->b, x, y, z));
}

void df_intersect(df *o, const df *a, const df *b)
{
	struct df_pair_t pair = { a, b };
	int   sq = 128;
	ivec3 n  =ivec3_make(sq, sq, sq);

	vec3 center = vec3_divs(vec3_add(a->center, b->center), 2);
	vec3 half   = vec3_max(a->half, b->half);

	df_build(o, n, malloc(sizeof(float)*n.x*n.y*n.z),
	         center, half,
	         gen_df_intersect, &pair);
}

static float gen_df_subtract(float x, float y, float z, void *arg)
{
	struct df_pair_t *pair = (struct df_pair_t *)arg;
	return fmax(     df_value_at(pair->a, x, y, z),
	                -df_value_at(pair->b, x, y, z));
}

void df_subtract (df *o, const df *a, const df *b)
{
	struct df_pair_t pair = { a, b };
	int   sq = 128;
	ivec3 n  =ivec3_make(sq, sq, sq);

	vec3 center = vec3_divs(vec3_add(a->center, b->center), 2);
	vec3 half   = vec3_max(a->half, b->half);

	df_build(o, n, malloc(sizeof(float)*n.x*n.y*n.z),
	         center, half,
	         gen_df_subtract, &pair);
}

