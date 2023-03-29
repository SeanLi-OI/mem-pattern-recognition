app_list = "/data/lixiang/mem-pattern-recognition/new_bench/app_list.txt"
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
        n2a[name]=app
        n2b[name]=bench
        # if name in n2a:
        #     n2a[name].append(app)
        #     n2b[name].append(bench)
        # else:
        #     n2a[name] = [app]
        #     n2b[name] = [bench]

with open("/data/lixiang/mem-pattern-recognition/new_bench/chosen.txt") as f:
    for line in f.readlines():
        line=line[1:-2].replace('\'','')
        print(line.split(', '))
        for app in line.split(', '):
            if app in n2b:
                print(n2b[app]+','+n2a[app])
        print('')

