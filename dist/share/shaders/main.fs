#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4      u_pvm_mat;
layout(location = 1) uniform mat4      u_i_pvm_mat;

layout(location = 9) uniform float     u_slice;

layout(binding  = 0) uniform sampler3D u_df;

in  vec3 f_rd; // ray dir
in  vec3 f_ro; // ray origin

out vec4 o_color;

float trace(vec3 ro, vec3 rd, int n) {
	float d_=0, d=0.0;
	for(int i=0; i<n; ++i, d += 1.02*d_) {
		vec3 s = ro + d * rd;
		d_ = texture(u_df, s+vec3(.5,.5,.5)).r;
		if (d_ < 0.01) {
			break;
		}
	}
	return d;
}

void main() {
	vec3 ro = (u_i_pvm_mat * vec4(f_ro, 1)).xyz;
	vec3 rd = (u_i_pvm_mat * vec4(f_rd, 0)).xyz;
	float d = trace(ro, rd, 64);
	o_color = vec4(d, d, d, 1);
}
