"""Original vertex-built assets. Run once; the renderer loads the exported OBJs.
No downloaded mesh or image is used. See docs/FUTURE_ME.md for inspiration.
"""
from pathlib import Path
import math, random
from PIL import Image, ImageDraw, ImageFont
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'Models/FutureMe'
OUT.mkdir(parents=True,exist_ok=True)
groups={}
materials={
 'green':((.014,.20,.065),0,.45,0), 'lime':((.15,.46,.12),0,.35,0),
 'dark':((.018,.03,.045),0,.75,0), 'metal':((.24,.32,.37),0,.85,0),
 'white':((.79,.87,.82),0,.4,0), 'gold':((.9,.42,.08),0,1.,0),
 'visor':((.018,.085,.12),0,1.,0), 'cyan':((.08,.8,1),4,.1,0),
 'amber':((1,.36,.035),3,.1,0), 'flame':((.1,.38,1),3,.1,0),
 'planet':((.1,.4,.5),0,.15,1), 'ring':((.55,.39,.24),.12,.1,2),
 'moon':((.24,.22,.3),0,.1,7), 'label':((1,1,1),0,.1,3),
 'fabric':((.022,.17,.065),0,.12,4), 'rubber':((.012,.018,.023),0,.05,5),
 'titanium':((.35,.39,.41),0,.95,6), 'brass':((.54,.31,.11),0,.95,6),
 'red':((.6,.02,.008),1,.2,0), 'screen':((.06,.55,.65),.5,.2,8),
 'stationMetal':((.28,.32,.35),0,.9,6), 'stationLight':((.1,.6,.8),2,.2,8),
 'solar':((.015,.04,.09),0,.7,9)}
def tri(mat,a,b,c,ns=None,uv=None):
 if ns is None:
  u=[b[i]-a[i] for i in range(3)];v=[c[i]-a[i] for i in range(3)]
  n=[u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]]
  l=math.sqrt(sum(x*x for x in n))
  if l<1e-9:return
  ns=[tuple(x/l for x in n)]*3
 groups.setdefault(mat,[]).extend(zip([a,b,c],ns,uv or [(0,0),(1,0),(1,1)]))
def sphere(mat,p,s,n=48,m=24):
 def vert(i,j):
  a=2*math.pi*i/n;b=math.pi*j/m
  q=(math.sin(b)*math.cos(a),math.cos(b),math.sin(b)*math.sin(a))
  normal=[q[k]/s[k] for k in range(3)];l=math.sqrt(sum(x*x for x in normal))
  return tuple(p[k]+q[k]*s[k] for k in range(3)),tuple(x/l for x in normal),(i/n,j/m)
 for j in range(m):
  for i in range(n):
   vs=[vert(i,j),vert(i+1,j),vert(i+1,j+1),vert(i,j+1)]
   for inds in [(0,2,1),(0,3,2)]:tri(mat,*[vs[k][0] for k in inds],[vs[k][1] for k in inds],[vs[k][2] for k in inds])
def box(mat,p,s):
 vs=[(p[0]+x*s[0],p[1]+y*s[1],p[2]+z*s[2]) for x,y,z in [(-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1)]]
 for a,b,c,d in [(0,3,2,1),(4,5,6,7),(0,4,7,3),(1,2,6,5),(3,7,6,2),(0,1,5,4)]:
  tri(mat,vs[a],vs[b],vs[c]);tri(mat,vs[a],vs[c],vs[d])
def bevelbox(mat,p,s,r=.06):
 # Rounded cuboid made by projecting a tessellated cube onto its inset box.
 for axis in range(3):
  for sign in [-1,1]:
   axes=[k for k in range(3) if k!=axis]
   coords=[[-s[k],-s[k]+r,s[k]-r,s[k]] for k in axes]
   def sample(u,v):
    q=[0,0,0];q[axis]=sign*s[axis];q[axes[0]]=u;q[axes[1]]=v
    core=[max(-s[k]+r,min(s[k]-r,q[k])) for k in range(3)]
    d=[q[k]-core[k] for k in range(3)];l=math.sqrt(sum(x*x for x in d));n=tuple(x/l for x in d)
    return tuple(p[k]+core[k]+r*n[k] for k in range(3)),n
   for i in range(3):
    for j in range(3):
     vs=[sample(coords[0][a],coords[1][b]) for a,b in [(i,j),(i+1,j),(i+1,j+1),(i,j+1)]]
     for idx in [(0,1,2),(0,2,3)]:tri(mat,*[vs[k][0] for k in idx],[vs[k][1] for k in idx])
