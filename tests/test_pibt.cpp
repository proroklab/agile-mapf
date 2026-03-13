#include <cassert>
#include <filesystem>
#include <iostream>
#include <lacam.hpp>

int main()
{
  std::filesystem::create_directories(".local");

  {
    /* 5 - 0
     * |   |
     * 6   1 - 2 - 3
     * |   |
     * 7 - 4
     */
    auto G = Graph();
    auto v0 = new Vertex(0, 1.1);
    auto v1 = new Vertex(0, 0);
    auto v2 = new Vertex(1, 0);
    auto v3 = new Vertex(2, 0);
    auto v4 = new Vertex(0, -1);
    auto v5 = new Vertex(-1, 1.5);
    auto v6 = new Vertex(-1, 0);
    auto v7 = new Vertex(-1, -1.5);
    auto add_edge = [&](Vertex* u, Vertex* v) {
      u->neighbors.push_back(v);
      v->neighbors.push_back(u);
    };
    add_edge(v0, v1);
    add_edge(v1, v2);
    add_edge(v2, v3);
    add_edge(v1, v4);
    add_edge(v5, v0);
    add_edge(v5, v6);
    add_edge(v6, v7);
    add_edge(v4, v7);
    G.V = Vertices({v0, v1, v2, v3, v4, v5, v6, v7});
    Vertices starts({v4, v1, v2});
    Vertices goals({v0, v1, v2});
    auto ins = Instance(&G, starts, goals);
    auto state_manager = StateManager();
    state_manager.Hz = 5;
    auto planner = LaCAM(&ins, &state_manager);
    planner.verbose = 1;
    planner.solve();
    planner.save(".local/test_pibt_1.txt");
    assert(planner.validate_solution());
  }

  {
    /* 5 - 0
     * |   |
     * 6   1 - 2
     * |   |
     * 7 - 4
     */
    auto G = Graph();
    auto v0 = new Vertex(0, 1.1);
    auto v1 = new Vertex(0, 0);
    auto v2 = new Vertex(1, 0);
    auto v4 = new Vertex(0, -1);
    auto v5 = new Vertex(-1, 1.5);
    auto v6 = new Vertex(-1, 0);
    auto v7 = new Vertex(-1, -1.5);
    auto add_edge = [&](Vertex* u, Vertex* v) {
      u->neighbors.push_back(v);
      v->neighbors.push_back(u);
    };
    add_edge(v0, v1);
    add_edge(v1, v2);
    add_edge(v1, v4);
    add_edge(v5, v0);
    add_edge(v5, v6);
    add_edge(v6, v7);
    add_edge(v4, v7);
    G.V = Vertices({v0, v1, v2, v4, v5, v6, v7});
    Vertices starts({v4, v1, v2});
    Vertices goals({v0, v1, v2});
    auto ins = Instance(&G, starts, goals);
    auto state_manager = StateManager();
    auto planner = LaCAM(&ins, &state_manager);
    planner.verbose = 1;
    planner.solve();
    planner.save(".local/test_pibt_2.txt");
    assert(planner.validate_solution());
  }

  return 0;
}
