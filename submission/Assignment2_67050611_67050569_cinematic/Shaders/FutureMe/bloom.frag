#version 330 core
in vec2 uv;
out vec4 color;
uniform sampler2D source;
uniform int mode;
void main(){
 if(mode==0){
  // Downsample a small footprint rather than skipping narrow emissive features.
  vec3 c=vec3(0);vec2 pixel=1./vec2(textureSize(source,0));
  for(int x=-1;x<=1;x++)for(int y=-1;y<=1;y++)c+=texture(source,uv+vec2(x,y)*pixel*2.).rgb/9.;
  color=vec4(max(c-.8,0.),1);return;
 }
 vec2 stepUV=(mode==1?vec2(1,0):vec2(0,1))/vec2(textureSize(source,0));
 vec3 c=vec3(0);float sum=0.;
 for(int i=-8;i<=8;i++){float w=exp(-float(i*i)/18.);c+=texture(source,uv+stepUV*float(i)).rgb*w;sum+=w;}
 color=vec4(c/sum,1);
}
