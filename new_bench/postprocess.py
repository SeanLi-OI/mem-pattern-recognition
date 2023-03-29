import sys
import random
from sklearn.cluster import AgglomerativeClustering, KMeans
app_list = "/data/lixiang/mem-pattern-recognition/new_bench/app_list.txt"
file_path = sys.argv[1]
benchs = {}
n2a = {}
n2b = {}
apps = []
percents = []
with open(app_list) as f:
    for line in f:
        bench = line.strip().split(',')[0]
        app = line.strip().split(',')[1]
        name = line.strip().split(',')[2]
        if bench not in benchs:
            benchs[bench] = {}
        benchs[bench][app] = []
        if name in n2a:
            n2a[name].append(app)
            n2b[name].append(bench)
        else:
            n2a[name] = [app]
            n2b[name] = [bench]

with open(file_path) as f:
    for line in f:
        app = line.strip().split(',')[0]
        percent = line.strip().split(',')[1:]
        percent = [float(i.strip()[:-1])/100.0 for i in percent]
        v1 = 1-sum(percent)
        if v1 > 0.95:
            continue
        if v1 < 0:
            v1 = 0.0
        p1 = (random.uniform(0.0, 0.05))
        p2 = (random.uniform(0.05, 0.95))
        p3 = 1-p1-p2
        percent[-2] = v1*p2
        percent[-1] = v1*p3
        percent.append(v1*p1)
        for i in range(len(n2a[app])):
            benchs[n2b[app][i]][n2a[app][i]] = percent
        apps.append(app)
        percents.append(percent)


def printBenchs():
    for bench in benchs.keys():
        for app in benchs[bench].keys():
            print(bench, ',', app, ',', ','.join(
                [str(round(i*100.0, 5))+"%" for i in benchs[bench][app]]))

def calcVar():
    for bname,apps in benchs.items():
        avgp=[0 for i in range(len(list(apps.values())[0]))]
        for app in apps.values():
            for i in range(len(app)):
                avgp[i]+=app[i]
        avgp=[val*1.0/len(apps) for val in avgp]
        dists=0
        for app in apps.values():
            dist=0
            for i in range(len(app)):
                dist+=(app[i]-avgp[i])**2
            dists+=dist
        dists=dists*1.0/len(apps)
        print(bname+','+str(dists))



def sortDist():
    N = len(apps)
    diss = []

    # print(benchs)

    def dist(a, b):
        s = 0
        for i in range(len(a)):
            s += (a[i]-b[i])**2
        return s
    # print(N)

    for i in range(N):
        for j in range(i+1, N):
            diss.append([apps[i], apps[j], round(
                dist(percents[i], percents[j]), 5)])

    def getKey(x):
        return x[2]
    diss.sort(key=getKey)
    for dis in diss:
        print(dis)

def clusterK(K):
    clf = KMeans(n_clusters=K)
    clf.fit(percents, apps)
    y = clf.fit_predict(percents)
    print(y)
    for i in range(K):
        now=[]
        for j in range(len(apps)):
            if y[j]==i:
                now.append(apps[j])
        print(now)


# printBenchs()
# sortDist()
# clusterK(7)
calcVar()
