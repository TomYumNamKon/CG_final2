#version 330 core
in vec2 uv;
out vec4 color;
float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float noise(vec2 p){vec2 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);return mix(mix(hash(i),hash(i+vec2(1,0)),f.x),mix(hash(i+vec2(0,1)),hash(i+1.),f.x),f.y);}
float fbm(vec2 p){float n=0.,a=.5;for(int i=0;i<6;i++){n+=a*noise(p);p=mat2(.8,-.6,.6,.8)*p*2.1+3.;a*=.5;}return n;}
void main(){
 vec2 p=uv*vec2(1.3333,1);
 float f=fbm(p*5.);
 float band=exp(-pow((uv.y-.28-uv.x*.33+f*.18)*4.,2.));
 vec3 c=vec3(.002,.004,.010)+band*pow(f,2.)*vec3(.045,.055,.11);
 c+=pow(fbm(p*8.+9.),3.)*band*vec3(.055,.012,.05);
 for(int layer=0;layer<3;layer++){
  float scale=180.+float(layer)*110.;vec2 grid=p*scale,cell=floor(grid);
  vec2 delta=fract(grid)-vec2(hash(cell+17.),hash(cell+31.));
  float h=hash(cell+float(layer)*91.);
  float star=exp(-dot(delta,delta)*(layer==0?170.:360.))*step(.977,h);
  c+=star*mix(vec3(.35,.65,1.),vec3(1.,.76,.48),hash(cell+8.))*(.5+h);
 }
 color=vec4(c,1);
}