def loft(mat,sections,n=64):
 # Continuous elliptical hull; profile controls taper and shoulder breaks.
 for j in range(len(sections)-1):
  def sample(k,i):
   x,y,ry,rz=sections[k];a=2*math.pi*i/n
   pre=sections[max(0,k-1)];nxt=sections[min(len(sections)-1,k+1)]
   dx=max(.001,nxt[0]-pre[0]);slope=(nxt[2]-pre[2])/dx
   norm=(-slope,math.cos(a),math.sin(a)*ry/max(rz,.001));l=math.sqrt(sum(t*t for t in norm))
   return (x,y+ry*math.cos(a),rz*math.sin(a)),tuple(t/l for t in norm)
  for i in range(n):
   vs=[sample(k,t) for k,t in [(j,i),(j+1,i),(j+1,i+1),(j,i+1)]]
   for inds in [(0,1,2),(0,2,3)]:tri(mat,*[vs[k][0] for k in inds],[vs[k][1] for k in inds])
def tube(mat,a,b,r1,r2=None,n=32):
 r2=r1 if r2 is None else r2
 d=[b[k]-a[k] for k in range(3)];l=math.sqrt(sum(x*x for x in d));d=[x/l for x in d]
 up=(0,1,0) if abs(d[1])<.95 else (1,0,0)
 u=(d[1]*up[2]-d[2]*up[1],d[2]*up[0]-d[0]*up[2],d[0]*up[1]-d[1]*up[0]);l=math.sqrt(sum(x*x for x in u));u=[x/l for x in u]
 v=(d[1]*u[2]-d[2]*u[1],d[2]*u[0]-d[0]*u[2],d[0]*u[1]-d[1]*u[0])
 def pt(p,r,i):return tuple(p[k]+r*(u[k]*math.cos(2*math.pi*i/n)+v[k]*math.sin(2*math.pi*i/n)) for k in range(3))
 for i in range(n):
  aa,ab,ba,bb=pt(a,r1,i),pt(a,r1,i+1),pt(b,r2,i),pt(b,r2,i+1)
  tri(mat,aa,ab,bb);tri(mat,aa,bb,ba);tri(mat,a,ab,aa);tri(mat,b,ba,bb)
def torus(mat,p,r,t,axis='x',n=80):
 def pt(a,b):
  q=((r+t*math.cos(b))*math.cos(a),t*math.sin(b),(r+t*math.cos(b))*math.sin(a))
  if axis=='x':q=(q[1],q[0],q[2])
  return tuple(p[k]+q[k] for k in range(3))
 for i in range(n):
  for j in range(10):
   vs=[pt(2*math.pi*a/n,2*math.pi*b/10) for a,b in [(i,j),(i+1,j),(i+1,j+1),(i,j+1)]]
   tri(mat,*vs[:3]);tri(mat,vs[0],vs[2],vs[3])
