#include <cassert>
#include <filesystem>
#include <iostream>
#include <lacam.hpp>

int main()
{
  std::filesystem::create_directories(".local");

  {
    // construct instance
    auto G = Graph();
    auto v1 = new Vertex(0, 0);
    auto v2 = new Vertex(1, 0);
    v1->neighbors.push_back(v2);
    v2->neighbors.push_back(v1);
    G.V.push_back(v1);
    G.V.push_back(v2);
    Vertices starts({v1});
    Vertices goals({v2});
    auto ins = Instance(&G, starts, goals);

    auto state_manager = StateManager();
    state_manager.Hz = 2;
    auto planner = LaCAM(&ins, &state_manager, 1000, 0);
    planner.verbose = 10;
    planner.solve();
    assert(planner.validate_solution());
  }

  {
    Obstacles obstacles;
    auto G = get_lattice_roadmap(obstacles);
    auto starts = Vertices({G.V[0]});
    auto goals = Vertices({G.V.back()});
    auto ins = Instance(&G, starts, goals);
    std::cout << G.size() << std::endl;

    auto state_manager = StateManager();
    auto planner = LaCAM(&ins, &state_manager);
    planner.verbose = 10;
    planner.solve();
    planner.save(".local/test_planner_1.txt");
    assert(planner.validate_solution());
  }

  {
    /*   0
     *   |
     * 1-2-3
     *   |
     *   4
     */
    auto G = Graph();
    auto v0 = new Vertex(0, 1);
    auto v1 = new Vertex(-1, 0);
    auto v2 = new Vertex(0, 0);
    auto v3 = new Vertex(1, 0);
    auto v4 = new Vertex(0, -1);
    auto add_edge = [&](Vertex* u, Vertex* v) {
      u->neighbors.push_back(v);
      v->neighbors.push_back(u);
    };
    add_edge(v0, v2);
    add_edge(v1, v2);
    add_edge(v2, v3);
    add_edge(v2, v4);
    G.V = Vertices({v0, v1, v2, v3, v4});

    Vertices starts({v0, v1});
    Vertices goals({v4, v3});
    auto ins = Instance(&G, starts, goals);

    auto state_manager = StateManager();
    auto planner = LaCAM(&ins, &state_manager);
    planner.verbose = 1;
    planner.solve();
    planner.save(".local/test_planner_2.txt");
    assert(!planner.flg_unsolvable);
    assert(planner.validate_solution());
  }

  {
    // unsolvable instance
    auto G = Graph();
    auto v0 = new Vertex(0, 0);
    auto v1 = new Vertex(0, 1);
    auto add_edge = [&](Vertex* u, Vertex* v) {
      u->neighbors.push_back(v);
      v->neighbors.push_back(u);
    };
    add_edge(v0, v1);
    G.V = Vertices({v0, v1});
    Vertices starts({v0, v1});
    Vertices goals({v1, v0});
    auto ins = Instance(&G, starts, goals);
    auto state_manager = StateManager();
    auto planner = LaCAM(&ins, &state_manager);
    planner.verbose = 1;
    planner.solve();
    assert(planner.solution.empty());
    assert(planner.flg_unsolvable);
  }

  {
    auto G = Graph();
    auto v0 = new Vertex(0, 1);
    auto v1 = new Vertex(-1, 0);
    auto v2 = new Vertex(0, 0);
    auto v3 = new Vertex(1, 0);
    auto v4 = new Vertex(0, -1);
    auto add_edge = [&](Vertex* u, Vertex* v) {
      u->neighbors.push_back(v);
      v->neighbors.push_back(u);
    };
    add_edge(v0, v2);
    add_edge(v1, v2);
    add_edge(v2, v3);
    add_edge(v2, v4);
    G.V = Vertices({v0, v1, v2, v3, v4});

    Vertices starts({v0, v4});
    Vertices goals({v4, v0});
    auto ins = Instance(&G, starts, goals);

    auto state_manager = StateManager();
    state_manager.Hz = 2;
    auto planner = LaCAM(&ins, &state_manager, 1000, 0);
    planner.verbose = 1;
    planner.solve();
    planner.save(".local/test_planner_3.txt");
  }

  {
    const int num_agents = 3;
    auto G = get_random_roadmap();
    auto ins = create_random_instance_from_graph(&G, num_agents);
    auto state_manager = StateManager();
    auto planner = LaCAM(&ins, &state_manager);
    planner.verbose = 1;
    planner.solve();
    planner.save(".local/test_planner_4.txt");
    assert(planner.validate_solution());
  }

  return 0;
}
