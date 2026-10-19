#version 330 core
in vec2 uv;
out vec4 color;
uniform mat4 inverseView,inverseProjection;
uniform vec3 sunDirection;
float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float noise(vec2 p){vec2 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);return mix(mix(hash(i),hash(i+vec2(1,0)),f.x),mix(hash(i+vec2(0,1)),hash(i+1.),f.x),f.y);}
float fbm(vec2 p){float n=0.,a=.5;for(int i=0;i<6;i++){n+=a*noise(p);p=mat2(.8,-.6,.6,.8)*p*2.1+3.;a*=.5;}return n;}
void main(){
 float halo=exp(-dot((uv-vec2(.47,.55))*vec2(1.3,1.),(uv-vec2(.47,.55))*vec2(1.3,1.))*7.);
 color=vec4(mix(vec3(.001,.002,.004),vec3(.012,.020,.028),halo),1);
}
