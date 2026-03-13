#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "../include/graph.hpp"
#include "../include/ofApp.hpp"
#include "../include/state.hpp"
#include "ofMain.h"

int main(int argc, char *argv[])
{
  try {
    if (argc < 2) {
      throw std::runtime_error("usage: visualizer <solution-file>");
    }

    const auto solution_file = std::filesystem::path(argv[1]);
    if (!std::filesystem::exists(solution_file)) {
      throw std::runtime_error("solution file not found: " +
                               solution_file.string());
    }

    auto graph_file = std::filesystem::path(solution_file.string() + ".graph");
    if (!std::filesystem::exists(graph_file)) {
      graph_file = solution_file.parent_path() /
                   (solution_file.stem().string() + "_graph.txt");
    }
    if (!std::filesystem::exists(graph_file)) {
      throw std::runtime_error("graph file not found for: " +
                               solution_file.string());
    }

    auto G = Graph::load(graph_file.string());
    auto state_manager = StateManager::load(solution_file.string());
    auto P = load_plan(solution_file.string(), &G, &state_manager);

    if (P.empty()) {
      throw std::runtime_error("no plan entries found in: " +
                               solution_file.string());
    }
    const auto num_agents = P.front().size();
    if (num_agents == 0) {
      throw std::runtime_error("plan is empty in: " + solution_file.string());
    }
    for (size_t t = 0; t < P.size(); ++t) {
      if (P[t].size() != num_agents) {
        throw std::runtime_error("inconsistent agent count at timestep " +
                                 std::to_string(t));
      }
    }

    ofSetupOpenGL(100, 100, OF_WINDOW);
    ofRunApp(new ofApp(&G, &P, state_manager.agent_rad));
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "visualizer error: " << e.what() << std::endl;
    return 1;
  }
}
