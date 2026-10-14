#version 330 core
layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 normal;
uniform mat4 model, view, projection, lightVP;
out vec3 P,N;
out vec2 UV;
out vec4 lightP;
void main(){
 vec4 world=model*vec4(position,1);
 P=world.xyz; N=mat3(transpose(inverse(model)))*normal; UV=uv;
 lightP=lightVP*world; gl_Position=projection*view*world;
}
