#pragma once

#include "action_model.hpp"
#include "action_model_torch.hpp"
#include "dist_table.hpp"
#include "graph.hpp"
#include "instance.hpp"
#include "nodes.hpp"
#include "pos.hpp"
#include "problem_builder.hpp"
#include "state.hpp"
#include "suo.hpp"
#include "utils.hpp"

enum Objective { NONE, FLOWTIME, MAKESPAN, TOTAL_DIST };
std::ostream& operator<<(std::ostream& os, const Objective objective);

struct LaCAM {
  Instance* ins;
  const int seed;
  const int time_limit_ms;
  int verbose;
  std::string log_prefix;
  std::mt19937 MT;
  Deadline deadline;

  DistTable dist_table;
  StateManager* state_manager;
  SUO suo;
  Objective objective;
  float random_insert_prob_init;
  float random_insert_prob_random;
  float restart_prob;
  int num_monte_calro_sampling;
  float max_space_deviation;
  bool flg_suo;
  bool flg_sort_low_level;
  int loop_cnt;
  float solution_cost;
  bool flg_unsolvable;
  bool flg_optimal;
  Plan solution;

  // search state
  HNode* H_init;
  HNode* H_goal;
  std::deque<HNode*> OPEN;
  std::unordered_map<Config, HNode*, ConfigHasher> EXPLORED;
  HNodes GC_HNODES;
  LNodes GC_LNODES;

  std::uniform_real_distribution<float> rnd_pibt;
  std::vector<float> costs_pibt;
  float dist_sqrd_safe_trivial;

  void solve();
  void backtrack();
  Config get_new_config(HNode* H, LNode* L);
  Config get_new_config_monte_calro(HNode* H, LNode* L);
  bool funcPIBT(HNode* H, Config& Q_to, const int i,
                const State* requested_state = nullptr);
  float get_edge_cost(Config& Q_from, Config& Q_to);
  float get_h_value(Config& Q);
  float get_state_value(const Config& Q_from, const int i, const State* s);
  bool is_goal_config(Config& Q);
  int is_colliding(HNode* H_from, Config& Q_to, const int i,
                   const State* s_i_to);
  int get_will_collide_agent(HNode* H_from, const int i, const State* s_i_to,
                             const std::vector<int>& excluding_agents);

  template <typename... Body>
  void solver_info(const int level, Body&&... body)
  {
    synchronized_info(level, verbose, format_elapsed_ms(deadline.elapsed_ms()),
                      "\t", log_prefix, "loop_cnt:", std::setw(8), loop_cnt,
                      "\t", (body)...);
  }

  LaCAM(Instance* ins, StateManager* state_manager, int time_limit_ms = 1000,
        int seed = 0);
  ~LaCAM();
  bool validate_solution();
  void save(const std::string& output_filename, int comp_time_ms = -1);
  void set_verbose(int _verbose);
};
