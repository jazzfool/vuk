// file generated by vush compiler, from ../../examples/test.vush
#version 460
#pragma shader_stage(vertex)
#extension GL_GOOGLE_cpp_style_line_directive : require
#extension GL_GOOGLE_include_directive : require

struct VS_IN {
	vec3 position;
	vec2 texcoord;
};

struct VS_OUT {
	vec2 texcoord;
	vec3 col;
};


struct VP {
	mat4 view;
	mat4 projection;
};

layout(location = 0) out VS_OUT _out;

layout(location = 0+0) in vec3 _VS_IN_position;
layout(location = 0+1) in vec2 _VS_IN_texcoord;
layout(std140, binding = 0) uniform _aspect_ {
	VP vp;
} _aspect;
layout(std140, binding = 1) uniform _user_ {
	mat4 model_matrix;
	vec3 col;
	vec3 col2;
} _user;
layout(binding = 1 + 1 + 0) uniform sampler2D t1;

#line 17 "../../examples/test.vush"
VS_OUT opaque_vertex(VS_IN vin, VP vp, mat4 model_matrix, vec3 col) {
	VS_OUT vout;
	gl_Position = vp.projection * vp.view * model_matrix * vec4(vin.position, 1.0);
	vout.texcoord = vin.texcoord;
	vout.col = col;
	return vout;
}
void main() {
	VS_IN vin;
	vin.position = _VS_IN_position;
	vin.texcoord = _VS_IN_texcoord;
	VP vp = _aspect.vp;
	mat4 model_matrix = _user.model_matrix;
	vec3 col = _user.col;
	_out = opaque_vertex(vin, vp, model_matrix, col);
}