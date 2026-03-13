#pragma once

#include "dist_table.hpp"
#include "state.hpp"
#include "utils.hpp"

// low level node
struct LNode {
  int who;
  const State* where;
  int depth;
  LNode* parent;

  LNode();
  LNode(LNode* parent, int i, const State* s);
  ~LNode();
};

// high level node
struct HNode {
  Config Q;
  HNode* parent;
  float g;
  const float h;
  float f;
  std::vector<float> priorities;
  std::vector<int> order;
  std::queue<LNode*> search_tree;

  HNode(Config& Q, float g, float h, DistTable& D, HNode* parent = nullptr);
  ~HNode();
};

using HNodes = std::vector<HNode*>;
using LNodes = std::vector<LNode*>;

std::ostream& operator<<(std::ostream& os, const HNode* H);
