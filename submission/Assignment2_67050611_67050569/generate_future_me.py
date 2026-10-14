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
# Hero: a space scooter with a long nose, twin engine pods, and a seated courier.
loft('dark',[(-2.8,-.1,.02,.02),(-2.5,-.1,.29,.65),(-1.7,-.12,.4,1.0),(0,-.12,.42,1.0),(1.6,-.12,.32,.78),(2.8,-.10,.12,.35),(3.2,-.08,.02,.02)])
loft('green',[(-2.7,.12,.01,.01),(-2.4,.12,.28,.63),(-1.7,.12,.48,.96),(0,.12,.48,1.01),(1.6,.12,.32,.79),(2.8,.12,.12,.33),(3.2,.12,.01,.01)])
loft('white',[(1.1,.34,.08,.53),(1.5,.34,.17,.53),(2.4,.24,.12,.36),(3.05,.14,.015,.10)])
for side in [-1,1]:
 # Swept stabilizer: crisp silhouette against the rounded pressure hull.
 a=(-1.9,.05,side*.85);b=(-1.25,-.1,side*2.55);c=(1.65,.05,side*.8)
 tri('green',a,b,c);tri('dark',tuple(v-(.09 if k==1 else 0) for k,v in enumerate(a)),c,b)
 tube('lime',b,c,.025)
 sphere('amber',b,(.08,.045,.08),16,8)
box('dark',(-.65,.68,0),(1.1,.16,.54))
for z in [-1,1]:
 # wing, mechanical supports and thrusters
 tube('metal',(-1.3,-.1,z*.6),(-1.7,-.15,z*1.7),.16)
 sphere('green',(-1.3,-.12,z*1.65),(1.5,.47,.48))
 tube('dark',(-2.65,-.12,z*1.65),(-2.05,-.12,z*1.65),.43,.49)
 torus('metal',(-2.64,-.12,z*1.65),.39,.075)
 torus('cyan',(-2.72,-.12,z*1.65),.30,.045)
 tube('cyan',(-2.74,-.12,z*1.65),(-3.15,-.12,z*1.65),.24,.19)
 # Tapered exhaust with visible compression diamonds, not a solid cone.
 for i in range(4):
  x=-3.1-i*.30;r=.13*(1-i/4.4)
  tube('flame',(x,-.12,z*1.65),(x-.14,-.12,z*1.65),r*.65,r)
  tube('cyan',(x-.14,-.12,z*1.65),(x-.30,-.12,z*1.65),r,r*.36)
 for x in [-2.15,-1.85,-1.55]:torus('metal',(x,-.12,z*1.65),.465,.024)
 # visible stripe and hull fasteners
 tube('lime',(-1.9,.25,z*.95),(1.8,.28,z*.85),.035)
 for x in [-1.8,-1.2,-.6,0,.6,1.2,1.8]:sphere('metal',(x,.04,z*1.02),(.043,.043,.025),12,8)
 sphere('white',(2.57,.35,z*.47),(.27,.13,.16))
 sphere('cyan',(2.75,.38,z*.47),(.12,.065,.1))
 # Bent legs, boots, arms, gloves.
 tube('dark',(.1,1.25,z*.30),(.85,.86,z*.65),.24,.21)
 tube('white',(.85,.86,z*.65),(.60,.22,z*.84),.20,.17)
 sphere('dark',(.85,.17,z*.86),(.43,.19,.23))
 sphere('fabric',(.05,2.12,z*.45),(.27,.28,.27))
 tube('fabric',(.05,2.1,z*.43),(.58,1.73,z*.58),.19,.16)
 tube('white',(.58,1.73,z*.58),(1.05,1.76,z*.48),.16,.13)
 sphere('dark',(1.12,1.77,z*.48),(.18,.14,.16))
 tube('metal',(1.13,1.57,z*.25),(1.13,1.77,z*.62),.045)
# Suit and helmet (visor is a forward inset ellipsoid).
sphere('fabric',(.03,1.75,0),(.43,.68,.48))
box('white',(.39,1.85,0),(.055,.28,.27))
box('dark',(.45,1.87,0),(.025,.17,.18))
for z in [-.1,0,.1]:sphere('cyan',(.48,1.87,z),(.02,.025,.025),12,8)
torus('metal',(.08,2.28,0),.32,.075,'y')
sphere('white',(.12,2.72,0),(.53,.56,.53))
sphere('visor',(.37,2.75,0),(.41,.38,.49))
sphere('gold',(.43,2.76,0),(.365,.31,.46))
sphere('visor',(.48,2.78,0),(.335,.265,.42))
for z in [-.53,.53]:
 sphere('dark',(.1,2.72,z),(.2,.21,.055))
 sphere('cyan',(.1,2.72,z*1.07),(.12,.12,.022))
