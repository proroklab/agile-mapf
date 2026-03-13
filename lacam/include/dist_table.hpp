#pragma once

#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

struct DistTable {
  const Instance *ins;
  std::vector<std::vector<float>> table;

  using Node = std::tuple<float, Vertex *>;
  using Nodes = std::vector<Node>;
  std::vector<std::priority_queue<Node, Nodes, std::greater<>>> OPEN;

  DistTable(const Instance *ins);
  float get(const int i, const Vertex *v);  // agent, vertex
  void set();
  void set(const int i);
};
