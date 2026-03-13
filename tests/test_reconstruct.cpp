#include <cassert>
#include <filesystem>
#include <iostream>
#include <lacam.hpp>

int main()
{
  std::filesystem::create_directories(".local");

  {
    auto G = Graph();
    auto v0 = new Vertex(0, 0);
    auto v1 = new Vertex(1, 0);
    v0->neighbors.push_back(v1);
    v1->neighbors.push_back(v0);
    G.V = Vertices({v0, v1});
    Vertices starts({v0});
    Vertices goals({v1});
    auto ins = Instance(&G, starts, goals);
    auto state_manager = StateManager();
    state_manager.agent_rad = 0.3;
    state_manager.Hz = 2;
    state_manager.length_past_locs = 1;
    auto planner = LaCAM(&ins, &state_manager);
    planner.verbose = 1;
    planner.solve();
    std::string filename = ".local/test_reconstruction_1.txt";
    planner.save(filename);
    auto state_manager_reconstructed = StateManager::load(filename);
    assert(state_manager_reconstructed.agent_rad == state_manager.agent_rad);
    assert(state_manager_reconstructed.Hz == state_manager.Hz);
    auto plan = load_plan(filename, &G, &state_manager_reconstructed);
    std::cout << plan << std::endl;
    assert(plan == planner.solution);
  }

  return 0;
}