tube('metal',(-.17,3.05,-.25),(-.2,3.5,-.25),.025)
sphere('amber',(-.2,3.52,-.25),(.065,.065,.065),16,8)
# Insulated box, rigid lid, white strips, latch, label.
bevelbox('rubber',(-1.38,1.35,0),(.67,.75,.68),.08)
bevelbox('green',(-1.38,1.40,0),(.64,.71,.65),.07)
bevelbox('green',(-1.38,2.12,0),(.69,.065,.70),.04)
for z in [-.64,.64]:
 box('white',(-1.38,.94,z),(.63,.055,.018))
 box('metal',(-1.38,2.00,z),(.09,.12,.025))
box('dark',(-1.38,2.24,0),(.23,.07,.10))
# Logo front-facing plane, baked as original text, not copied logo artwork.
a=(-1.96,1.12,.662);b=(-.8,1.12,.662);c=(-.8,1.87,.662);d=(-1.96,1.87,.662)
tri('label',a,b,c,uv=[(0,1),(1,1),(1,0)]);tri('label',a,c,d,uv=[(0,1),(1,0),(0,0)])
# A second printed panel on the rear face stays readable on the approach camera.
a=(-2.058,1.1,-.56);b=(-2.058,1.1,.56);c=(-2.058,1.88,.56);d=(-2.058,1.88,-.56)
tri('label',a,b,c,uv=[(0,1),(1,1),(1,0)]);tri('label',a,c,d,uv=[(0,1),(1,0),(0,0)])
# Cockpit console and windshield rim.
tube('dark',(1.25,.55,0),(1.3,1.45,0),.18)
box('metal',(1.30,1.46,0),(.20,.08,.54))
box('screen',(1.30,1.55,0),(.14,.015,.39))
# Mechanical layering: nozzle petals, coolant lines, vents, access panels,
# rivets, cargo locks, handgrip fingers, suit seams and oxygen hardware.
for side in [-1,1]:
 for i in range(18):
  a=i*2*math.pi/18;yy=-.12+math.cos(a)*.405;zz=side*1.65+math.sin(a)*.405
  tube('titanium',(-2.7,yy,zz),(-2.18,yy*1.08,zz),.028,.045,8)
 for x in [-1.85,-1.3,-.75]:
  bevelbox('titanium',(x,.29,side*1.94),(.19,.08,.11),.035)
  sphere('dark',(x,.30,side*2.05),(.065,.045,.015),16,8)
 for i in range(9):
  x=-1.95+i*.13
  box('rubber',(x,.62,side*.72),(.035,.012,.16))
 # exposed braided pipes and plumbing near the rear engines
 for k in range(12):
  t=k/12;tn=(k+1)/12
  def pipe(t):return (-2.35+1.5*t,.35+.15*math.sin(t*math.pi),side*(1.55+.07*math.sin(t*math.pi*2)))
  tube('brass',pipe(t),pipe(tn),.035,n=12)
 for x in [-.1,.3,.7,1.1,1.5,1.9,2.3]:
  # Panel seams follow the tapered flanks instead of floating off the hull.
  z=side*(.99-max(0,x)*.19)
  tube('rubber',(x,-.02,z),(x,.27,z),.012,n=8)
  sphere('titanium',(x,.19,z+side*.015),(.027,.027,.018),12,8)
 bevelbox('titanium',(.4,-.36,side*.8),(.65,.08,.16),.045)
 for i in range(7):box('rubber',(-.12+i*.16,-.275,side*.8),(.02,.01,.13))
 # Cargo corner guards, latches, webbing straps and stitching.
 for x in [-1.94,-.82]:
  bevelbox('titanium',(x,1.39,side*.65),(.055,.63,.05),.025)
  bevelbox('dark',(x,2.11,side*.65),(.11,.08,.085),.035)
 for x in [-1.78,-.98]:
  bevelbox('titanium',(x,1.98,side*.68),(.06,.12,.03),.02)
  box('rubber',(x,2.02,side*.715),(.02,.055,.01))
 # Knee armor and pressure-suit articulated joints.
 sphere('rubber',(.85,.88,side*.65),(.23,.22,.23))
 bevelbox('white',(.9,.9,side*.83),(.18,.18,.065),.06)
 for i in range(4):
  tube('rubber',(.43+i*.055,1.81-i*.025,side*.57),(.43+i*.055,1.81-i*.025,side*.70),.025,n=12)
  sphere('rubber',(1.19+i*.037,1.74,side*.52),(.035,.095,.045),16,8)
 bevelbox('rubber',(.85,.03,side*.86),(.40,.045,.23),.025)
 for i in range(7):box('titanium',(.54+i*.10,.01,side*1.08),(.02,.025,.014))
 # Harness across torso and helmet fasteners.
 tube('rubber',(-.05,2.21,side*.39),(.39,1.4,side*.28),.045,n=12)
 bevelbox('titanium',(.30,1.65,side*.35),(.055,.075,.045),.025)
 for a in range(0,360,45):
  r=math.radians(a)
  sphere('titanium',(.1+.17*math.cos(r),2.72+.17*math.sin(r),side*.585),(.025,.025,.015),12,8)
 # Oxygen hose running around the hip.
 for i in range(16):
  t=i/16;u=(i+1)/16
  def hose(t):return (.23-.6*t,1.9-.8*t,side*(.43+.18*math.sin(math.pi*t)))
  tube('brass',hose(t),hose(u),.04,n=12)
  sphere('rubber',hose(t),(.047,.047,.047),12,8)
