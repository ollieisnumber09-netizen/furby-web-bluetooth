# Generates Dino's 20x20 sprite frames (facing right) as C string arrays.
from PIL import Image
N=20
def tri(g,pts,c):
    (x0,y0),(x1,y1),(x2,y2)=pts
    def s(ax,ay,bx,by,px,py): return (px-bx)*(ay-by)-(ax-bx)*(py-by)
    for y in range(N):
        for x in range(N):
            px,py=x+.5,y+.5
            d1=s(px,py,x0,y0,x1,y1);d2=s(px,py,x1,y1,x2,y2);d3=s(px,py,x2,y2,x0,y0)
            if not ((d1<0 or d2<0 or d3<0) and (d1>0 or d2>0 or d3>0)): g[y][x]=c
def frame(kind):
    g=[['.']*N for _ in range(N)]
    b=1 if kind=='run2' else 0          # bob
    cx,cy,rx0,ry=9.0,10.0+b,4.9,6.6
    # spikes on his back (left), straight triangles, top one sweeps back from the top of the head
    tri(g,[(0.6,0.2+b),(10.5,4.2+b),(5.0,9.6+b)],'g')
    tri(g,[(0.0,11.6+b),(5.6,8.4+b),(5.0,15.4+b)],'g')
    for y in range(N):
        for x in range(N):
            px,py=x+.5,y+.5
            t=(py-cy)/ry
            rx=rx0+0.5*max(0,t)
            if t*t+((px-cx)/rx)**2<=1.0: g[y][x]='w'
    legs={'idle':[(5,15,4,3),(10,15,4,3)],'blink':[(5,15,4,3),(10,15,4,3)],
          'run1':[(4,15,4,3),(11,14,4,3)],'run2':[(6,15,4,3),(9,15,4,3)],
          'jump':[(5,14,4,3),(10,14,4,3)],'hurt':[(4,15,4,3),(11,15,4,3)]}[kind]
    for (lx,ly,lw,lh) in legs:
        for y in range(ly,ly+lh):
            for x in range(lx,lx+lw):
                if 0<=y<N: g[y][x]='w'
    if kind in('idle','blink','hurt','jump'):
        ly=legs[0][1]+legs[0][2]-1
        g[ly][9]='.'
        if kind!='jump': g[ly-1][9]='.'
    filled={(x,y) for y in range(N) for x in range(N) if g[y][x]!='.'}
    for (x,y) in list(filled):
        for dx,dy in((1,0),(-1,0),(0,1),(0,-1)):
            a,c=x+dx,y+dy
            if 0<=a<N and 0<=c<N and (a,c) not in filled: g[c][a]='k'
    # black line where spikes meet the body
    for y in range(N):
        for x in range(N):
            if g[y][x]=='g' and any(0<=x+dx<N and 0<=y+dy<N and g[y+dy][x+dx]=='w' for dx,dy in((1,0),(0,1))):
                g[y][x]='k'
    ey=6+b
    if kind=='blink':
        g[ey+3][10]='k'; g[ey+3][12]='k'
    elif kind=='hurt':
        for (x,y) in [(9,ey+1),(11,ey+1),(10,ey+2),(9,ey+3),(11,ey+3)]: g[y][x]='k'
        for (x,y) in [(12,ey+1),(14,ey+1),(13,ey+2),(12,ey+3),(14,ey+3)]: g[y][x]='k'
    else:
        for y in range(ey,ey+5): g[y][10]='k'; g[y][12]='k'
    my=ey+6
    if kind=='hurt':
        for x in range(10,13): g[my][x]='k'
    else:
        g[my][9]='k';g[my][11]='k';g[my][13]='k';g[my+1][10]='k';g[my+1][12]='k'
    # floating circle hand in front
    hy={'idle':9,'blink':9,'run1':8,'run2':10,'jump':4,'hurt':3}[kind]+b
    hx=16
    for (x,y) in [(hx,hy),(hx+1,hy),(hx,hy+1),(hx+1,hy+1)]: g[y][x]='w'
    for (x,y) in [(hx-1,hy),(hx-1,hy+1),(hx+2,hy),(hx+2,hy+1),(hx,hy-1),(hx+1,hy-1),(hx,hy+2),(hx+1,hy+2)]:
        if 0<=x<N and 0<=y<N: g[y][x]='k'
    return [''.join(r) for r in g]
pal={'.':(0,0,0,0),'k':(20,20,20,255),'w':(255,255,255,255),'g':(76,140,43,255)}
kinds=['idle','blink','run1','run2','jump','hurt']
out=open('dino_sprites.h','w')
out.write('// Dino, 20x20, facing right. Made by dino/gen.py.\n')
im=Image.new('RGBA',((N+2)*len(kinds),N),(120,190,240,255))
for i,k in enumerate(kinds):
    f=frame(k)
    out.write('static const char* SPR_DINO_%s[20] = {\n'%k.upper()+''.join('  "%s",\n'%r for r in f)+'};\n')
    for y,r in enumerate(f):
        for x,c in enumerate(r):
            if c!='.': im.putpixel((i*(N+2)+x,y),pal[c])
im.resize((im.width*10,im.height*10),Image.NEAREST).save('preview.png')
