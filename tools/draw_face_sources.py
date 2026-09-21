"""Original rounded sprite artwork. Export editable SVG parts, not runtime drawing."""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]/'assets/images/xiaozhi-face'
NAMES='neutral happy laughing funny sad angry crying loving embarrassed surprised shocked thinking winking cool relaxed delicious kissy confident sleepy silly confused'.split()
def ellipse(x,y,rx,ry,fill='white'):
    return f'<ellipse cx="{x}" cy="{y}" rx="{rx}" ry="{ry}" fill="{fill}"/>'
def rect(x,y,w,h,r=0,fill='white'):
    return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{fill}"/>'
def line(points,width=3,fill='white'):
    return '<polyline points="'+' '.join(f'{x:.2f},{y:.2f}' for x,y in points)+f'" fill="none" stroke="{fill}" stroke-width="{width}" stroke-linecap="round" stroke-linejoin="round"/>'
def poly(points):return '<polygon points="'+' '.join(f'{x},{y}' for x,y in points)+'"/>'
def curve(a,b,c,width=3):
    return line([((1-t)**2*a[0]+2*t*(1-t)*b[0]+t*t*c[0],(1-t)**2*a[1]+2*t*(1-t)*b[1]+t*t*c[1]) for t in [i/24 for i in range(25)]],width)
def eye(x,y=16,rx=7,ry=10):return ellipse(x,y,rx,ry)+ellipse(x-2,y-4,2.5,3,'black')
def smile_eye(x):return curve((x-8,19),(x,4),(x+8,19),4)
def shut_eye(x):return curve((x-8,15),(x,24),(x+8,15),3)
def heart(x,y,s=1):
    return ellipse(x-4*s,y-2*s,5*s,5*s)+ellipse(x+4*s,y-2*s,5*s,5*s)+poly([(x-8*s,y),(x,y+10*s),(x+8*s,y)])
def svg(name,w,h,body):
    (ROOT/(name+'.svg')).write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" fill="white">{body}</svg>\n',encoding='utf-8')
