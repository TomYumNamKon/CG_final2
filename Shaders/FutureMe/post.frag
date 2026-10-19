#version 330 core
in vec2 uv;
out vec4 color;
uniform sampler2D sceneTex,overlayTex,depthTex,bloomTex;
uniform mat4 projection,inverseProjection;
uniform mat4 inverseView,inverseShip;
uniform float time;
uniform vec3 sunScreen;
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
vec3 exhaust(out float glow){
 vec3 camera=(inverseShip*inverseView*vec4(0,0,0,1)).xyz;
 vec3 surface=(inverseShip*inverseView*vec4(viewPosition(uv),1)).xyz;
 vec3 ray=normalize(surface-camera);float sceneDistance=length(surface-camera);
 vec3 sum=vec3(0);glow=0.;
 for(int engine=0;engine<2;engine++){
  float z=engine==0?-1.48:1.48;vec3 center=vec3(-5.0,-.10,z);
  vec3 offset=camera-center;float b=dot(offset,ray),d=b*b-dot(offset,offset)+5.76;
  if(d<0.)continue;
  float start=max(0.,-b-sqrt(d)),end=min(sceneDistance,-b+sqrt(d));
  if(end<=start)continue;
  float stepSize=(end-start)/40.;
  for(int i=0;i<40;i++){
   vec3 p=camera+ray*(start+(float(i)+.5)*stepSize);
   float tail=-p.x-3.43;if(tail<0.||tail>3.8)continue;
   float radial=length(p.yz-vec2(-.1,z));float width=.33+tail*.055;
   float density=exp(-radial*radial/(width*width)*3.)*exp(-tail*.72);
   density*=.70+.30*sin(tail*17.-time*8.+sin(p.y*35.)*.4);
   float core=exp(-radial*radial/(width*width)*16.);
   sum+=mix(vec3(.025,.28,1.7),vec3(.5,1.5,2.4),core)*density*stepSize*5.;
   glow+=density*stepSize;
  }
 }
 return sum;
}
void main(){
 vec3 c=texture(sceneTex,uv).rgb*contactAO();
 float plasma;vec3 plume=exhaust(plasma);
 // Heat shimmer affects the background only where the ray meets the exhaust.
 if(plasma>.025){vec2 bend=vec2(sin(uv.y*160.+time*3.),cos(uv.x*170.-time*2.))*.0012*min(plasma,1.);
  c=mix(c,texture(sceneTex,uv+bend).rgb,.25);}
 c+=plume;
 if(bloomEnabled==1){
  c+=texture(bloomTex,uv).rgb*.32;
  if(sunScreen.z>0.&&all(greaterThan(sunScreen.xy,vec2(0)))&&all(lessThan(sunScreen.xy,vec2(1)))){
   float visibility=step(.9999,texture(depthTex,sunScreen.xy).r);
   vec2 delta=(uv-sunScreen.xy)*vec2(1.3333,1);float d=length(delta);
   vec3 warm=vec3(1.,.72,.39);
   c+=warm*visibility*(.35*exp(-d*12.)+.07*exp(-abs(delta.y)*180.-abs(delta.x)*3.));
  }
 }
 c*=1.-.22*pow(length((uv-.5)*1.35),2.);
 c=mix(vec3(dot(c,vec3(.2126,.7152,.0722))),c,.88);
 c=pow(aces(c*1.1),vec3(1./2.2));
 if(overlayEnabled==1){vec4 o=texture(overlayTex,vec2(uv.x,1.-uv.y));c=mix(c,o.rgb,o.a);}
 color=vec4(c,1);
}
