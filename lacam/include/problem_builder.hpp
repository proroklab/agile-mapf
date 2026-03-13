#pragma once

#include "graph.hpp"
#include "instance.hpp"

struct ProblemBuilderOptions {
  int seed;
  int num_agents;
  float agent_rad;

  std::string graph_type;
  std::string graph_file;
  std::vector<std::string> obstacles_raw;
  int num_random_obstacles;
  float obstacle_rad_min;
  float obstacle_rad_max;

  float x_min;
  float x_max;
  float y_min;
  float y_max;
  float z_min;
  float z_max;
  int num_vertices;
  float step_size;
  float connection_rad;

  std::vector<std::string> agents_raw;
};

Graph build_graph(const ProblemBuilderOptions& options);
Instance build_instance(Graph* graph, const ProblemBuilderOptions& options);
