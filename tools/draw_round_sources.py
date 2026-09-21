"""Reference-led round-faced boy: flat colors, scalloped hair, tiny features."""
from draw_face_sources import NAMES, ROOT, ellipse, rect, line, poly, curve
from draw_abstract_sources import path

def svg(name,w,h,body):
    (ROOT/f'round_{name}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" fill="white">{body}</svg>\n',encoding='utf-8')

def dot(x,y=19,rx=1.5,ry=2.5):return ellipse(x,y,rx,ry)
def brow(x,y=9):return ellipse(x,y,6.8,1.8)
def calm(x):return brow(x)+dot(x)
def pleased(x):return brow(x,8)+curve((x-3.5,20),(x,15),(x+3.5,20),1.4)
def resting(x):return brow(x,9)+curve((x-3.5,18),(x,21),(x+3.5,18),1.4)
def worry(x,left):return line([(x-6,11 if left else 7),(x+6,7 if left else 11)],2.4)+dot(x,21)
def shades(kind=0):
    if kind==1:shapes=ellipse(22,19,7,6)+ellipse(90,19,7,6)
    elif kind==2:shapes=poly([(13,15),(31,16),(27,24),(16,22)])+poly([(81,16),(99,15),(96,22),(85,24)])
    else:shapes=rect(13,14,18,10,3)+rect(81,14,18,10,3)
    return brow(22,7)+brow(90,7)+shapes+line([(30,16),(81,16)],1.6)

