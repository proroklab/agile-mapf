#include "../include/nodes.hpp"

LNode::LNode() : who(), where(), depth(0) {}

LNode::LNode(LNode* _parent, int i, const State* s)
    : who(i), where(s), depth(_parent->depth + 1), parent(_parent)
{
}

LNode::~LNode(){};

HNode::HNode(Config& _Q, float _g, float _h, DistTable& D, HNode* _parent)
    : Q(_Q),
      parent(_parent),
      g(_g),
      h(0),
      f(g + h),
      priorities(Q.size()),
      order(Q.size(), 0)
{
  search_tree.push(new LNode());

  // set priorities
  const auto N = Q.size();
  if (parent == nullptr) {
    // initialize
    for (size_t i = 0; i < N; ++i) priorities[i] = D.get(i, Q[i]->v_to) / 10000;
  } else {
    // dynamic priorities, akin to PIBT
    for (size_t i = 0; i < N; ++i) {
      if (D.get(i, Q[i]->v_to) != 0) {
        priorities[i] = parent->priorities[i] + 1;
      } else {
        priorities[i] = parent->priorities[i] - (int)parent->priorities[i];
      }
    }
  }

  // set order
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(),
            [&](int i, int j) { return priorities[i] > priorities[j]; });
}

HNode::~HNode()
{
  while (!search_tree.empty()) {
    delete search_tree.front();
    search_tree.pop();
  }
}

std::ostream& operator<<(std::ostream& os, const HNode* H)
{
  os << "HNode("
     << "g=" << std::fixed << std::setprecision(1) << H->g << "; ";
  for (auto s : H->Q) os << s << ", ";
  os << ")";
  return os;
}
