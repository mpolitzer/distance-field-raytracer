#version 330 core
#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_shading_language_420pack  : enable

layout(location = 0) uniform mat4      u_t_pvm_mat;
layout(location = 1) uniform mat4      u_i_pvm_mat;

layout(location = 9) uniform float     u_slice;

layout(binding  = 0) uniform sampler3D u_df;

layout(location = 0) in      vec2      v_pos;

out vec3 f_rd; // ray dir
out vec3 f_ro; // ray origin

void main() {
	gl_Position = vec4(v_pos,0,1);

	//f_ro = (-vec4(u_pvm_mat[3].xyz,1)*u_pvm_mat).xyz;
	//f_rd = ( vec4(v_pos,-1,0)        *u_pvm_mat).xyz;

	f_ro = vec3(0,0,0);
	f_rd = vec3(v_pos,-1);
}