def main():
    # Broad silhouette, no ears or face narrowed into a human oval.
    outline=path((10,73),[
        ((5,52),(12,35),(22,29)),((26,20),(38,20),(44,14)),
        ((50,11),(54,6),(61,6)),((67,7),(72,2),(80,3)),((88,1),(95,4),(99,7)),
        ((107,5),(111,14),(122,17)),((134,20),(141,31),(148,38)),
        ((155,50),(155,63),(150,77)),((154,103),(139,121),(114,132)),
        ((85,144),(49,139),(25,122)),((9,110),(7,92),(10,73))])
    svg('outline',160,140,poly(outline)+ellipse(8,79,8,12)+ellipse(152,79,8,12))
    hair=path((12,76),[
        ((7,53),(14,37),(24,31)),((28,22),(40,22),(46,16)),
        ((52,13),(56,8),(62,8)),((69,9),(73,4),(80,5)),((88,3),(95,6),(98,9)),
        ((106,7),(110,16),(122,19)),((132,22),(139,33),(146,40)),
        ((153,51),(153,66),(148,78)),(12,76)])
    svg('hair',160,140,poly(hair))
    face=path((14,74),[
        ((18,57),(27,45),(35,40)),((40,42),(45,41),(49,38)),
        ((56,42),(63,40),(66,37)),((73,41),(80,40),(84,37)),
        ((90,40),(97,40),(101,37)),((106,41),(112,41),(115,39)),
        ((121,44),(128,43),(131,43)),((142,54),(149,66),(148,83)),
        ((151,105),(135,121),(111,130)),((85,141),(50,136),(28,120)),
        ((11,108),(9,90),(14,74))])
    svg('skin',160,140,ellipse(8,79,6,10)+ellipse(152,79,6,10)+poly(face))
    svg('nose',160,140,curve((74,96),(74,101),(81,100),1.2)+curve((81,100),(87,101),(87,96),1.2))
    svg('shirt',160,140,poly(path((46,133),[((62,139),(99,139),(116,133)),(136,140),(25,140),(46,133)])))
    eyes={
      'neutral':calm(22)+calm(90),
      'happy':pleased(22)+pleased(90),
      'laughing':brow(22,7)+brow(90,7)+curve((17,21),(22,14),(27,21),1.8)+curve((85,21),(90,14),(95,21),1.8),
      'funny':pleased(22)+resting(90),
      'sad':worry(22,True)+worry(90,False),
      'angry':line([(15,7),(29,12)],3)+line([(83,12),(97,7)],3)+dot(23,21)+dot(89,21),
      'crying':worry(22,True)+worry(90,False)+curve((18,20),(22,18),(26,21),1.5)+curve((86,21),(90,18),(94,20),1.5),
      'loving':resting(22)+resting(90),
      'embarrassed':brow(22)+brow(90)+dot(24,21)+dot(92,21),
      'surprised':brow(22,5)+brow(90,5)+dot(22,19,1.8,3.2)+dot(90,19,1.8,3.2),
      'shocked':brow(22,3)+brow(90,3)+dot(22,19,2.2,4.1)+dot(90,19,2.2,4.1),
      'thinking':line([(15,7),(29,5)],2.8)+brow(90,9)+dot(20,19)+dot(88,19),
      'winking':calm(22)+resting(90),
      'cool':shades(),
      'relaxed':resting(22)+resting(90),
      'delicious':brow(22,8)+brow(90,8)+curve((18,19),(22,15),(26,19),1.3)+curve((86,19),(90,15),(94,19),1.3),
      'kissy':resting(22)+pleased(90),
      'confident':brow(22,9)+line([(83,9),(97,6)],2.8)+dot(23,20)+dot(91,20),
      'sleepy':brow(22,11)+brow(90,11)+curve((18,20),(22,22),(26,20),1.5)+curve((86,20),(90,22),(94,20),1.5),
      'silly':brow(22,8)+brow(90,10)+dot(22,17,1.5,2.5)+curve((86,19),(90,22),(94,19),1.5),
      'confused':line([(15,8),(29,5)],2.5)+line([(83,8),(97,11)],2.5)+dot(23,20)+dot(91,20),
      'blink':resting(22)+resting(90),
      'cool_round':shades(1),'cool_sport':shades(2),
      'angry_puff':line([(15,9),(29,13)],3)+line([(83,13),(97,9)],3)+curve((18,21),(22,18),(26,21),1.5)+curve((86,21),(90,18),(94,21),1.5),
      'wink_left':resting(22)+calm(90)}
    for name,shape in eyes.items():svg('eyes_'+name,112,28,shape)
    flat=line(path((12,12),[((16,15),(22,9),(27,10)),((33,10),(40,15),(44,12))]),1.3)
    smile=curve((14,10),(27,18),(40,10),1.5)
    tiny=curve((17,12),(27,16),(37,11),1.3)
    bowl=poly(path((18,10),[((23,13),(31,13),(36,10)),((32,22),(22,22),(18,10))]))
    kiss=curve((24,9),(31,12),(25,14),1.3)+curve((25,14),(32,18),(24,18),1.3)
    mouths={
      'neutral':flat,'happy':smile,'laughing':bowl,'funny':bowl,
      'sad':curve((16,15),(27,6),(38,15),1.4),'angry':curve((15,13),(27,10),(39,13),1.8),
      'crying':curve((19,16),(27,5),(35,16),1.8),
      'loving':tiny,'embarrassed':curve((20,12),(27,16),(34,12),1.2),
      'surprised':ellipse(27,13,2.8,3.5),'shocked':ellipse(27,13,4,5.4),
      'thinking':curve((21,14),(28,11),(36,13),1.3),
      'winking':tiny,'cool':curve((15,13),(28,19),(40,9),1.5),
      'relaxed':tiny,'delicious':smile+curve((33,14),(39,17),(38,11),1.3),
      'kissy':kiss,'confident':curve((17,13),(29,17),(39,10),1.5),
      'sleepy':ellipse(27,14,2.5,3),'silly':bowl+ellipse(30,18,2.4,2.7)+ellipse(30,18,.8,1.5,'black'),
      'confused':curve((18,15),(27,10),(37,13),1.3)}
    assert set(mouths)==set(NAMES)
    for name,shape in mouths.items():svg('mouth_'+name,56,24,shape)
    for i,(rx,ry) in enumerate([(2.5,2),(4,3),(5.5,4.5)]):svg('talk_'+str(i),56,24,ellipse(27,13,rx,ry))
    print('Round reference style: 5 fixed layers, 26 eye parts and 24 mouth parts')

if __name__=='__main__':main()
