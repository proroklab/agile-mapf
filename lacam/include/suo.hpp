#pragma once

#include <random>

#include "dist_table.hpp"
#include "instance.hpp"
#include "state.hpp"
#include "utils.hpp"

struct SUO {
  const Instance* ins;
  StateManager* state_manager;
  DistTable* dist_table;
  Deadline deadline;
  std::mt19937 MT;
  int verbose;
  std::string log_prefix;
  float cost_margin;

  std::vector<std::unordered_map<const Vertex*, const Vertex*> > scatter_data;

  SUO(const Instance* ins, StateManager* state_manager, DistTable* dist_table,
      int time_limit_ms, int seed = 0);
  ~SUO();
  void construct();
  int count_collision(std::vector<States>& paths, const int i);

  template <typename... Body>
  void solver_info(const int level, Body&&... body)
  {
    synchronized_info(level, verbose, format_elapsed_ms(deadline.elapsed_ms()),
                      "\t", log_prefix, "SUO\t", (body)...);
  }
};
