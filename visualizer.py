# EXAMPLE: python visualize.py maps/sample.json icfpc_log0.txt

import matplotlib.pyplot as plt
import matplotlib.cm as cm
import json
import sys

argvs = sys.argv
assert len(argvs)==3
json_filename = argvs[1]
log_filename = argvs[2]

json_string = open( json_filename , 'r' ).read();

id2cord = dict()

data = json.loads(json_string)
ids = []
Xs = []
Ys = []
for elm in data['sites']:
	id2cord[elm['id']] = (elm['x'],-elm['y'])
	ids.append(elm['id'])
	Xs.append(elm['x'])
	Ys.append(-elm['y'])

for elm in data['mines']:
	plt.text(id2cord[elm][0],id2cord[elm][1],'◎')

for (label,x,y) in zip(ids,Xs,Ys):
	plt.text(x,y,label)
	plt.scatter(x,y,alpha = 0)

names = []
count = [0]*16
first_line = True
for line in open(log_filename,'r'):
	if first_line == True:
		names = line.rstrip().split(' ')
		first_line = False
	else:
		l = line.rstrip().split(' ')
		p,u,v = int(l[0]),int(l[1]),int(l[2])
		if u==-1 and v==-1 :
			continue
		a,b = id2cord[u],id2cord[v]

		if count[p]==0:
			plt.plot([a[0],b[0]],[a[1],b[1]],color=cm.hsv((p+1)/4),label=names[p])
		else:
			plt.plot([a[0],b[0]],[a[1],b[1]],color=cm.hsv((p+1)/4))
		count[p]+=1
#	plt.text((a[0]+b[0])/2,(a[1]+b[1])/2,count[p],color=cm.hsv((p+1)/10))
	
plt.legend()
plt.show()