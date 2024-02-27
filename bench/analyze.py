import os
import re
import pandas as pd
import json
import matplotlib.pyplot as plt

def parse_filename(name): return re.findall('([0-9]+)', name)

dirs = [f.path for f in os.scandir('../build/bench/benchmarks') if f.is_dir() ]
dirs = [[dir, next(os.walk(dir))[2]] for dir in dirs]

df = pd.DataFrame()

names, D, H, N, T = [], [], [], [], []
for dir in dirs:
    name = dir[0][dir[0].rfind(os.path.sep)+1:]
    for file in dir[1]:
        res = parse_filename(file)
        names.append(name)
        D.append(res[0])
        H.append(res[1])
        N.append(0 if len(res) == 2 else res[2])
        with open(os.path.join(dir[0], file), 'r') as file:
            js = json.load(file)
            T.append(float(js['benchmarks'][0]['real_time']))

df['name'] = names
df['D'] = D 
df['H'] = H 
df['N'] = N 
df['T'] = T 
df['H'] = df['H'].astype(int)
df['T'] = df['T'].astype(int);
df['D'] = df['D'].astype(int);
df['N'] = df['N'].astype(int);


def plot(name):
    Ds = [1, 2, 4, 8]
    Ns = [100000, 1000000, 10000000]
    fig, ax = plt.subplots(4, 3, sharey=True, sharex=True)
    for x, n in enumerate(Ns):
        for y, dim in enumerate(Ds):
            data = df[(df.name == name) & (df.D == dim) & (df.N == n)]
            data = data.sort_values(by=['H'])

            time = data['T'].to_list()
            depth = data['H'].to_list()

            ax[y, x].plot(depth, time)

    for i, v in enumerate(Ns): plt.setp(ax[-1, i], xlabel='{:.2e}'.format(v))
    for i, v in enumerate(Ds): plt.setp(ax[i, 0], ylabel=v)

plot("knn_query")

plt.subplots_adjust(wspace=0, hspace=0)
plt.margins(x=0, y=0)
plt.show()
