#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4      u_t_pvm_mat;
layout(location = 1) uniform mat4      u_i_pvm_mat;

layout(location = 9) uniform float     u_slice; // UNUSED

layout(binding  = 0) uniform sampler3D u_df;

in  vec3 f_rd; // ray dir
in  vec3 f_ro; // ray origin

out vec4 o_color;

float vmax(vec2 v) {
	return max(v.x, v.y);
}

float vmax(vec3 v) {
	return max(max(v.x, v.y), v.z);
}

float vmax(vec4 v) {
	return max(max(v.x, v.y), max(v.z, v.w));
}

float box(vec3 p, vec3 b) {
	vec3 d = abs(p) - b;
	return length(max(d, vec3(0))) + vmax(min(d, vec3(0)));
}

float scene(vec3 p) {
	vec3 canvas = vec3(.5,.5,.5);
	return max(box(p, canvas), texture(u_df, p+.5).r);
}

float trace(vec3 ro, vec3 rd, int n) {
	float d_=0, d=0.0;
	for(int i=0; i<n; ++i, d += d_) {
		vec3 s = ro + d * rd;
		d_ = scene(s);
		if (d_ < 0.01) {
			break;
		}
	}
	return d;
}

/* sample around p to calculate the normal */
vec3 normal(vec3 p) {
	float e = 1e-1;
	vec3 n = vec3(
			scene(p+vec3(e,0,0))-scene(p-vec3(e,0,0)),
			scene(p+vec3(0,e,0))-scene(p-vec3(0,e,0)),
			scene(p+vec3(0,0,e))-scene(p-vec3(0,0,e)));
	return normalize(n);
}

void main() {
	vec3 ro = (u_i_pvm_mat * vec4(f_ro, 1)).xyz;
	vec3 rd = (u_t_pvm_mat * vec4(f_rd, 0)).xyz;
	float d = trace(ro, rd, 32);
	vec3  p = ro + d*rd;
	vec3  n = normal(p);
	//if (d < .9) {
	//	o_color = vec4(n, 1);
	//} else {
	//	o_color = vec4(0,0,0, 0);
	//}

	//o_color = vec4(d, d, d, 1);
	o_color = vec4(n, 1);
}
