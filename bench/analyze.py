import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl

mpl.rcParams['axes.spines.left'] = False
mpl.rcParams['axes.spines.right'] = False
#mpl.rcParams['axes.spines.top'] = False
#mpl.rcParams['axes.spines.bottom'] = False

def parse_filename(name): return re.findall('([0-9]+)', name)

dirs = [f.path for f in os.scandir('../build/bench/benchmarks') if f.is_dir()]
dirs = [[dir, next(os.walk(dir))[2]] for dir in dirs]

df = pd.DataFrame()

names, D, H, N, T, C = [], [], [], [], [], []
for dir in dirs:
    name = dir[0][dir[0].rfind(os.path.sep)+1:]
    for file in dir[1]:
        res = parse_filename(file)
        names.append(name)
        D.append(res[0])
        H.append(res[1])
        N.append(0 if len(res) == 2 else res[2])
        C.append(1 if len(res) == 2 or len(res) == 3 else res[3])
        with open(os.path.join(dir[0], file), 'r') as file:
            l = file.readlines()
            T.append(int(l[0]))

df['name'] = names
df['D'] = D 
df['H'] = H 
df['N'] = N 
df['T'] = T 
df['C'] = C 
df['H'] = df['H'].astype(int)
df['T'] = df['T'].astype(int);
df['D'] = df['D'].astype(int);
df['N'] = df['N'].astype(int);
df['C'] = df['C'].astype(int);

def plot(name):
    Ds = [1, 2, 4, 8]
    Ns = [100000, 1000000, 10000000]
    fig, ax = plt.subplots(4, 3, sharey=True, sharex=True)
    for x, n in enumerate(Ns):
        for y, dim in enumerate(Ds):
            data = df[(df.name == name) & (df.D == dim) & (df.N == n)]
            data = data.sort_values(by=['H'])

            time = data['T']
            time[time < 0] = 0
            time = time.to_list()
            depth = data['H'].to_list()

            ax[y, x].grid(axis='y')
            ax[y, x].plot(depth, time)

    for i, v in enumerate(Ns): plt.setp(ax[-1, i], xlabel='{:.2e}'.format(v))
    for i, v in enumerate(Ds): plt.setp(ax[i, 0], ylabel=v)

def plot_cmp(name1, name2):
    Ds = [1, 2, 4, 8]
    Ns = [100000, 1000000, 10000000]
    fig, ax = plt.subplots(4, 3, sharey=True, sharex=True)
    for x, n in enumerate(Ns):
        for y, dim in enumerate(Ds):
            data1 = df[(df.name == name1) & (df.D == dim) & (df.N == n)]
            data1 = data1.sort_values(by=['H'])

            data2 = df[(df.name == name2) & (df.D == dim) & (df.N == n)]
            data2 = data2.sort_values(by=['H'])

            time1 = data1['T']
            time1[time1 < 0] = 0
            time1 = time1.to_list()
            depth = data1['H'].to_list()

            time2 = data2['T']
            time2[time2 < 0] = 0
            time2 = time2.to_list()

            ax[y, x].grid(axis='y')
            ax[y, x].plot(depth, time1, label=name1)
            ax[y, x].plot(depth, time2, label=name2)

    ax[0, 0].legend()

    for i, v in enumerate(Ns): plt.setp(ax[-1, i], xlabel='{:.2e}'.format(v))
    for i, v in enumerate(Ds): plt.setp(ax[i, 0], ylabel=v)

def plot_cpu(name, dim, name2=str()):
    Cs = [1, 2, 4, 8]
    Ns = [100000, 1000000, 10000000]
    fig, ax = plt.subplots(4, 3, sharey=True, sharex=True)
    for x, n in enumerate(Ns):
        for y, cpu in enumerate(Cs):
            data = df[(df.name == name) & (df.C == cpu) & (df.N == n) & (df.D == dim)]
            data = data.sort_values(by=['H'])

            time = [float(x) for x in data['T'].to_list()]
            depth = data['H'].to_list()

            ax[y, x].grid(axis='y')
            ax[y, x].plot(depth, time, label=name)

            if(name2!=str()):
                data = df[(df.name == name2) & (df.D == dim) & (df.N == n)]
                data = data.sort_values(by=['H'])

                time = data['T'].to_list()
                depth = data['H'].to_list()
                ax[y, x].plot(depth, time, label=name2)

    ax[0, 0].legend()

    for i, v in enumerate(Ns): plt.setp(ax[-1, i], xlabel='{:.2e}'.format(v))
    for i, v in enumerate(Cs): plt.setp(ax[i, 0], ylabel=v)

#plot_cpu("insert_parallel", 4, "insert")
#plot("point_query")'
plot_cmp("intersect_query", "intersect_query_simd")

plt.subplots_adjust(wspace=0, hspace=0)
plt.margins(x=0, y=0)
plt.show()
