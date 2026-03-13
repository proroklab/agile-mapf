#pragma once

#include "lacam.hpp"

struct PlannerOptions {
  std::string model_fpath;
  int verbose;
  int seed;
  float speed_default;
  float time_deviation_var_default;
  float space_deviation_mu_default;
  float space_deviation_var_default;
  int Hz;
  float agent_rad;
  int length_past_locs;
  float time_sigma_level;
  float space_sigma_level;
  float time_limit_sec;
  Objective objective;
  int num_planners;
  float random_insert_prob_init;
  float random_insert_prob_random;
  float restart_prob;
  int num_monte_calro_sampling;
  float max_space_deviation;
  bool sort_low_level;
};

struct Planner {
  Instance* ins;
  PlannerOptions options;
  Deadline deadline;

  std::vector<std::unique_ptr<ActionModel>> action_models;
  std::vector<std::unique_ptr<StateManager>> state_managers;
  std::vector<std::unique_ptr<LaCAM>> lacams;

  int best_planner_idx;
  float best_score;

  Planner(Instance* ins, const PlannerOptions& options);

  void solve();
  void save(const std::string& output_filename, int comp_time_ms = -1);
  bool validate_solution();
  bool solved() const;
  float solution_cost() const;
  LaCAM* best_lacam();
  const LaCAM* best_lacam() const;

  template <typename... Body>
  void planner_info(const int level, Body&&... body) const
  {
    synchronized_info(level, options.verbose,
                      format_elapsed_ms(deadline.elapsed_ms()), ", ",
                      (body)...);
  }

  template <typename... Body>
  void worker_info(const int level, const int planner_idx, Body&&... body) const
  {
    synchronized_info(level, options.verbose,
                      format_elapsed_ms(deadline.elapsed_ms()), ", planner[",
                      planner_idx, "], ", (body)...);
  }
};
