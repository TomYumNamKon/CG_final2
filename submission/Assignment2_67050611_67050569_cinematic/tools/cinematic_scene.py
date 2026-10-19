"""Original mechanical geometry for the cinematic OpenGL scene (not a backdrop)."""
import math, random

def build_scene(g):
    sphere,box,bevel,tube,torus,tri=[g[k] for k in ['sphere','box','bevelbox','tube','torus','tri']]
    groups=g['groups']; mats=g['materials']
    mats.update({
        'armor':((.045,.09,.052),0,.65,10),
        'armorLight':((.085,.13,.078),0,.60,10),
        'armorDark':((.022,.042,.031),0,.65,10),
        'eva':((.55,.51,.41),0,.05,11),
        'evaWhite':((.72,.71,.63),0,.12,11),
        'visorGold':((.72,.43,.12),0,.98,12),
        'marking':((1,1,1),0,.2,16),
        'atmosphere':((.10,.40,.85),0,0,14),
        'rock':((.13,.11,.095),0,.05,15),
        'cyan':((.07,.48,1),5,.1,0),
        'amber':((1,.30,.035),2,.1,0),
    })
    def rotated_box(mat,p,s,angle=0,axis=0,r=.035):
        start=len(groups.get(mat,[]));bevel(mat,(0,0,0),s,r)
        c=math.cos(angle);sn=math.sin(angle)
        def rot(v):
            q=list(v);a=(axis+1)%3;b=(axis+2)%3
            q[a]=c*v[a]-sn*v[b];q[b]=sn*v[a]+c*v[b];return tuple(q)
        groups[mat][start:]=[(tuple(rot(v)[k]+p[k] for k in range(3)),rot(n),uv) for v,n,uv in groups[mat][start:]]
    def panel(mat,vs,thick=.06):
        tri(mat,vs[0],vs[1],vs[2]);tri(mat,vs[0],vs[2],vs[3])
        for i in range(4):
            a=vs[i];b=vs[(i+1)%4];c=(b[0],b[1]-thick,b[2]);d=(a[0],a[1]-thick,a[2])
            tri('titanium',a,b,c);tri('titanium',a,c,d)
    def limb(a,b,r):
        tube('eva',a,b,r,r*.88,32)
        sphere('eva',a,(r,r,r),24,16);sphere('eva',b,(r*.9,r*.9,r*.9),24,16)
    def hose(points,mat='rubber',r=.025):
        for a,b in zip(points,points[1:]):tube(mat,a,b,r,n=10)
    # Load-bearing keel and armored octagonal hull, with true geometric panel gaps.
    bevel('dark',(-.1,-.12,0),(3.1,.32,.68),.16)
    bevel('titanium',(-.4,-.42,0),(2.7,.10,.54),.065)
    sections=[(-3.15,.82,.33),(-2.5,1.07,.55),(-1.2,1.04,.60),(.1,.95,.53),(1.2,.79,.42),(2.3,.53,.28),(3.3,.22,.13),(3.65,.025,.035)]
    for j in range(len(sections)-1):
        x,w,h=sections[j];xx,ww,hh=sections[j+1];gap=.025
        for side in [-1,1]:
            panel('armorLight' if j%3==0 else 'armor',[(x+gap,h*.45,side*w),(xx-gap,hh*.45,side*ww),(xx-gap,hh,side*ww*.58),(x+gap,h,side*w*.58)])
            panel('armorDark',[(x+gap,-h*.6,side*w*.9),(xx-gap,-hh*.6,side*ww*.9),(xx-gap,hh*.45,side*ww),(x+gap,h*.45,side*w)])
            for t in [.14,.83]:
                px=x+(xx-x)*t;ph=h+(hh-h)*t;pw=w+(ww-w)*t
                sphere('titanium',(px,ph*.63,side*pw*.85),(.027,.018,.027),10,8)
        panel('armor',[(x+gap,h,-w*.55),(xx-gap,hh,-ww*.55),(xx-gap,hh,ww*.55),(x+gap,h,w*.55)])
    # Seat, floor and control console, within a protective angular cockpit.
    bevel('rubber',(-.2,.68,0),(.8,.12,.42),.10)
    rotated_box('rubber',(-.72,1.03,0),(.10,.43,.43),-.16,2,.07)
    for side in [-1,1]:
        bevel('titanium',(.4,.12,side*.85),(.73,.09,.24),.04)
        for i in range(9):box('rubber',(-.22+i*.15,.22,side*.85),(.033,.015,.20))
        panel('armorLight',[(.75,.48,side*.64),(2.25,.35,side*.52),(1.65,1.25,side*.44),(.98,1.05,side*.58)])
        hose([(.8,.7,side*.62),(1.05,1.12,side*.55),(1.7,1.30,side*.42),(2.25,.35,side*.52)],'titanium',.035)
    rotated_box('dark',(1.15,1.28,0),(.28,.08,.44),-.28,2)
    rotated_box('screen',(1.15,1.37,0),(.19,.014,.32),-.28,2,.01)
    for side in [-1,1]:
        tube('titanium',(.97,1.18,side*.2),(.98,1.43,side*.55),.04,n=16)
        tube('rubber',(.98,1.43,side*.43),(.98,1.43,side*.67),.065,n=24)
        for i in range(5):sphere('amber' if i==0 else 'metal',(1.38,1.36,-.26+i*.12),(.025,.02,.025),12,8)
    # Two large armored propulsion nacelles; nozzle interiors remain hollow.
    for side in [-1,1]:
        z=side*1.48
        tube('dark',(-3.35,-.10,z),(-.8,-.10,z),.54,.47,48)
        for x,r in [(-3.4,.58),(-3.23,.56),(-2.95,.55),(-2.6,.55),(-1.6,.50)]:
            torus('titanium',(x,-.1,z),r,.045,n=64)
        torus('cyan',(-3.43,-.1,z),.43,.025,n=64)
        torus('brass',(-3.35,-.1,z),.49,.018,n=64)
        for i in range(28):
            a=2*math.pi*i/28;dy=math.cos(a);dz=math.sin(a)
            tube('titanium',(-3.44,-.1+dy*.51,z+dz*.51),(-2.90,-.1+dy*.44,z+dz*.44),.035,.045,n=8)
        # Recessed turbine seen through the open nozzle.
        for i in range(18):
            a=i*2*math.pi/18;b=a+.2
            tri('metal',(-2.98,-.1+math.cos(a)*.14,z+math.sin(a)*.14),(-3.04,-.1+math.cos(a)*.43,z+math.sin(a)*.43),(-3.01,-.1+math.cos(b)*.40,z+math.sin(b)*.40))
        sphere('cyan',(-3.09,-.1,z),(.06,.19,.19),24,16)
        for i,x in enumerate([-2.6,-1.85,-1.10]):
            rotated_box('armor' if i!=1 else 'armorLight',(x,.41,z),(.34,.11,.45),side*.09,0,.05)
            bevel('dark',(x,.54,z),(.25,.035,.27),.025)
            for j in range(7):box('metal',(x-.21+j*.07,.58,z),(.017,.023,.24))
            rotated_box('armor',(x,-.03,z+side*.50),(.32,.24,.045),0,0,.025)
            for dx in [-.24,.24]:sphere('titanium',(x+dx,.14,z+side*.55),(.026,.026,.015),12,8)
        tube('titanium',(-1.85,-.2,side*.6),(-1.85,-.2,z),.15,n=16)
        tube('brass',(-1.5,-.1,side*.65),(-1.5,-.1,z),.075,n=16)
        # Swept wing with layered plates, structural spars, and navigation light.
        vs=[(-2.55,.13,z),(-1.90,-.03,side*2.25),(.85,.18,side*.91),(-.6,.45,side*.92)]
        panel('armor',vs,.10)
        tube('titanium',vs[1],vs[2],.032,n=12)
        sphere('amber',vs[1],(.065,.03,.055),16,10)
        for k in range(3):
            points=[(-2.8+k*.1,.32,z+side*.27),(-2.3,.72+k*.055,z*.90),(-1.2,.72+k*.055,side*.98),(-.75,.45,side*.75)]
            hose(points,'brass' if k==0 else 'rubber',.027)
        # Radiators and service hatches along each side of the central hull.
        for x in [-2.3,-1.5,-.7,.1]:
            bevel('titanium',(x,-.02,side*1.04),(.25,.13,.04),.025)
            bevel('dark',(x,.00,side*1.09),(.20,.09,.012),.01)
            for j in range(5):box('metal',(x-.16+j*.08,.0,side*1.11),(.016,.075,.014))
        # Small headlight, not a giant glowing toy eye.
        bevel('dark',(2.7,.22,side*.28),(.18,.075,.09),.035)
        bevel('white',(2.8,.24,side*.28),(.045,.025,.055),.015)
    # Shipping crate: worn enamel, framed corners, hinges and mounting hardware.
    bevel('rubber',(-1.80,1.35,0),(.77,.73,.68),.065)
    bevel('armor',(-1.8,1.38,0),(.74,.69,.65),.055)
    bevel('armorLight',(-1.8,2.08,0),(.80,.065,.71),.035)
    for side in [-1,1]:
        for x in [-2.49,-1.11]:
            bevel('titanium',(x,1.38,side*.655),(.045,.65,.045),.015)
            for y in [.78,1.97]:
                bevel('dark',(x,y,side*.68),(.095,.11,.055),.025)
                sphere('brass',(x,y,side*.741),(.025,.025,.015),12,8)
        for y in [.78,1.98]:bevel('metal',(-1.8,y,side*.66),(.69,.028,.025),.015)
        for x in [-2.18,-1.42]:
            bevel('dark',(x,2.10,side*.67),(.08,.13,.055),.025)
            bevel('brass',(x,2.10,side*.73),(.027,.065,.018),.01)
        for x in [-2.35,-1.25]:tube('dark',(x,.66,side*.6),(x,.45,side*.78),.065,n=16)
    bevel('dark',(-1.8,2.21,0),(.25,.07,.09),.025)
    # Printed lettering occupies painted faces rather than a bright green sticker.
    for vs in [[(-2.42,1.02,.692),(-1.18,1.02,.692),(-1.18,1.82,.692),(-2.42,1.82,.692)],
               [(-2.578,1.02,-.56),(-2.578,1.02,.56),(-2.578,1.82,.56),(-2.578,1.82,-.56)]]:
        tri('label',*vs[:3],uv=[(0,1),(1,1),(1,0)]);tri('label',vs[0],vs[2],vs[3],uv=[(0,1),(1,0),(0,0)])
    # Human-scaled pressure suit, forward lean, layered shoulder/hip bellows.
    sphere('eva',(.05,1.65,0),(.33,.54,.39),48,32)
    sphere('rubber',(-.1,1.13,0),(.36,.25,.36),32,20)
    bevel('evaWhite',(-.33,1.73,0),(.16,.43,.34),.085)
    bevel('titanium',(-.50,1.75,0),(.025,.32,.26),.015)
    bevel('evaWhite',(-.533,1.80,0),(.015,.18,.20),.012)
    for j in range(5):box('rubber',(-.552,1.48+j*.035,0),(.009,.008,.18))
    for side in [-1,1]:
        bevel('armor',(-.51,1.94,side*.23),(.03,.085,.04),.012)
    for side in [-1,1]:
        limb((-.05,1.12,side*.24),(.64,.82,side*.51),.20)
        limb((.64,.82,side*.51),(.51,.26,side*.70),.17)
        sphere('rubber',(.64,.84,side*.52),(.19,.18,.19),24,16)
        bevel('evaWhite',(.70,.87,side*.66),(.16,.13,.045),.035)
        bevel('rubber',(.68,.16,side*.72),(.30,.12,.18),.075)
        for j in range(6):box('metal',(.45+j*.09,.07,side*.88),(.018,.02,.018))
        limb((.16,1.98,side*.35),(.52,1.60,side*.47),.16)
        limb((.52,1.60,side*.47),(.94,1.46,side*.53),.135)
        sphere('evaWhite',(.14,2.00,side*.36),(.20,.19,.19),32,20)
        sphere('rubber',(.52,1.60,side*.47),(.145,.145,.145),24,16)
        sphere('eva',(.96,1.45,side*.54),(.14,.10,.12),24,16)
        for j in range(4):limb((1+j*.035,1.45,side*.59),(1+j*.035,1.36,side*.57),.028)
        # Wrinkled pressure-joint ribs and visible suit stitching.
        for j in range(6):
            t=j/6
            sphere('eva',(.25+t*.23,1.86-t*.23,side*.48),(.14,.035,.035),16,10)
            tube('dark',(.25+t*.23,1.86-t*.23,side*.49),(.25+t*.23,1.86-t*.23,side*.51),.015,n=8)
        hose([(-.15,2.06,side*.31),(.33,1.79,side*.34),(.31,1.32,side*.27)],'rubber',.035)
        bevel('metal',(.29,1.54,side*.31),(.045,.07,.025),.015)
        points=[]
        for j in range(20):
            t=j/19;points.append((.29-.72*t,1.74-.55*t,side*(.36+.13*math.sin(t*math.pi))))
        hose(points,'brass',.032)
        for p in points:sphere('rubber',p,(.038,.038,.038),10,8)
    # Reflective gold visor wraps around the shell so it reads from the side.
    headStarts={name:len(vertices) for name,vertices in groups.items()}
    center=(.30,2.46,0)
    torus('titanium',(.22,2.15,0),.27,.045,'y',64)
    sphere('evaWhite',center,(.39,.44,.39),64,40)
    def visor(phi,theta):
        q=(math.cos(phi)*math.cos(theta),math.sin(phi),math.cos(phi)*math.sin(theta))
        scale=(.413,.38,.412);n=tuple(q[k]/scale[k] for k in range(3));ln=math.sqrt(sum(v*v for v in n))
        return tuple(center[k]+q[k]*scale[k] for k in range(3)),tuple(v/ln for v in n)
    for j in range(22):
        for i in range(64):
            vs=[visor(-.55+1.15*b/22,-1.88+3.76*a/64) for a,b in [(i,j),(i+1,j),(i+1,j+1),(i,j+1)]]
            for idx in [(0,1,2),(0,2,3)]:tri('visorGold',*[vs[k][0] for k in idx],[vs[k][1] for k in idx])
    for phi in [-.55,.60]:hose([visor(phi,-1.88+3.76*i/64)[0] for i in range(65)],'rubber',.018)
    for side in [-1,1]:
        sphere('dark',(.11,2.46,side*.38),(.12,.14,.04),24,16)
        sphere('titanium',(.11,2.46,side*.418),(.085,.10,.02),24,16)
        for j in range(6):
            a=j*math.pi/3;sphere('brass',(.11+.075*math.cos(a),2.46+.09*math.sin(a),side*.441),(.013,.013,.01),10,8)
    bevel('evaWhite',(.17,2.88,0),(.15,.04,.13),.025)
    # Rider checks the approach over one shoulder; expose the gold visor profile.
    c=math.cos(.45);s=math.sin(.45)
    def turn(v,point=False):
        x=v[0]-(center[0] if point else 0);z=v[2]
        return (c*x-s*z+(center[0] if point else 0),v[1],s*x+c*z)
    for name,vertices in groups.items():
        start=headStarts.get(name,0)
        vertices[start:]=[(turn(p,True),turn(n),uv) for p,n,uv in vertices[start:]]
    # Inhabited destination, atmospheric shell, and geometrically broad rings.
    pc=(18,6,-22);radius=14
    sphere('planet',pc,(radius,)*3,192,96)
    sphere('atmosphere',pc,(radius*1.012,)*3,144,72)
    for inner,outer in [(1.37,1.60),(1.63,1.98),(2.02,2.08)]:
        for i in range(512):
            def rp(r,a):
                x=radius*r*math.cos(a);z=radius*r*math.sin(a)
                return (pc[0]+x,pc[1]+z*.12-x*.35,pc[2]+z*.99)
            a=math.tau*i/512;b=math.tau*(i+1)/512
            tri('ring',rp(inner,a),rp(inner,b),rp(outer,b));tri('ring',rp(inner,a),rp(outer,b),rp(outer,a))
    sphere('moon',(-10,6,-21),(1.25,)*3,48,32)
    # Irregular triangulated rocks: deterministic noise distorts actual vertices.
    random.seed(86)
    for j in range(14):
        p=(random.uniform(-13,18),random.uniform(-8,-3),random.uniform(-24,0));s=random.uniform(.10,.38)
        start=len(groups.get('rock',[]));sphere('rock',p,(s,s*.8,s*1.15),12,8)
        old=groups['rock'][start:];new=[]
        for k in range(0,len(old),3):
            pts=[]
            for v,n,uv in old[k:k+3]:
                q=tuple((v[a]-p[a])/s for a in range(3));h=1+.16*math.sin(q[0]*13+q[1]*7)*math.cos(q[2]*9)
                pts.append(tuple(p[a]+(v[a]-p[a])*h for a in range(3)))
            u=[pts[1][a]-pts[0][a] for a in range(3)];v=[pts[2][a]-pts[0][a] for a in range(3)]
            n=(u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]);ln=math.sqrt(sum(t*t for t in n))
            if ln<1e-8:continue
            n=tuple(t/ln for t in n)
            new.extend((p,n,uv) for p,(_,_,uv) in zip(pts,old[k:k+3]))
        groups['rock'][start:]=new
