"""Original cartoon human portrait SVG parts; stable abstract_* asset IDs."""
from draw_face_sources import NAMES, ROOT, ellipse, rect, line, poly, curve, heart

def svg(name,w,h,body):
    (ROOT/f'abstract_{name}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" fill="white">{body}</svg>\n',encoding='utf-8')

def cubic(a,b,c,d):
    return [((1-t)**3*a[0]+3*(1-t)**2*t*b[0]+3*(1-t)*t*t*c[0]+t**3*d[0],(1-t)**3*a[1]+3*(1-t)**2*t*b[1]+3*(1-t)*t*t*c[1]+t**3*d[1]) for t in [i/24 for i in range(25)]]

def path(start,segments):
    result=[start]
    for segment in segments:
        if len(segment)==2:result.append(segment)
        else:result+=cubic(result[-1],*segment)[1:]
    return result

def pupil(x,y=18,rx=3.3,ry=5.6):
    return ellipse(x,y,rx,ry)+ellipse(x-.8,y-1.8,.9,1.4,'black')
def brow(x,raised=False):return curve((x-8,7 if raised else 9),(x,0 if raised else 5),(x+7,5 if raised else 8),2.3)
def eye(x):return pupil(x)+brow(x)
def happy(x):return curve((x-7,20),(x,8),(x+7,20),2.7)
def shut(x):return curve((x-7,16),(x,23),(x+7,16),2.3)
def soft_wink(x):return curve((x-7,17),(x,23),(x+7,16),2.7)
def worry(x,left):
    return curve((x-8,9 if left else 4),(x,7),(x+8,4 if left else 9),2.5)
def surprised(x,strong=False):
    return pupil(x,19,4 if strong else 3.3,7.5 if strong else 6.3)+brow(x,True)
def glasses(round=False,sport=False):
    if round:base=ellipse(22,18,11,11)+ellipse(62,18,11,11)
    elif sport:base=poly(path((9,8),[((18,8),(31,9),(34,12)),((33,27),(14,28),(9,8))]))+poly(path((49,12),[((55,9),(69,8),(76,8)),((71,28),(50,27),(49,12))]))
    else:base=rect(9,8,26,21,7)+rect(49,8,26,21,7)
    base+=curve((34,13),(42,9),(50,13),3)+line([(3,11),(11,13)],2.5)+line([(73,13),(79,11)],2.5)
    return base+curve((15,13),(18,11),(23,12),1.8).replace('white','black')+curve((55,13),(58,11),(63,12),1.8).replace('white','black')

