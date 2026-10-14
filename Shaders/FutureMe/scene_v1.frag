#version 330 core
in vec3 P,N;
in vec2 UV;
in vec4 lightP;
out vec4 color;
uniform vec3 baseColor,eye;
uniform float emission,specular;
uniform int kind,shadows;
uniform sampler2D shadowMap,cargo;
float shadow(vec3 n,vec3 l){
 vec3 q=lightP.xyz/lightP.w*.5+.5;
 if(q.z>1.||q.z<0.||any(lessThan(q.xy,vec2(0)))||any(greaterThan(q.xy,vec2(1))))return 0.;
 float bias=max(.0018*(1.-dot(n,l)),.00035),s=0.;
 vec2 pixel=1./vec2(textureSize(shadowMap,0));
 for(int x=-2;x<=2;x++)for(int y=-2;y<=2;y++)
  s+=q.z-bias>texture(shadowMap,q.xy+vec2(x,y)*pixel).r?1.:0.;
 return s/25.;
}
void main(){
 vec3 n=normalize(N),v=normalize(eye-P),l=normalize(vec3(-3,7,5));
 vec3 albedo=baseColor;
 if(kind==3)albedo=texture(cargo,UV).rgb;
 if(kind==1){
  float bands=sin(P.y*9.+sin(P.x*2.)*.65+sin(P.z*3.)*.3);
  albedo=mix(vec3(.025,.13,.23),vec3(.18,.56,.58),bands*.5+.5);
 }
 if(kind==2){
  float r=length((P-vec3(.5,3.5,-12)).xz);
  albedo*=.65+.35*sin(r*82.);
 }
 float sh=shadows==1?shadow(n,l):0.;
 float ndl=max(dot(n,l),0.);
 float gloss=pow(max(dot(n,normalize(l+v)),0.),mix(28.,140.,specular));
 vec3 c=albedo*(vec3(.065,.10,.16)+vec3(1.8,1.43,1.02)*ndl*(1.-sh*.87));
 c+=vec3(1.4,1.25,1.05)*gloss*specular*(1.-sh);
 vec3 rimDir=normalize(vec3(-3,2,-5));
 c+=albedo*vec3(.1,.65,1.4)*max(dot(n,rimDir),0.)*.95;
 float rim=pow(1.-max(dot(n,v),0.),3.);
 c+=vec3(.06,.30,.42)*rim*specular;
 if(kind==1)c+=vec3(.08,.55,.9)*pow(1.-max(dot(n,v),0.),4.)*.9;
 for(int i=0;i<2;i++){
  vec3 delta=vec3(-3.66,-.5964,i==0?-2.013:2.013)-P;
  c+=albedo*vec3(.08,.55,1.5)*max(dot(n,normalize(delta)),0.)/(1.+dot(delta,delta)*.5);
 }
 c+=albedo*emission;
 color=vec4(c,1);
}
