// Author: Lixiang

#ifndef LRU_QUEUE_H
#define LRU_QUEUE_H

#include <cstddef>
#include <map>
template <typename T, std::size_t N>
class LRUqueue {
 private:
  struct Node {
    T key;
    Node *pre, *next;
    Node(T key) : key(key), pre(nullptr), next(nullptr) {}
  };
  std::map<T, Node *> p;
  Node *head, *tail;
  void remove(Node *cur) {
    if (cur == head)
      head = cur->next;
    else if (cur == tail)
      tail = cur->pre;
    else {
      cur->pre->next = cur->next;
      cur->next->pre = cur->pre;
    }
  }
  void setHead(Node *cur) {
    cur->next = head;
    if (head != nullptr) head->pre = cur;
    head = cur;
    if (tail == nullptr) tail = head;
  }

 public:
  LRUqueue() : head(nullptr), tail(nullptr) {}
  bool check(T key) { return p.find(key) != p.end(); }
  bool push(T key) {  // true if key exists
    auto it = p.find(key);
    if (it != p.end()) {
      Node *cur = it->second;
      remove(cur);
      setHead(cur);
      return true;
    } else {
      Node *tmp = new Node(key);
      if (p.size() >= N) {
        it = p.find(tail->key);
        remove(tail);
        p.erase(it);
      }
      setHead(tmp);
      p[key] = tmp;
    }
    return false;
  }
};
#endif