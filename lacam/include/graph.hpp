#pragma once
#include "pos.hpp"
#include "utils.hpp"

struct Vertex {
  static int UUID;

  const int id;
  Pos pos;
  std::vector<Vertex*> neighbors;

  Vertex(float _x, float _y, float _z = 0);
  ~Vertex();
};
using Vertices = std::vector<Vertex*>;
bool operator==(const Vertices& Q1, const Vertices& Q2);

struct Graph {
  Vertices V;
  Graph();
  ~Graph();

  float max_edge_dist;

  int size() const;
  void save(const std::string& output_filename);
  static Graph load(const std::string& filename);
};

using Obstacle = std::tuple<Pos, float>;
using Obstacles = std::vector<Obstacle>;

float dist(Vertex* u, Vertex* b);

Graph get_lattice_roadmap(const Obstacles& obstacles = Obstacles(),
                          const float step_size = 0.5, const float x_min = -1,
                          const float x_max = 1, const float y_min = -1,
                          const float y_max = 1, const float z_min = 0.0,
                          const float z_max = 0.0,
                          const float connection_rad = 0.5,
                          const float agent_rad = 0.25);

Graph get_random_roadmap(const Obstacles& obstacles = Obstacles(),
                         const int num_vertices = 50, const int seed = 0,
                         const float x_min = -1, const float x_max = 1,
                         const float y_min = -1, const float y_max = 1,
                         const float z_min = 0.0, const float z_max = 0.0,
                         const float connection_rad = 0.5,
                         const float agent_rad = 0.25);

void add_edges(Graph& G, const Obstacles& obstacles,
               const float connection_rad = 0.5, const float agent_rad = 0.25);

std::ostream& operator<<(std::ostream& os, const Vertex* v);
std::ostream& operator<<(std::ostream& os, const Vertices& arr);
std::ostream& operator<<(std::ostream& os, const Graph* G);
