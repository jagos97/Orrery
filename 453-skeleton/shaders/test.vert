#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 T;

out vec3 fragPos;
out vec3 n;
out vec2 tc;
//out mat4 tr;


void main() {
	fragPos = vec3( T * vec4(pos,1));
	n = vec3( T * vec4(normal,0));
	//tr = T;
	tc = texCoord;
	gl_Position = P * V * M * T * vec4(pos, 1.0);
}