from cinematic_scene import build_scene
build_scene(globals())
manifest=[]
for name,verts in groups.items():
 if name not in ['planet','ring','moon','rock','atmosphere']:
  forward=(22,4,-10);ln=math.sqrt(sum(x*x for x in forward));forward=tuple(x/ln for x in forward)
  side=(-forward[2],0,forward[0]);ln=math.sqrt(sum(x*x for x in side));side=tuple(x/ln for x in side)
  up=(side[1]*forward[2]-side[2]*forward[1],side[2]*forward[0]-side[0]*forward[2],side[0]*forward[1]-side[1]*forward[0])
  def rotate(p):return tuple(forward[k]*p[0]+up[k]*p[1]+side[k]*p[2] for k in range(3))
  verts=[(tuple(rotate(p)[k]*1.15+[-.5,.7,2][k] for k in range(3)),rotate(n),uv) for p,n,uv in verts]
 unique=list(dict.fromkeys(verts));lookup={v:i+1 for i,v in enumerate(unique)}
 assert all(math.isfinite(x) for p,n,uv in unique for x in (*p,*n,*uv))
 assert all(abs(sum(x*x for x in n)-1)<1e-4 for p,n,uv in unique)
 with (OUT/(name+'.obj')).open('w') as f:
  f.write('# Original FUTURE ME geometry; generated by tools/generate_future_me.py\n')
  for p,n,uv in unique:f.write('v '+' '.join(f'{v:.6f}' for v in p)+'\n')
  for p,n,uv in unique:f.write('vt '+' '.join(f'{v:.6f}' for v in uv)+'\n')
  for p,n,uv in unique:f.write('vn '+' '.join(f'{v:.6f}' for v in n)+'\n')
  for i in range(0,len(verts),3):f.write('f '+' '.join(f'{k}/{k}/{k}' for k in [lookup[v] for v in verts[i:i+3]])+'\n')
 color,emit,spec,kind=materials[name]
 manifest.append(f'{name}.obj '+ ' '.join(map(str,(*color,emit,spec,kind))))
(OUT/'scene.txt').write_text('\n'.join(manifest))
TEX=ROOT/'Textures/FutureMe';TEX.mkdir(parents=True,exist_ok=True)
def font(size,bold=False):return ImageFont.truetype('C:/Windows/Fonts/'+('arialbd.ttf' if bold else 'arial.ttf'),size)
im=Image.new('RGBA',(512,320),(28,42,29,255));d=ImageDraw.Draw(im)
d.text((256,85),'GrabFood',font=font(78,True),anchor='mm',fill='white')
d.text((256,171),'ORBITAL DELIVERY',font=font(27,True),anchor='mm',fill=(200,255,215))
d.line((55,216,457,216),fill=(190,255,210),width=3)
d.text((256,264),'HOT MEALS / ZERO GRAVITY',font=font(22),anchor='mm',fill='white')
im.save(TEX/'cargo.png')
# Graphic typography overlay only; the scene itself is rendered entirely in OpenGL.
im=Image.new('RGBA',(1600,1200));d=ImageDraw.Draw(im)
d.text((72,62),'F U T U R E   M E   /   2 0 8 6',font=font(22,True),fill=(153,231,205,255))
d.text((68,101),'NEXT STOP:',font=font(68,True),fill=(238,247,244,255))
d.text((68,171),'ANOTHER WORLD.',font=font(68,True),fill=(238,247,244,255))
d.line((74,275,184,275),fill=(92,242,169,255),width=5)
d.text((72,299),'Same rider. A little further from home.',font=font(23),fill=(159,187,196,255))
d.text((1240,505),'KEPLER 04',font=font(23,True),anchor='mm',fill=(196,224,226,230))
d.text((1240,535),'ORBITAL FOOD TERMINAL',font=font(15),anchor='mm',fill=(132,190,200,230))
d.text((75,1065),'GRABFOOD  /  INTERSTELLAR',font=font(24,True),fill=(165,250,205,255))
d.text((75,1104),'ONE HOT MEAL. ACROSS A COLD UNIVERSE.',font=font(19),fill=(152,177,192,255))
d.text((1520,1035),'ORDER #2086',font=font(21,True),anchor='ra',fill=(132,227,209,255))
d.text((1520,1074),'KEPLER PORT  /  04',font=font(27,True),anchor='ra',fill=(229,242,245,255))
d.text((1520,1115),'67050611  +  67050569',font=font(19),anchor='ra',fill=(152,177,192,255))
im.save(TEX/'overlay.png')
print(f'Exported {len(groups)} OBJ meshes; {sum(len(v)//3 for v in groups.values()):,} triangles')


