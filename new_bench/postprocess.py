import sys
import random
import math
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
        percent = [float(i.strip()[:-1]) for i in percent]
        v1 = 100.0-sum(percent)
        if v1 > 95.0:
            continue
        if v1 < 0.0:
            v1 = 0.0
        p1 = (random.uniform(0.0, 0.05))
        p2 = (random.uniform(0.05, 0.95))
        p3 = 1.0-p1-p2
        percent[-2] = v1*p2
        percent[-1] = v1*p3
        percent.append(v1*p1)
        for i in range(len(n2a[app])):
            benchs[n2b[app][i]][n2a[app][i]] = percent
        apps.append(app)
        percents.append(percent)



def dist(a, b):
    s = 0
    for i in range(len(a)):
        s += (a[i]-b[i])**2
    return 100-math.sqrt(s)/1.414


def printBenchs():
    bname=[]
    bpercent=[]
    for bench in benchs.keys():
        sum=[0.0 for i in range(len(list(benchs[bench].values())[0]))]
        n=0.0
        for app in benchs[bench].keys():
            sum=[sum[i]+benchs[bench][app][i] for i in range(len(benchs[bench][app]))]
            n=n+1.0
            # print(bench, ',', app, ',', ','.join(
            #     [str(round(i, 5)) for i in benchs[bench][app]]))
        bname.append(bench)
        bpercent.append([i/n for i in sum])
        # print(bench," ",','.join([str(round(i/n, 5)) for i in sum]))
    for i in range(len(bname)):
        print(","+bname[i],end='')
    print("")
    for i in range(len(bname)):
        print(bname[i],end=',')
        for j in range(len(bname)):
            print(round(dist(bpercent[i],bpercent[j]),1),end=',')
        print("")
    


def calcVar():
    c_list = ['ua.A.x', 'h264ref_base.prefetch-riscv', 'zeusmp_base.prefetch-riscv', 'bisort',
              'hmmer_base.prefetch-riscv', 'deepsjeng_r_base.prefetch-m64', 'power', 'bh', 'radix', 'mst', 'tsp']
    avgp_c = [0 for i in range(
        len(list(list(benchs.values())[0].values())[0]))]
    app_c = []
    for bname, apps in benchs.items():
        avgp = [0 for i in range(len(list(apps.values())[0]))]
        for aname, app in apps.items():
            for i in range(len(app)):
                avgp[i] += app[i]
            if aname in c_list:
                app_c.append(app)
                for i in range(len(app)):
                    avgp_c[i] += app[i]
        avgp = [val*1.0/len(apps) for val in avgp]
        dists = 0
        for app in apps.values():
            dist = 0
            for i in range(len(app)):
                dist += (app[i]-avgp[i])**2
            dists += dist
        dists = dists*1.0/len(apps)
        print(bname+','+str(dists))
    avgp_c = [val*1.0/len(app_c) for val in avgp_c]
    dists = 0
    for app in app_c:
        dist = 0
        for i in range(len(app)):
            dist += (app[i]-avgp_c[i])**2
        dists += dist
    dists = dists*1.0/len(app_c)
    print('chosen'+','+str(dists))


def sortDist():
    N = len(apps)
    diss = []

    # print(benchs)


    # print(N)

    for i in range(N):
        for j in range(i+1, N):
            diss.append([apps[i], apps[j], round(
                dist(percents[i], percents[j]), 5)])

    def getKey(x):
        return x[2]
    diss.sort(key=getKey)
    sum=0.0
    for dis in diss:
        sum=sum+dis[2]
        print(dis)
    print(sum/(N*(N-1)/2.0))


def clusterK(K):
    clf = KMeans(n_clusters=K)
    clf.fit(percents, apps)
    y = clf.fit_predict(percents)
    print(y)
    for i in range(K):
        now = []
        for j in range(len(apps)):
            if y[j] == i:
                now.append(apps[j])
        print(now)


def sortDist2():
    N = len(apps)
    diss = []

    # print(benchs)
    # print(N)
    avg_percent = [0.1, 11.1, 11.1, 11.1, 11.1,
                   11.1, 11.1, 11.1, 11.1, 11.1, 0.0]
    # avg_percent = [0.1, 14.28, 14.28, 14.28, 14.28,
    #                14.28, 14.28, 14.28, 14.28, 0.0, 0.0]
    sum = 0.0
    for i in range(N):
        diss.append([apps[i], round(
            dist(percents[i], avg_percent), 5)])
        sum = sum+round(dist(percents[i], avg_percent), 5)
    print(sum/N)

    def getKey(x):
        return x[1]
    diss.sort(key=getKey)
    for dis in diss:
        print(dis)


printBenchs()
# sortDist()
# clusterK(7)
# calcVar()
# sortDist2()

# N = len(apps)
# for i in range(N):
#     print(percents[i])