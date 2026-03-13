#include "../include/instance.hpp"

Instance::Instance(Graph* _G, Vertices& _starts, Vertices& _goals)
    : G(_G), starts(_starts), goals(_goals), N(starts.size())
{
  assert(starts.size() == goals.size());
};

Instance::~Instance(){};

Instance create_random_instance_from_graph(Graph* G, const int num_agents,
                                           const int seed,
                                           const float agent_rad)
{
  auto MT = std::mt19937(seed);

  // define starts
  Vertices starts;
  while ((int)starts.size() < num_agents) {
    auto s = random_choose(G->V, MT);
    if (std::any_of(starts.begin(), starts.end(),
                    [&](Vertex* v) { return dist(v, s) <= agent_rad * 2; }))
      continue;
    starts.push_back(s);
  }

  // define goals
  Vertices goals;
  while ((int)goals.size() < num_agents) {
    auto g = random_choose(G->V, MT);
    if (std::any_of(goals.begin(), goals.end(),
                    [&](Vertex* v) { return dist(v, g) <= agent_rad * 2; }))
      continue;
    goals.push_back(g);
  }

  return Instance(G, starts, goals);
}
