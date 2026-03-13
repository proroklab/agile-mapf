#pragma once

#include "graph.hpp"
#include "utils.hpp"

struct Instance {
  Graph* G;
  Vertices starts;
  Vertices goals;
  const int N;  // number of agents

  Instance(Graph* _G, Vertices& _starts, Vertices& _goals);
  ~Instance();
};

Instance create_random_instance_from_graph(Graph* G, const int num_agents,
                                           const int seed = 0,
                                           const float agent_rad = 0.25);
