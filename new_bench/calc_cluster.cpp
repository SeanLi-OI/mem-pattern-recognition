#include <stdio.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
const int maxn = 100;
std::map<std::string, int> mydict;
std::vector<std::string> dict2;
std::vector<int> fa;
int N = 0, M;
int getfa(int id) {
  if (fa[id] == id)
    return id;
  else
    return fa[id] = getfa(fa[id]);
}
int main() {
  scanf("%d", &M);
  for (int i = 0; i < maxn; i++) fa.push_back(i);
  while (M--) {
    char app1[100];
    char app2[100];
    double p;
    scanf("%s %s %f", app1, app2, &p);
    std::string a1(app1);
    std::string a2(app2);
    int id1, id2;
    auto it1 = mydict.find(a1);
    if (it1 == mydict.end()) {
      id1 = N++;
      mydict[a1] = id1;
      dict2.push_back(a1);
    } else
      id1 = it1->second;
    auto it2 = mydict.find(a2);
    if (it2 == mydict.end()) {
      id2 = N++;
      mydict[a2] = id2;
      dict2.push_back(a2);
    } else
      id2 = it2->second;
    int fa1 = fa[id1] = getfa(id1);
    int fa2 = fa[id2] = getfa(id2);
    if (fa1 != fa2) {
      fa[fa1] = fa2;
    }
  }
  for (int i = 0; i < maxn; i++) {
    for (int j = 0; j < maxn; j++) {
      if (fa[j] == i) {
        std::cout << dict2[j] << std::endl;
      }
    }
    std::cout << std::endl;
  }
}