# Visible chest fittings, helmet band, rectangular top lamp.
bevelbox('titanium',(.12,3.22,0),(.19,.055,.19),.035)
bevelbox('white',(.30,3.22,0),(.04,.035,.12),.02)
for z in [-.18,.18]:
 tube('titanium',(-.37,1.53,z),(-.37,2.11,z),.10,n=24)
 torus('dark',(-.37,1.63,z),.11,.02,'y',32)
# Destination planet: separate geometry, striped surface and atmospheric edge in GLSL.
sphere('planet',(5.8,4.5,-11),(3.4,3.4,3.4),128,64)
# Broad tilted annulus, actual vertices with gaps.
for inner,outer in [(4.25,4.8),(4.9,5.7),(5.8,6.05)]:
 for i in range(320):
  def rp(r,a):
   x=r*math.cos(a);z=r*math.sin(a)
   return (5.8+x,4.5+z*.32,-11+z*.95)
  a=2*math.pi*i/320;b=2*math.pi*(i+1)/320
  tri('ring',rp(inner,a),rp(inner,b),rp(outer,b));tri('ring',rp(inner,a),rp(outer,b),rp(outer,a))
sphere('moon',(-1,3,-13),(.7,.7,.7))
# Orbital receiving station between the courier and the inhabited world.
torus('stationMetal',(5.3,1.65,-6.7),.55,.065,'x')
torus('stationLight',(5.28,1.65,-6.7),.48,.018,'x')
tube('stationMetal',(5.3,.9,-6.7),(5.3,2.5,-6.7),.12)
for side in [-1,1]:
 tube('stationMetal',(5.3,1.7,-6.7),(5.3+side*1.4,1.7,-6.7),.025)
 for i in range(3):
  bevelbox('solar',(5.3+side*(.75+i*.28),1.72,-6.7),(.12,.025,.42),.015)
  for j in range(5):box('stationMetal',(5.3+side*(.75+i*.28),1.75,-7.06+j*.17),(.12,.006,.004))
random.seed(23)
for i in range(24):
 x=random.uniform(-10,10);y=random.uniform(-4,-2.3);z=random.uniform(-8,-2)
 s=random.uniform(.045,.20);sphere('moon',(x,y,z),(s,s*.7,s),10,6)
manifest=[]
for name,verts in groups.items():
 if name in ['planet','ring']:
  # Pull destination inside the frame, retaining concentric ring geometry.
  verts=[(tuple((p[k]-[5.8,4.5,-11][k])*.98+[7,4,-10][k] for k in range(3)),n,uv) for p,n,uv in verts]
 elif name=='moon':
  verts=[((p[0]-7,p[1]-1,p[2]),n,uv) if p[1]>0 else (p,n,uv) for p,n,uv in verts]
 elif name not in ['stationMetal','stationLight','solar']:
  # Nose (+X) points precisely toward the destination planet from ship origin.
  forward=(8,4.35,-11);ln=math.sqrt(sum(x*x for x in forward));forward=tuple(x/ln for x in forward)
  side=( -forward[2],0,forward[0]);ln=math.sqrt(sum(x*x for x in side));side=tuple(x/ln for x in side)
  up=(side[1]*forward[2]-side[2]*forward[1],side[2]*forward[0]-side[0]*forward[2],side[0]*forward[1]-side[1]*forward[0])
  def rotate(p):return tuple(forward[k]*p[0]+up[k]*p[1]+side[k]*p[2] for k in range(3))
  verts=[(tuple(rotate(p)[k]*1.08+[-1,-.35,1][k] for k in range(3)),rotate(n),uv) for p,n,uv in verts]
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
im=Image.new('RGBA',(512,320),(5,125,47,255));d=ImageDraw.Draw(im)
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

