#include <cassert>
#include <iostream>
#include <lacam.hpp>

int main()
{
  {
    // construct graph
    auto G = Graph();
    auto v1 = new Vertex(0, 0);
    auto v2 = new Vertex(1, 0);
    v1->neighbors.push_back(v2);
    v2->neighbors.push_back(v1);
    G.V.push_back(v1);
    G.V.push_back(v2);

    // construct instance
    Vertices starts({v1});
    Vertices goals({v2});
    auto ins = Instance(&G, starts, goals);
    assert(ins.N == 1);
  }

  {
    const int num_agents = 3;
    auto G = get_lattice_roadmap();
    auto ins = create_random_instance_from_graph(&G, num_agents);
    assert(ins.N == num_agents);
    std::cout << "starts:\n" << ins.starts;
    std::cout << "goals:\n" << ins.goals;
  }

  return 0;
}
