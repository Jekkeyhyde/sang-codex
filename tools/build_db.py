# Собирает native/db.bin из data/items.json. Запуск: python tools/build_db.py
import json,os
root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
I=json.load(open(os.path.join(root,'data','items.json'),encoding='utf-8'))
RS,US,GS='\x1e','\x1f','\x1d'
def clean(x): return (x or '').replace(RS,' ').replace(US,' ').replace(GS,' ')
recs=[]
for a in I:
    f=[a['t'],a['code'],a['num'],a.get('stars',''),a.get('jur',''),a['title'],a.get('pun',''),a.get('note',''),a.get('full',''),a.get('kw',''),a.get('doc','')]
    f=[clean(x) for x in f]+[GS.join(a.get(k,[])) for k in ('arts','say','steps','warn')]
    recs.append(US.join(f))
open(os.path.join(root,'native','db.bin'),'wb').write(RS.join(recs).replace('★','*').encode('utf-8'))
print('db.bin:',len(I),'записей')
