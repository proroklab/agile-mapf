#include <cassert>
#include <iostream>
#include <lacam.hpp>

int main()
{
  {
    // construct instance
    auto G = Graph();
    auto v1 = new Vertex(0, 0);
    auto v2 = new Vertex(1.0, 0);
    v1->neighbors.push_back(v2);
    v2->neighbors.push_back(v1);
    G.V.push_back(v1);
    G.V.push_back(v2);
    Vertices starts({v1});
    Vertices goals({v2});
    auto ins = Instance(&G, starts, goals);

    auto dist_table = DistTable(&ins);
    dist_table.set();
    assert(dist_table.get(0, v1) == 1.0);
    assert(dist_table.get(0, v2) == 0.0);
  }

  return 0;
}