def main():
    ROOT.mkdir(parents=True,exist_ok=True)
    svg('body',144,132,ellipse(27,34,18,22)+ellipse(117,34,18,22)+ellipse(72,77,66,49)+ellipse(20,122,12,7)+ellipse(124,122,12,7)+curve((60,31),(55,7),(73,7),6)+curve((73,7),(84,12),(72,18),6))
    eyes={
      'neutral':eye(16)+eye(64),
      'happy':smile_eye(16)+smile_eye(64),
      'laughing':line([(7,9),(22,17),(7,25)],4)+line([(73,9),(58,17),(73,25)],4),
      'funny':smile_eye(16)+eye(64,14,8,11),
      'sad':eye(16,20,6,7)+eye(64,20,6,7)+line([(7,9),(23,3)],3)+line([(57,3),(73,9)],3),
      'angry':poly([(7,8),(25,16),(24,26),(8,22)])+poly([(55,16),(73,8),(72,22),(56,26)]),
      'crying':curve((6,15),(16,3),(26,15),4)+curve((54,15),(64,3),(74,15),4)+line([(7,7),(25,1)],2)+line([(55,1),(73,7)],2),
      'loving':heart(16,12)+heart(64,12),
      'embarrassed':ellipse(21,18,4,7)+ellipse(69,18,4,7)+line([(7,7),(24,9)],2)+line([(55,9),(72,7)],2),
      'surprised':eye(16,16,9,12)+eye(64,16,9,12),
      'shocked':ellipse(16,16,12,14)+ellipse(64,16,12,14)+ellipse(16,16,8,10,'black')+ellipse(64,16,8,10,'black')+ellipse(16,16,3,5)+ellipse(64,16,3,5),
      'thinking':ellipse(11,20,5,7)+ellipse(59,20,5,7)+curve((6,8),(16,1),(26,6),3)+line([(55,8),(73,8)],3),
      'winking':eye(16)+smile_eye(64),
      'cool':rect(2,6,31,20,6)+rect(47,6,31,20,6)+rect(31,9,18,4,1)+line([(8,11),(15,11)],2,'black')+line([(53,11),(60,11)],2,'black'),
      'relaxed':shut_eye(16)+shut_eye(64),
      'delicious':smile_eye(16)+smile_eye(64)+ellipse(28,23,2,2)+ellipse(52,23,2,2),
      'kissy':shut_eye(16)+line([(73,9),(58,17),(73,23)],3),
      'confident':rect(8,14,17,9,4)+rect(55,14,17,9,4)+line([(8,7),(25,7)],2)+line([(55,7),(72,2)],2),
      'sleepy':line([(7,20),(25,20)],3)+line([(55,20),(73,20)],3)+line([(8,9),(24,11)],2)+line([(56,11),(72,9)],2),
      'silly':eye(16,15,8,11)+line([(56,9),(71,16),(56,24)],4),
      'confused':ellipse(16,16,11,12)+ellipse(64,16,11,12)+ellipse(16,17,8,9,'black')+ellipse(64,17,8,9,'black')+ellipse(16,9,4,5)+ellipse(64,9,4,5)}
    mouths={
      'neutral':curve((16,11),(24,18),(32,11),2.5),
      'happy':curve((12,8),(24,27),(36,8),3),
      'laughing':ellipse(24,12,12,11)+rect(15,3,18,5,2,'black'),
      'funny':ellipse(25,13,11,9)+ellipse(29,18,6,3,'black'),
      'sad':curve((15,17),(24,5),(33,17),3),
      'angry':line([(15,15),(24,10),(33,15)],3),
      'crying':ellipse(24,15,10,8)+ellipse(24,20,6,3,'black'),
      'loving':curve((13,8),(24,27),(35,8),3)+ellipse(24,17,6,5),
      'embarrassed':line([(17,13),(22,11),(27,15),(32,12)],2.5),
      'surprised':ellipse(24,12,5,8),
      'shocked':ellipse(24,12,8,12)+ellipse(24,12,5,9,'black'),
      'thinking':line([(21,13),(31,13)],3),
      'winking':curve((14,10),(26,22),(35,6),3),
      'cool':curve((16,11),(25,21),(35,9),3),
      'relaxed':curve((18,12),(24,17),(30,12),2),
      'delicious':curve((13,9),(25,26),(36,9),3)+ellipse(32,16,4,7),
      'kissy':line([(19,6),(29,11),(20,15),(29,19),(19,23)],3),
      'confident':curve((16,15),(28,20),(35,7),3),
      'sleepy':ellipse(24,14,4,5),
      'silly':curve((12,8),(24,22),(36,8),3)+rect(21,12,10,13,4)+line([(26,15),(26,19)],1,'black'),
      'confused':line([(17,16),(30,12)],3)}
    for name in NAMES:svg('eyes_'+name,80,32,eyes[name]);svg('mouth_'+name,48,26,mouths[name])
    svg('eyes_blink',80,32,shut_eye(16)+shut_eye(64))
    svg('eyes_cool_round',80,32,ellipse(16,16,13,13)+ellipse(64,16,13,13)+rect(28,10,24,3,1)+line([(10,10),(17,10)],2,'black')+line([(58,10),(65,10)],2,'black'))
    svg('eyes_cool_sport',80,32,poly([(2,6),(37,11),(30,25),(9,21)])+poly([(43,11),(78,6),(71,21),(50,25)])+rect(34,11,12,3)+line([(9,10),(22,12)],2,'black')+line([(57,12),(70,10)],2,'black'))
    svg('eyes_angry_puff',80,32,ellipse(16,21,6,7)+ellipse(64,21,6,7)+line([(6,5),(26,13)],4)+line([(54,13),(74,5)],4))
    svg('eyes_wink_left',80,32,smile_eye(16)+eye(64))
    for i,(rx,ry) in enumerate([(5,4),(8,7),(10,11)]):svg('talk_'+str(i),48,26,ellipse(24,13,rx,ry))
    svg('decor_cheek',14,7,ellipse(7,3.5,7,3.5))
    svg('decor_tear',12,27,rect(2,0,8,22,4)+ellipse(6,22,5,5))
    svg('decor_heart',20,22,heart(10,9))
    svg('decor_sweat',12,20,poly([(6,0),(1,12),(11,12)])+ellipse(6,13,5,6))
    svg('decor_question',16,24,curve((3,6),(7,-1),(13,5),3)+curve((13,5),(16,10),(8,13),3)+line([(8,13),(8,16)],3)+ellipse(8,22,1.8,1.8))
    svg('decor_zzz',19,24,line([(2,12),(10,12),(2,21),(10,21)],2)+line([(10,2),(17,2),(10,9),(17,9)],2))
    svg('decor_star',18,20,poly([(9,0),(12,7),(18,10),(12,13),(9,20),(6,13),(0,10),(6,7)]))
    svg('decor_hand',22,24,ellipse(11,15,9,8)+rect(4,1,7,17,3)+line([(7,11),(7,18)],1.5,'black'))
    print('Original SVG sources: 21 expressions plus shared character and accent parts')
if __name__=='__main__':main()
