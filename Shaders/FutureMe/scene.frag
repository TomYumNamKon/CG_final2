#version 330 core
in vec3 P,N;
in vec2 UV;
in vec4 lightP;
out vec4 color;
uniform vec3 baseColor,eye,shipOffset;
uniform float emission,specular;
uniform int kind,shadows;
uniform sampler2D shadowMap,cargo;
const float PI=3.14159265;
const vec3 planetCenter=vec3(7,4,-10);
float hash(vec3 p){return fract(sin(dot(p,vec3(127.1,311.7,74.7)))*43758.5453);}
float noise(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);return mix(mix(mix(hash(i),hash(i+vec3(1,0,0)),f.x),mix(hash(i+vec3(0,1,0)),hash(i+vec3(1,1,0)),f.x),f.y),mix(mix(hash(i+vec3(0,0,1)),hash(i+vec3(1,0,1)),f.x),mix(hash(i+vec3(0,1,1)),hash(i+1.),f.x),f.y),f.z);}
float fbm(vec3 p){float v=0.,a=.5;for(int i=0;i<6;i++){v+=noise(p)*a;p=p*2.03+vec3(5.2,1.3,7.1);a*=.5;}return v;}
float shadow(vec3 n,vec3 l){
 vec3 q=lightP.xyz/lightP.w*.5+.5;
 if(q.z>1.||q.z<0.||any(lessThan(q.xy,vec2(0)))||any(greaterThan(q.xy,vec2(1))))return 0.;
 float bias=max(.0007*(1.-dot(n,l)),.00018),s=0.;vec2 pixel=1./vec2(textureSize(shadowMap,0));
 for(int x=-2;x<=2;x++)for(int y=-2;y<=2;y++)s+=q.z-bias>texture(shadowMap,q.xy+vec2(x,y)*pixel).r?1.:0.;
 return s/25.;
}
vec3 fresnel(float c,vec3 f){return f+(1.-f)*pow(1.-c,5.);}
vec3 brdf(vec3 n,vec3 v,vec3 l,vec3 albedo,float rough,float metallic){
 vec3 h=normalize(v+l);float nv=max(dot(n,v),.001),nl=max(dot(n,l),0.),nh=max(dot(n,h),0.);
 float a=rough*rough,a2=a*a,d=nh*nh*(a2-1.)+1.;
 float D=a2/max(PI*d*d,.0001),k=(rough+1.)*(rough+1.)/8.;
 float G=nv/(nv*(1.-k)+k)*nl/(nl*(1.-k)+k);
 vec3 F=fresnel(max(dot(h,v),0.),mix(vec3(.04),albedo,metallic));
 return ((1.-F)*(1.-metallic)*albedo/PI+D*G*F/max(4.*nv*nl,.001))*nl;
}
vec3 shipPoint(vec3 p){
 vec3 f=normalize(vec3(8,4.35,-11)),s=normalize(vec3(-f.z,0,f.x)),u=cross(s,f);
 return mat3(f,u,s)*p*1.08+vec3(-1,-.35,1)+shipOffset;
}
void main(){
 vec3 n=normalize(N),v=normalize(eye-P),l=normalize(vec3(-3,7,5));
 vec3 albedo=baseColor;float rough=.38,metallic=.12;
 float grain=noise(P*190.);
 if(kind==3){albedo=pow(texture(cargo,UV).rgb,vec3(2.2));rough=.65;metallic=0.;}
 if(kind==4){rough=.89;metallic=0.;albedo*=.82+.18*sin(P.y*80.)*sin(P.z*80.);}
 if(kind==5){rough=.92;metallic=0.;}
 if(kind==6){rough=.29;metallic=.92;albedo*=.85+.15*noise(P*vec3(3,160,160));}
 if(kind==0){
  metallic=specular>.8?.86:.12;rough=specular>.9?.15:.38;
  if(baseColor.b>baseColor.r*3.&&emission==0.){metallic=.92;rough=.12;}
  float wear=smoothstep(.72,.86,noise(P*85.))*smoothstep(.5,.7,noise(P*6.));
  albedo=mix(albedo,vec3(.23,.25,.24),wear*.32);rough+=grain*.08;
 }
 if(kind==8){albedo*=.3+.7*step(.25,fract(P.x*25.));}
 if(kind==1){
  vec3 q=normalize(P-planetCenter);
  float terrain=fbm(q*5.+vec3(3,8,1));float land=smoothstep(.48,.515,terrain);
  vec3 ground=mix(vec3(.022,.07,.028),vec3(.24,.19,.09),smoothstep(.51,.68,terrain));
  albedo=mix(vec3(.006,.035,.085),ground,land);
  float ice=smoothstep(.82,.96,abs(q.y)+noise(q*24.)*.07);albedo=mix(albedo,vec3(.72,.83,.88),ice);
  float cloud=smoothstep(.51,.67,fbm(q*13.+vec3(fbm(q*8.)*3.)));
  albedo=mix(albedo,vec3(.75,.82,.85),cloud*.90);
  rough=mix(.20,.83,max(land,cloud));metallic=0.;
  float city=step(.82,noise(q*350.))*step(.53,terrain)*(1.-cloud)*(1.-smoothstep(-.2,.12,dot(n,l)));
  vec3 c=brdf(n,v,l,albedo,rough,metallic)*vec3(4.7,4.3,3.8)+albedo*.025;
  c+=vec3(.08,.33,.68)*pow(1.-max(dot(n,v),0.),3.8)*(.3+.7*max(dot(n,l),0.));
  c+=vec3(1.7,.65,.16)*city;
  color=vec4(c,1);return;
 }
 if(kind==2){
  vec3 off=P-planetCenter;float radius=length(off);
  albedo*=.42+.32*sin(radius*95.)+.20*sin(radius*213.);
  float b=dot(off,l),c=dot(off,off)-3.332*3.332;
  float eclipse=(b<0.&&b*b-c>0.)?.05:1.;
  color=vec4(albedo*(.10+abs(dot(n,l))*.9)*eclipse,1);return;
 }
 if(kind==7){albedo*=.4+.8*fbm(P*9.);rough=.96;metallic=0.;}
 // Derivative-based micro relief perturbs the normal in the surface tangent plane.
 float bump=(noise(P*75.)-.5)*(kind==4?.0005:.00012);
 vec3 dpdx=dFdx(P),dpdy=dFdy(P),r1=cross(dpdy,n),r2=cross(n,dpdx);
 float det=dot(dpdx,r1);
 n=normalize(n-sign(det)*(dFdx(bump)*r1+dFdy(bump)*r2)/max(abs(det),.000001));
 float sh=shadows==1?shadow(n,l):0.;
 vec3 c=brdf(n,v,l,albedo,rough,metallic)*vec3(4.2,3.65,2.9)*(1.-sh*.97);
 c+=brdf(n,v,normalize(vec3(2,4,-5)),albedo,rough,metallic)*vec3(.32,.72,1.3);
 vec3 env=mix(vec3(.012,.019,.03),vec3(.16,.20,.25),n.y*.5+.5);
 c+=albedo*env*(1.-metallic)+fresnel(max(dot(n,v),0.),mix(vec3(.04),albedo,metallic))*env*.6;
 // Analytic environment reflection of the actual destination on glossy surfaces.
 vec3 refl=reflect(-v,n),dest=normalize(planetCenter-P);
 float reflection=smoothstep(.91,.99,dot(refl,dest));
 c+=vec3(.12,.36,.47)*reflection*(1.-rough)*metallic;
 for(int i=0;i<2;i++){
  vec3 delta=shipPoint(vec3(-2.9,-.12,i==0?-1.65:1.65))-P;
  c+=brdf(n,v,normalize(delta),albedo,rough,metallic)*vec3(.10,1.4,3.4)/(1.+dot(delta,delta)*1.4);
 }
 c+=brdf(n,v,normalize(vec3(-1,4,12)),albedo,rough,metallic)*vec3(.55,.65,.75);
 c+=albedo*emission;
 color=vec4(c,1);
}

