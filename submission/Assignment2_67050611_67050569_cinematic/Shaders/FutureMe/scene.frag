#version 330 core
in vec3 P,N;
in vec2 UV;
in vec4 lightP;
out vec4 color;
uniform vec3 baseColor,eye,shipOffset,sunDirection;
uniform float emission,specular;
uniform int kind,shadows;
uniform sampler2D shadowMap,cargo,grabLogo,cloudMap;
const float PI=3.14159265;
const vec3 planetCenter=vec3(18,6,-22);
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
 vec3 f=normalize(vec3(22,4,-10)),s=normalize(vec3(-f.z,0,f.x)),u=cross(s,f);
 return mat3(f,u,s)*p*1.15+vec3(-.5,.7,2)+shipOffset;
}
void main(){
 vec3 n=normalize(N),v=normalize(eye-P),l=normalize(sunDirection);
 vec3 albedo=baseColor;float rough=.38,metallic=.12;
 vec3 materialP=P-shipOffset;
 float grain=noise(P*190.);
 float logoMask=0.;
 if(kind==3){
  vec2 q=(UV-.5)*vec2(1.10,1.9)+.5;
  if(all(greaterThanEqual(q,vec2(0)))&&all(lessThanEqual(q,vec2(1))))logoMask=texture(grabLogo,q).a;
  albedo=mix(vec3(.008,.30,.055),vec3(.95),logoMask);rough=.73;metallic=0.;
 }
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
 if(kind==10){
  rough=.48;metallic=.48;
  float mottling=fbm(materialP*12.);
  float chips=smoothstep(.74,.89,noise(materialP*vec3(38,110,38)))*smoothstep(.50,.65,mottling);
  float scratches=pow(noise(materialP*vec3(5,280,140)),14.);
  albedo*=.62+.55*mottling;
  albedo=mix(albedo,vec3(.30,.28,.22),chips*.85+scratches*.5);
  rough=mix(rough,.72,chips);metallic=mix(metallic,.8,chips);
 }
 if(kind==11){
  rough=.88;metallic=0.;
  float soil=fbm(materialP*18.);albedo*=.65+.40*soil;
 }
 if(kind==12){rough=.14;metallic=.98;}
 if(kind==14){
  float edge=pow(1.-max(dot(n,v),0.),3.5);
  float day=smoothstep(-.18,.3,dot(n,l));
  color=vec4(vec3(.12,.32,.65)*edge*day*2.,edge*.65);return;
 }
 if(kind==1){
  vec3 q=normalize(P-planetCenter);
  vec3 warp=vec3(fbm(q*3.+7.),fbm(q*3.+21.),fbm(q*3.+48.));
  float terrain=fbm(q*3.1+warp*1.6+vec3(3,8,1));float land=smoothstep(.49,.51,terrain);
  vec3 ground=mix(vec3(.025,.065,.03),vec3(.27,.21,.115),smoothstep(.51,.64,terrain));
  ground*=.65+.6*fbm(q*110.);
  albedo=mix(vec3(.008,.033,.065),ground,land);
  float ice=smoothstep(.82,.96,abs(q.y)+noise(q*24.)*.07);albedo=mix(albedo,vec3(.72,.83,.88),ice);
  float cloud=texture(cloudMap,vec2(fract(UV.x+.18),UV.y)).r;
  cloud=smoothstep(.12,.88,cloud);
  albedo=mix(albedo,vec3(.75,.82,.85),cloud*.90);
  rough=mix(.20,.83,max(land,cloud));metallic=0.;
  float city=step(.90,noise(q*350.))*step(.505,terrain)*(1.-smoothstep(.52,.55,terrain))*step(.5,noise(q*65.))*(1.-cloud)*(1.-smoothstep(-.2,.12,dot(n,l)));
  vec3 c=brdf(n,v,l,albedo,rough,metallic)*vec3(7.,5.9,4.4)+albedo*.12;
  c+=vec3(.08,.33,.68)*pow(1.-max(dot(n,v),0.),3.8)*(.3+.7*max(dot(n,l),0.));
  c+=vec3(1.7,.65,.16)*city;
  color=vec4(c,1);return;
 }
 if(kind==2){
  vec3 off=P-planetCenter;float radius=length(off);
  albedo*=.52+.16*sin(radius*43.)+.11*sin(radius*97.)+.13*noise(P*18.);
  float b=dot(off,l),c=dot(off,off)-14.*14.;
  float eclipse=(b<0.&&b*b-c>0.)?.05:1.;
  color=vec4(albedo*(.12+abs(dot(n,l))*1.4)*eclipse,1);return;
 }
 if(kind==7||kind==15){albedo*=.4+.8*fbm(P*9.);rough=.96;metallic=0.;}
 // Derivative-based micro relief perturbs the normal in the surface tangent plane.
 float bump=(noise(materialP*75.)-.5)*(kind==11?.0005:kind==10?.0007:.00012);
 vec3 dpdx=dFdx(P),dpdy=dFdy(P),r1=cross(dpdy,n),r2=cross(n,dpdx);
 float det=dot(dpdx,r1);
 n=normalize(n-sign(det)*(dFdx(bump)*r1+dFdy(bump)*r2)/max(abs(det),.000001));
 float sh=shadows==1?shadow(n,l):0.;
 vec3 c=brdf(n,v,l,albedo,rough,metallic)*vec3(11.,8.4,5.3)*(1.-sh*.98);
 c+=brdf(n,v,normalize(planetCenter-P),albedo,rough,metallic)*vec3(.5,.65,.82);
 vec3 env=mix(vec3(.005,.008,.014),vec3(.055,.075,.10),n.y*.5+.5);
 c+=albedo*env*(1.-metallic)+fresnel(max(dot(n,v),0.),mix(vec3(.04),albedo,metallic))*env*.6;
 // Analytic environment reflection of the actual destination on glossy surfaces.
 vec3 refl=reflect(-v,n),dest=normalize(planetCenter-P);
 float reflection=smoothstep(.91,.99,dot(refl,dest));
 c+=vec3(.12,.36,.47)*reflection*(1.-rough)*metallic;
 if(kind==12){
  // Trace reflected rays against the same planet sphere and ring plane.
  vec3 off=P-planetCenter;float b=dot(off,refl),disc=b*b-dot(off,off)+196.;
  if(disc>0.&&-b+sqrt(disc)>0.)c+=vec3(.16,.29,.36)*.65;
  vec3 ringN=normalize(vec3(.35,1.,-.12/.99));float denom=dot(refl,ringN);
  if(abs(denom)>.0001){float t=dot(planetCenter-P,ringN)/denom;
   float r=length(P+refl*t-planetCenter);
   if(t>0.&&r>19.18&&r<29.12)c+=vec3(.8,.63,.34)*(.5+.2*sin(r*43.));
  }
 }
 for(int i=0;i<2;i++){
  vec3 delta=shipPoint(vec3(-3.6,-.10,i==0?-1.48:1.48))-P;
  c+=brdf(n,v,normalize(delta),albedo,rough,metallic)*vec3(.15,2.5,7)/(1.+dot(delta,delta)*1.4);
 }
 c+=brdf(n,v,normalize(vec3(-1,4,12)),albedo,rough,metallic)*vec3(.42,.50,.62);
 // Broad, warm ring bounce reveals materials without replacing the direct shadows.
 c+=brdf(n,v,normalize(vec3(-4,6,4)),albedo,min(rough+.2,1.),metallic)*vec3(.85,.64,.40);
 if(kind==3)c+=albedo*.20+vec3(.10)*logoMask;
 c+=albedo*emission;
 color=vec4(c,1);
}

