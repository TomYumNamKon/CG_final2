#version 330 core
in vec2 uv;
out vec4 color;
uniform sampler2D sceneTex,overlayTex,depthTex;
uniform mat4 projection,inverseProjection;
uniform int overlayEnabled,bloomEnabled;
vec3 aces(vec3 x){return clamp((x*(2.51*x+.03))/(x*(2.43*x+.59)+.14),0.,1.);}
vec3 viewPosition(vec2 coord){vec4 p=inverseProjection*vec4(coord*2.-1.,texture(depthTex,coord).r*2.-1.,1.);return p.xyz/p.w;}
float contactAO(){
 if(texture(depthTex,uv).r>.9999)return 1.;
 vec3 p=viewPosition(uv),n=normalize(cross(dFdx(p),dFdy(p)));
 if(dot(n,-p)<0.)n=-n;
 vec3 tangent=normalize(cross(n,abs(n.y)<.9?vec3(0,1,0):vec3(1,0,0))),bitangent=cross(n,tangent);
 float occlusion=0.;
 for(int i=0;i<16;i++){
  float a=float(i)*2.39996,r=.10+.50*float(i)/15.;
  vec3 q=p+(tangent*cos(a)*.7+bitangent*sin(a)*.7+n*.6)*r;
  vec4 clip=projection*vec4(q,1);vec2 sampleUV=clip.xy/clip.w*.5+.5;
  if(any(lessThan(sampleUV,vec2(0)))||any(greaterThan(sampleUV,vec2(1))))continue;
  float z=viewPosition(sampleUV).z;
  occlusion+=step(q.z+.035,z)*smoothstep(0.,1.,.5/max(abs(p.z-z),.001));
 }
 return 1.-occlusion/16.*.65;
}
void main(){
 vec3 c=texture(sceneTex,uv).rgb*contactAO();
 if(bloomEnabled==1){
  vec3 glow=vec3(0);float weight=0.;
  for(int x=-5;x<=5;x++)for(int y=-5;y<=5;y++){
   float w=exp(-float(x*x+y*y)/12.);vec3 s=texture(sceneTex,uv+vec2(x,y)/vec2(800,600)*2.).rgb;
   glow+=max(s-vec3(1.),vec3(0))*w;weight+=w;
  }
  c+=glow/weight*.65;
 }
 c*=1.-.22*pow(length((uv-.5)*1.35),2.);
 c=pow(aces(c*1.2),vec3(1./2.2));
 if(overlayEnabled==1){vec4 o=texture(overlayTex,vec2(uv.x,1.-uv.y));c=mix(c,o.rgb,o.a);}
 color=vec4(c,1);
}
