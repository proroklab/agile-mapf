#include <cassert>
#include <filesystem>
#include <iostream>
#include <lacam.hpp>

int main()
{
  std::filesystem::create_directories(".local");

  {
    /*     0 ---
     *     |    |
     * 1 - 2    3
     *     |    |
     *     4 ---
     */
    Vertex::UUID = 0;
    auto G = Graph();
    auto v0 = new Vertex(0, 1);
    auto v1 = new Vertex(-1, 0);
    auto v2 = new Vertex(0, 0);
    auto v3 = new Vertex(4, 0);
    auto v4 = new Vertex(0, -1);
    auto add_edge = [&](Vertex* u, Vertex* v) {
      u->neighbors.push_back(v);
      v->neighbors.push_back(u);
    };
    add_edge(v0, v2);
    add_edge(v1, v2);
    add_edge(v2, v4);
    add_edge(v4, v3);
    add_edge(v0, v3);
    G.V = Vertices({v0, v2, v3, v4});
    G.V = Vertices({v0, v1, v2, v3, v4});
    Vertices starts({v4, v2});
    Vertices goals({v0, v2});
    auto ins = Instance(&G, starts, goals);
    auto state_manager = StateManager();
    state_manager.Hz = 1;
    auto planner_flowtime = LaCAM(&ins, &state_manager);
    planner_flowtime.objective = Objective::FLOWTIME;
    planner_flowtime.verbose = 3;
    planner_flowtime.solve();
    planner_flowtime.save(".local/test_star_flowtime.txt");
    assert(planner_flowtime.validate_solution());
    assert(planner_flowtime.solution_cost == 8);
    assert(planner_flowtime.flg_optimal);

    auto planner_makespan = LaCAM(&ins, &state_manager);
    planner_makespan.objective = Objective::MAKESPAN;
    planner_makespan.verbose = 3;
    planner_makespan.solve();
    planner_makespan.save(".local/test_star_makespan.txt");
    assert(planner_makespan.validate_solution());
    assert(planner_makespan.solution_cost == 6);
  }

  return 0;
}