def main():
    # A continuous bald silhouette. Broad soft reflection and a short glint
    # sit above the eyebrows and never animate independently of the face.
    face=path((36,64),[
        ((31,30),(48,8),(77,8)),((107,7),(125,31),(122,66)),
        ((125,94),(111,119),(90,127)),((69,136),(45,118),(37,94)),
        ((33,85),(33,74),(36,64))])
    svg('body',144,132,poly(face)+ellipse(30,77,11,14)+ellipse(123,77,7,12))
    nose=line(path((79,73),[((78,77),(82,81),(81,83)),((80,86),(77,86),(75,84))]),2.2)
    ear=curve((30,71),(23,71),(28,81),2.4)
    svg('detail',144,132,nose+ear)
    gloss=ellipse(63,29,20,12,'#202020')+ellipse(62,27,15,8,'#404040')+ellipse(60,25,10,5,'#707070')
    gloss+=line(path((44,42),[((47,27),(56,21),(65,21))]),3.7)
    gloss+=poly([(112,15),(114,20),(119,22),(114,24),(112,29),(110,24),(105,22),(110,20)])
    svg('bald_gloss',144,132,gloss)
    svg('tear',10,16,poly(path((5,1),[((6,5),(9,8),(9,11)),((9,17),(1,17),(1,11)),((1,8),(4,5),(5,1))])))
    # Rounded side-part bob: its continuous outer edge reads at 240 x 320.
    bob=path((18,69),[
        ((16,37),(28,11),(58,6)),((90,-1),(118,13),(128,38)),
        ((136,59),(127,101),(133,114)),((121,128),(100,127),(83,126)),
        ((56,132),(25,128),(14,116)),((22,99),(17,88),(18,69))])
    bob_inner=[(75+(x-75)*.95,67+(y-67)*.95) for x,y in bob]
    opening=path((39,53),[
        ((46,45),(65,47),(76,30)),((86,42),(101,48),(116,45)),
        ((120,68),(117,98),(105,113)),((93,128),(73,131),(58,120)),
        ((41,108),(35,81),(39,53))])
    # Black here cuts a hole in the alpha mask, exposing the light face below.
    cut=poly(opening).replace('/>',' fill="black"/>')
    svg('female_body',144,132,poly(bob))
    svg('female_detail',144,132,poly(bob_inner)+cut+nose)
    clip=line([(28,47),(38,43)],3)+line([(29,53),(39,49)],3)
    rings=ellipse(33,91,3,4)+ellipse(119,91,3,4)
    svg('female_accent',144,132,clip+rings)
    # Curved lids and directed gaze replace angular symbol-like expressions.
    eyes={
      'neutral':eye(22)+eye(62),
      'happy':happy(22)+happy(62),
      'laughing':curve((14,20),(22,7),(30,20),3.1)+curve((54,20),(62,7),(70,20),3.1)+curve((15,25),(22,28),(28,24),1.3)+curve((55,24),(62,28),(69,25),1.3),
      'funny':happy(22)+soft_wink(62)+curve((54,8),(63,3),(70,8),2),
      'sad':worry(22,True)+worry(62,False)+pupil(22,22,3,4.4)+pupil(62,22,3,4.4),
      'angry':curve((14,7),(22,8),(30,14),2.9)+curve((54,14),(62,8),(70,7),2.9)+pupil(24,21,3,4.2)+pupil(60,21,3,4.2),
      'crying':worry(22,True)+worry(62,False)+curve((14,21),(22,14),(30,22),2.6)+curve((54,22),(62,14),(70,21),2.6),
      'loving':shut(22)+shut(62)+curve((14,8),(22,4),(30,8),1.8)+curve((54,8),(62,4),(70,8),1.8),
      'embarrassed':pupil(25,21,2.8,4.8)+pupil(65,21,2.8,4.8)+curve((14,8),(22,10),(29,7),2)+curve((54,7),(62,10),(70,8),2),
      'surprised':surprised(22)+surprised(62),
      'shocked':surprised(22,True)+surprised(62,True),
      'thinking':pupil(19,19,3,5)+pupil(59,19,3,5)+curve((14,7),(22,1),(30,6),2.2)+curve((54,9),(62,7),(70,9),2.2),
      'winking':eye(22)+soft_wink(62),
      'cool':glasses(),
      'relaxed':shut(22)+shut(62),
      'delicious':happy(22)+happy(62)+curve((14,6),(22,3),(29,6),1.7)+curve((55,6),(62,3),(70,6),1.7),
      'kissy':soft_wink(22)+shut(62),
      'confident':curve((14,13),(22,8),(30,12),2.4)+curve((54,12),(62,8),(70,13),2.4)+pupil(24,19,3,4)+pupil(64,19,3,4)+curve((55,5),(63,1),(70,4),1.8),
      'sleepy':curve((14,17),(22,22),(30,17),2.6)+curve((54,17),(62,22),(70,17),2.6)+curve((14,9),(22,12),(30,9),1.5)+curve((54,9),(62,12),(70,9),1.5),
      'silly':pupil(23,15,3.5,6)+brow(22)+soft_wink(62),
      'confused':pupil(24,20,3,5)+pupil(64,20,3,5)+curve((14,6),(22,1),(30,7),2.3)+curve((54,10),(62,13),(70,8),2.3),
      'blink':shut(22)+shut(62),
      'cool_round':glasses(round=True),
      'cool_sport':glasses(sport=True),
      'angry_puff':curve((14,9),(22,11),(30,14),2.7)+curve((54,14),(62,11),(70,9),2.7)+curve((15,22),(23,18),(30,22),2.5)+curve((54,22),(61,18),(69,22),2.5),
      'wink_left':soft_wink(22)+eye(62)}
    for name,body in eyes.items():svg('eyes_'+name,80,32,body)
    def open_smile(w=12,depth=19):
        return poly(path((24-w,7),[((19,11),(29,11),(24+w,7)),((24+w-2,depth),(24-w+2,depth),(24-w,7))]))+curve((17,10),(24,12),(31,10),2.3).replace('white','black')
    soft_smile=curve((15,11),(24,20),(33,11),2.5)
    tiny_smile=curve((17,12),(24,17),(31,11),2.2)
    kiss=line(path((21,9),[((27,6),(29,12),(25,13)),((31,16),(27,21),(21,17))]),2.2)
    tongue=open_smile(10,18)+ellipse(29,17,3.2,4.2)+ellipse(29,17,1.2,2.2,'black')
    mouths={
      'neutral':tiny_smile,'happy':soft_smile,'laughing':open_smile(12,24),
      'funny':open_smile(11,21),'sad':curve((17,16),(24,8),(31,16),2.3),
      'angry':curve((15,14),(24,12),(33,14),2.8),
      'crying':poly(path((16,17),[((18,8),(30,8),(32,17)),((29,15),(19,15),(16,17))])),
      'loving':tiny_smile,'embarrassed':curve((18,13),(25,17),(31,12),2),
      'surprised':ellipse(24,13,4,5.3),'shocked':ellipse(24,13,5.5,7.5),
      'thinking':curve((20,14),(25,13),(30,14),2.2),
      'winking':soft_smile,'cool':curve((16,13),(26,20),(34,9),2.6),
      'relaxed':tiny_smile,
      'delicious':soft_smile+curve((29,15),(36,19),(35,10),2.5),
      'kissy':kiss,'confident':curve((16,14),(26,18),(34,9),2.7),
      'sleepy':ellipse(24,14,3.5,4.5),'silly':tongue,
      'confused':curve((18,15),(24,12),(31,13),2.2)}
    assert set(mouths)==set(NAMES),'Each human emotion has deliberately drawn lips'
    for name in NAMES:svg('mouth_'+name,48,26,mouths[name])
    for i,(rx,ry) in enumerate([(3.5,2.5),(5.5,4.5),(7.5,6.5)]):
        svg('talk_'+str(i),48,26,ellipse(24,13,rx,ry)+curve((21,10),(24,11),(27,10),1.5).replace('white','black'))
    print('Bald portrait with soft gloss; 21 hand-tuned portrait emotions and shared rounded bob')
if __name__=='__main__':main()
