#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable
#extension GL_ARB_arrays_of_arrays          : enable

/* TODO: over-relaxation in the trace function. */
#define LIMIT (1./256)
#define MAX_ITERATIONS 64

layout(location = 0) uniform mat4      u_t_pvm_mat;
layout(location = 1) uniform mat4      u_i_pvm_mat;

layout(location = 9) uniform float     u_slice; // UNUSED

layout(binding  = 0) uniform sampler3D u_df;

in  vec3 f_rd; // ray dir
in  vec3 f_ro; // ray origin

out vec4 o_color;

float vmax(vec3 v) {
	return max(max(v.x, v.y), v.z);
}

float box(vec3 p, vec3 b) {
	vec3 d = abs(p) - b;
	return length(max(d, vec3(0))) + vmax(min(d, vec3(0)));
}

float combine(float u, float v, float r) {
	float m = min(u,v);
	if (u < r && v < r) {
		return min(m, u+v-r);
	}
	return m;
}

float scene(vec3 p) {
	vec3 canvas = vec3(.5, .5, .5);
	return max(box(p, canvas), texture(u_df, p+.5).r);
}

float trace(vec3 ro, vec3 rd, int n, out float off) {
	float radius=0.0, d=0.0;
	for(int i=0; i<n; ++i, d += 1.02*radius) {
		vec3 s = ro + d * rd;
		radius = scene(ro+d*rd);
		if (radius < LIMIT) {
			break;
		}
	}
	off = radius;
	return d;
}

float sobel_kernel_x[3][3][3] = {
	{	{-1,  -3,  -1},
		{-3,  -6,  -3},
		{-1,  -3,  -1}},

	{	{ 0,   0,   0},
		{ 0,   0,   0},
		{ 0,   0,   0}},

	{	{+1,  +3,  +1},
		{+3,  +6,  +3},
		{+1,  +3,  +1}}};

float sobel_kernel_y[3][3][3] = {
	{	{ 1,  3,  1},
		{ 0,  0,  0},
		{-1, -3, -1}},

	{	{ 3,  6,  3},
		{ 0,  0,  0},
		{-3, -6, -3}},

	{	{ 1,  3,  1},
		{ 0,  0,  0},
		{-1, -3, -1}}};

float sobel_kernel_z[3][3][3] = {
	{	{-1,  0,  1},
		{-3,  0,  3},
		{-1,  0,  1}},

	{	{-3,  0,  3},
		{-6,  0,  6},
		{-3,  0,  3}},

	{	{-1,  0,  1},
		{-3,  0,  3},
		{-1,  0,  1}}};

vec3 sobel_normalization(vec3 p)
{
	float e = 1./64;
	vec3 n = vec3(0);

	for (int x=-1; x<=1; ++x) {
		for (int y=-1; y<=1; ++y) {
			for (int z=-1; z<=1; ++z) {
				float df = scene(p + vec3(e*x,e*y,e*z));
				n.x += df*sobel_kernel_x[+x+1][-y+1][+z+1];
				n.y += df*sobel_kernel_y[+x+1][-y+1][+z+1];
				n.z += df*sobel_kernel_z[+x+1][-y+1][+z+1];
			}
		}
	}

	return normalize(n);
}

/* sample around p to calculate the normal */
vec3 normal(vec3 p) {
	float e = 1./64;
	vec3 n = vec3(
			scene(p+vec3(e,0,0))-scene(p-vec3(e,0,0)),
			scene(p+vec3(0,e,0))-scene(p-vec3(0,e,0)),
			scene(p+vec3(0,0,e))-scene(p-vec3(0,0,e)));
	return normalize(n);
}

void main() {
	vec3 ro = (u_i_pvm_mat * vec4(f_ro, 1)).xyz;
	vec3 rd = (u_t_pvm_mat * vec4(f_rd, 0)).xyz;
	float f;
	float d = trace(ro, rd, MAX_ITERATIONS, f);
	vec3  p = ro + d*rd;
	//vec3  n = normal(p);
	vec3  n = sobel_normalization(p);

	if (f <= LIMIT) {
		o_color = vec4(n, 1);
	} else {
		o_color = vec4(.1,0,.4,1);
	}
}
