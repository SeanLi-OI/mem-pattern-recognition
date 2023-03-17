import sys
import os
from sklearn.decomposition import PCA
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans

argc = len(sys.argv)

if argc < 2 or argc > 3:
    print("Usage: ./cluster.py result_file (figure_path)")
    exit()

file = sys.argv[1]
if not os.path.isabs(file):
    file = os.path.join(os.getcwd(), file)
print(f"Read result_file from {file}")
content = open(file)
app = []
X = []
for line in content.readlines():
    tmp = line.strip().split(',')
    app.append(tmp[0])
    X.append(tmp[1:])

clf = KMeans(n_clusters=3)
clf.fit(X, app)
y = clf.fit_predict(X)
print(app)
print(y)

pca = PCA(n_components=2)
X_pca = pca.fit_transform(X)

plt.plot(X_pca[y == 0, 0], X_pca[y == 0, 1], 'bo', label='Bench A')
plt.plot(X_pca[y == 1, 0], X_pca[y == 1, 1], 'g^', label='Bench B')
plt.plot(X_pca[y == 2, 0], X_pca[y == 2, 1], 'r*', label='Bench C')
plt.legend(loc=0)

fig_path = "paint.png"
if argc == 3:
    fig_path = sys.argv[2]
if not os.path.isabs(fig_path):
    fig_path = os.path.join(os.getcwd(), fig_path)

print(f"Write fig_file to {fig_path}")
plt.savefig(fig_path)
