#pragma once
#include "action_model.hpp"
#include "graph.hpp"
#include "instance.hpp"
#include "utils.hpp"

struct State;
struct StateManager;
using States = std::vector<const State*>;

struct State {
  Vertex* v_from;
  Vertex* v_to;
  int step;
  int total_step;
  float time;
  Vertices past_locs;
  float space_deviation;
  float time_deviation;

  //
  float time_lower_bound;
  float time_upper_bound;

  State(Vertex* v_from, Vertex* v_to, int step, int total_step, float time,
        Vertices past_locs, float space_deviation, float time_deviation);
  ~State();
  bool operator<(const State& other) const;
  float get_time_lower_bound() const;
  float get_time_upper_bound() const;

  static const State* load(std::string s, Graph* G,
                           StateManager* state_manager);
};

inline float State::get_time_lower_bound() const { return time_lower_bound; }

inline float State::get_time_upper_bound() const { return time_upper_bound; }

using Config = std::vector<const State*>;
using Plan = std::vector<Config>;

struct StateManager {
  std::vector<std::set<State> > states_cache;
  std::unordered_map<const State*, States> successors_cache;

  ActionModel* action_model;
  float agent_rad;
  float agent_rad2;
  int Hz;  // the number of divisions for one timestep
  int length_past_locs;
  float space_deviation_sigma_level;  // assuming Gaussian
  float time_deviation_sigma_level;
  const bool flg_del_action_model;

  StateManager();
  StateManager(ActionModel* _action_model);
  ~StateManager();

  const State* get_state(Vertex* v_from);
  const State* get_state(Vertex* v_from, Vertex* v_to, int step, int total_step,
                         float time, Vertices past_locs, float space_deviation,
                         float time_deviation);
  States get_successors(const State* s_from,
                        const float max_space_deviation = 10.0);

  bool is_colliding(const Vertex* v_i, const Vertex* v_j,
                    const float safe_dist);
  bool is_colliding(const Vertex* v_i, const Vertex* v_j_from,
                    const Vertex* v_j_to, const float safe_dist);
  bool is_colliding(const Vertex* v_i_from, const Vertex* v_i_to,
                    const Vertex* v_j_from, const Vertex* v_j_to,
                    const float safe_dist,
                    const float dist_sqrd_safe_trivial = -1);
  bool is_colliding(const Vertex* v_i_from, const Vertex* v_i_to,
                    const Vertex* v_j_from, const Vertex* v_j_to);
  bool is_colliding(const State* s_i_from, const State* s_i_to,
                    const State* s_j_from, const State* s_j_to,
                    const float dist_sqrd_safe_trivial = -1);
  bool is_colliding(Config& Q_from, Config& Q_to);
  bool is_colliding(Config& Q_from, Config& Q_to, const int i,
                    const State* s_i_to);

  bool validate_plan(Instance* ins, Plan& plan);

  void set_agent_rad(float r);

  static StateManager load(const std::string& filename);
};

Plan load_plan(const std::string& filename, Graph* G, StateManager* sm);

void save_plan_with_path_format(const Plan& plan,
                                const std::string& output_filename);

// hash function of configuration
// important: ignore time factors to avoid infinite search space
struct ConfigHasher {
  uint operator()(const Config& Q) const;
};
bool operator==(const Config& Q1, const Config& Q2);

std::ostream& operator<<(std::ostream& os, const State* s);
std::ostream& operator<<(std::ostream& os, const Config& Q);
std::ostream& operator<<(std::ostream& os, const Plan& plan);
std::ostream& operator<<(std::ostream& os, const StateManager& state_manager);
