#include "../include/lacam.hpp"

#include <algorithm>

LaCAM::LaCAM(Instance* _ins, StateManager* _state_manager, int _time_limit_ms,
             int _seed)
    : ins(_ins),
      seed(_seed),
      time_limit_ms(_time_limit_ms),
      verbose(0),
      MT(std::mt19937(seed)),
      deadline(Deadline(time_limit_ms)),
      dist_table(ins),
      state_manager(_state_manager),
      suo(ins, state_manager, &dist_table, deadline.time_limit_ms / 2, _seed),
      objective(Objective::NONE),
      random_insert_prob_init(0.0025),
      random_insert_prob_random(0.0025),
      restart_prob(0.001),
      num_monte_calro_sampling(10),
      max_space_deviation(10),
      flg_suo(true),
      flg_sort_low_level(false),
      loop_cnt(0),
      solution_cost(0),
      flg_unsolvable(false),
      flg_optimal(false),
      rnd_pibt(0, 1e-4),
      costs_pibt(1000, 0)
{
}

LaCAM::~LaCAM() {}

void LaCAM::solve()
{
  solver_info(1, "start search");
  dist_sqrd_safe_trivial = (ins->G->max_edge_dist + state_manager->agent_rad2 +
                            max_space_deviation * 2);
  dist_sqrd_safe_trivial *= dist_sqrd_safe_trivial;
  dist_table.set();
  if (flg_suo) {
    solver_info(2, "set SUO");
    suo.construct();
    solver_info(2, "set distance table");
  }

  Config Q_init;
  for (auto v : ins->starts) {
    Q_init.push_back(state_manager->get_state(v));
  }
  H_init = new HNode(Q_init, 0, get_h_value(Q_init), dist_table);
  OPEN.push_front(H_init);
  EXPLORED[H_init->Q] = H_init;

  H_goal = nullptr;
  while (!OPEN.empty() && !is_expired(deadline)) {
    ++loop_cnt;

    if (H_goal != nullptr) {
      auto r = get_random_float(MT);
      if (r < random_insert_prob_init) {
        OPEN.push_front(H_init);
      } else if (r < random_insert_prob_init + random_insert_prob_random) {
        HNodes solution_path;
        auto _H = H_goal;
        while (_H != nullptr) {
          solution_path.push_back(_H);
          _H = _H->parent;
        }
        auto H = solution_path[get_random_int(MT, 0, solution_path.size() - 1)];
        OPEN.push_front(H);
      }
    }

    auto H = OPEN.front();
    solver_info(6, "select ", H);

    if (H_goal != nullptr && H->f >= H_goal->g) {
      solver_info(5, "prune HNode with f=", H->f);
      OPEN.pop_front();
      continue;
    }

    if (is_goal_config(H->Q)) {
      if (H_goal == nullptr) {
        solver_info(1, "find goal, cost=", H->g);
        H_goal = H;
        if (objective == Objective::NONE) break;
        OPEN.pop_front();
        continue;
      } else {
        solver_info(2, "cost update: ", H_goal->g, " -> ", H->g);
        H_goal = H;
      }
    }

    if (H->search_tree.empty()) {
      OPEN.pop_front();
      continue;
    }

    auto L = H->search_tree.front();
    GC_LNODES.push_back(L);
    H->search_tree.pop();

    auto Q = get_new_config_monte_calro(H, L);
    if (Q.empty()) continue;

    if (L->depth < ins->N) {
      auto i = L->who;
      auto successors =
          state_manager->get_successors(H->Q[i], max_space_deviation);
      if (flg_sort_low_level) {
        std::sort(
            successors.begin(), successors.end(),
            [&](const State* a, const State* b) {
              auto f_a = dist(a->v_from, a->v_to) + dist_table.get(i, a->v_to);
              auto f_b = dist(b->v_from, b->v_to) + dist_table.get(i, b->v_to);
              return (int)f_a < (int)f_b;
            });
      } else {
        std::shuffle(successors.begin(), successors.end(), MT);
      }
      for (auto s_to : successors) {
        H->search_tree.emplace(new LNode(L, i, s_to));
      }
    }

    auto g = H->g + get_edge_cost(H->Q, Q);
    auto h = get_h_value(Q);
    if (H_goal != nullptr && g + h >= H_goal->g) continue;

    auto iter = EXPLORED.find(Q);
    if (iter != EXPLORED.end()) {
      auto H_known = iter->second;
      if (get_random_float(MT) <= restart_prob) {
        solver_info(4, "restart search");
        OPEN.push_front(H_init);
      } else if (H_known->g <= g) {
        OPEN.push_front(H_known);
        solver_info(4, "search node reinsert");
      } else {
        solver_info(4, "search node replace");
        auto H_new = new HNode(Q, g, h, dist_table, H);
        iter->second = H_new;
        OPEN.push_front(H_new);
        GC_HNODES.push_back(H_known);
      }
    } else {
      auto H_new = new HNode(Q, g, h, dist_table, H);
      OPEN.push_front(H_new);
      EXPLORED[H_new->Q] = H_new;
    }
  }

  if (H_goal != nullptr) {
    backtrack();
    flg_optimal = OPEN.empty();
    solver_info(1, "construct solution: cost=", H_goal->g,
                ", length=", solution.size());
  } else if (OPEN.empty()) {
    flg_unsolvable = true;
    solver_info(1, "unsolvable instance");
  } else {
    solver_info(1, "failed to find solution");
  }

  for (auto L : GC_LNODES) delete L;
  for (auto H : GC_HNODES) delete H;
  for (auto p : EXPLORED) delete p.second;
}

void LaCAM::backtrack()
{
  solution.clear();
  auto H = H_goal;
  while (H != nullptr) {
    solution.push_back(H->Q);
    H = H->parent;
  }
  std::reverse(solution.begin(), solution.end());
  solution_cost = H_goal->g;
}

Config LaCAM::get_new_config(HNode* H, LNode* L)
{
  auto Q_from = H->Q;
  auto Q_to = Config(ins->N, nullptr);

  auto M = L;
  while (M->depth > 0) {
    Q_to[M->who] = M->where;
    if (is_colliding(H, Q_to, M->who, Q_to[M->who])) {
      Q_to.clear();
      return Q_to;
    }
    M = M->parent;
  }

  for (auto k : H->order) {
    if (Q_to[k] == nullptr && !funcPIBT(H, Q_to, k)) {
      Q_to.clear();
      break;
    }
  }

  return Q_to;
};

Config LaCAM::get_new_config_monte_calro(HNode* H, LNode* L)
{
  if (num_monte_calro_sampling <= 1) return get_new_config(H, L);

  Config best_Q;
  float best_cost = std::numeric_limits<float>::max();
  for (auto k = 0; k < num_monte_calro_sampling; ++k) {
    auto Q = get_new_config(H, L);
    if (Q.empty()) continue;
    auto cost = get_edge_cost(H->Q, Q) + get_h_value(Q);
    if (cost < best_cost) {
      best_cost = cost;
      best_Q = Q;
    }
  }
  return best_Q;
}

bool LaCAM::funcPIBT(HNode* H_from, Config& Q_to, const int i,
                     const State* requested_state)
{
  auto Q_from = H_from->Q;
  auto successors =
      state_manager->get_successors(Q_from[i], max_space_deviation);

  const State* stay_action = nullptr;
  for (auto s : successors) {
    if (s->v_from == s->v_to) {
      stay_action = s;
      break;
    }
  }

  if (requested_state != nullptr) {
    auto itr = std::remove_if(
        successors.begin(), successors.end(), [&](const State* s) {
          auto safe_dist = state_manager->agent_rad * 2 + s->space_deviation +
                           requested_state->space_deviation;
          return state_manager->is_colliding(s->v_to, s->v_to,
                                             requested_state->v_from,
                                             requested_state->v_to, safe_dist);
        });
    successors.erase(itr, successors.end());
  }

  auto cmp_pibt = [&](int a, int b) { return costs_pibt[a] > costs_pibt[b]; };
  std::priority_queue<int, std::vector<int>, decltype(cmp_pibt)> indices_queue(
      cmp_pibt);

  const int K = successors.size();
  for (int k = 0; k < K; ++k) {
    costs_pibt[k] = get_state_value(Q_from, i, successors[k]);
    costs_pibt[k] = (int)costs_pibt[k] + rnd_pibt(MT);
    indices_queue.push(k);
  }

  while (!indices_queue.empty()) {
    auto idx = indices_queue.top();
    indices_queue.pop();
    auto s_i_to = successors[idx];

    if (s_i_to->space_deviation > max_space_deviation) {
      solver_info(4, "discard successor due to max space deviation:", s_i_to);
      continue;
    }

    if (state_manager->is_colliding(Q_from, Q_to, i, s_i_to)) {
      Q_to[i] = nullptr;
      continue;
    }

    std::vector<int> resolved_agents;
    auto j = get_will_collide_agent(H_from, i, s_i_to, resolved_agents);

    if (j < 0) {
      Q_to[i] = s_i_to;
      return true;
    }

    if (stay_action != nullptr) {
      Q_to[i] = stay_action;
      while (true) {
        if (Q_to[j] == nullptr && !funcPIBT(H_from, Q_to, j, s_i_to)) break;
        resolved_agents.push_back(j);
        j = get_will_collide_agent(H_from, i, s_i_to, resolved_agents);
        if (j < 0) return true;
      }
    }

    Q_to[i] = nullptr;
  }

  Q_to[i] = nullptr;
  return false;
}

int LaCAM::is_colliding(HNode* H_from, Config& Q_to, const int i,
                        const State* s_i_to)
{
  auto s_i_from = H_from->Q[i];
  for (auto j = 0; j < ins->N; ++j) {
    if (j == i) continue;
    if (state_manager->is_colliding(s_i_from, s_i_to, H_from->Q[j], Q_to[j],
                                    dist_sqrd_safe_trivial)) {
      return true;
    }
    auto H = H_from;
    while (H->parent != nullptr) {
      auto s_j_to = H->Q[j];
      auto s_j_from = H->parent->Q[j];

      if (s_j_to->get_time_upper_bound() < s_i_from->get_time_lower_bound()) {
        break;
      }

      if (state_manager->is_colliding(s_i_from, s_i_to, s_j_from, s_j_to,
                                      dist_sqrd_safe_trivial)) {
        return true;
      }
      H = H->parent;
    }
  }
  return false;
}

int LaCAM::get_will_collide_agent(HNode* H_from, const int i,
                                  const State* s_i_to,
                                  const std::vector<int>& excluding_agents)
{
  auto s_i_from = H_from->Q[i];
  for (auto j = 0; j < ins->N; ++j) {
    if (j == i) continue;
    if (is_contained(j, excluding_agents)) continue;
    if (state_manager->is_colliding(s_i_from, s_i_to, H_from->Q[j],
                                    H_from->Q[j], dist_sqrd_safe_trivial)) {
      return j;
    }

    auto H = H_from;
    while (H->parent != nullptr) {
      auto s_j_to = H->Q[j];
      auto s_j_from = H->parent->Q[j];

      if (s_j_to->get_time_upper_bound() < s_i_from->get_time_lower_bound()) {
        break;
      }

      if (state_manager->is_colliding(s_i_from, s_i_to, s_j_from, s_j_to,
                                      dist_sqrd_safe_trivial)) {
        return j;
      }
      H = H->parent;
    }
  }
  return -1;
}

float LaCAM::get_edge_cost(Config& Q_from, Config& Q_to)
{
  if (objective == Objective::FLOWTIME || objective == Objective::NONE) {
    float cost = 0;
    for (size_t i = 0; i < Q_from.size(); ++i) {
      if (Q_from[i]->v_from == ins->goals[i] && Q_to[i]->v_to == ins->goals[i])
        continue;
      cost += Q_to[i]->time - Q_from[i]->time;
    }
    return cost;
  } else if (objective == Objective::TOTAL_DIST) {
    float cost = 0;
    for (size_t i = 0; i < Q_from.size(); ++i) {
      cost += dist(Q_from[i]->v_to, Q_to[i]->v_to);
    }
    return cost;
  } else {
    return Q_to[0]->time - Q_from[0]->time;
  }
}

float LaCAM::get_h_value(Config& Q)
{
  float h = 0;
  const float max_speed = 3.0;
  if (objective == Objective::FLOWTIME) {
    for (auto i = 0; i < ins->N; ++i) {
      auto d = dist_table.get(i, Q[i]->v_to);
      h += d / max_speed;
    }
  } else if (objective == Objective::MAKESPAN) {
    for (auto i = 0; i < ins->N; ++i) {
      auto d = dist_table.get(i, Q[i]->v_to);
      h = std::max(h, d / max_speed);
    }
  }
  return h;
}

float LaCAM::get_state_value(const Config& Q_from, const int i, const State* s)
{
  float cost = 0;

  if (s->step == s->total_step && s->v_to == ins->goals[i]) {
    return -1;
  }

  if (H_goal == nullptr && s->v_from != s->v_to) {
    auto itr = suo.scatter_data[i].find(s->v_from);
    if (itr != suo.scatter_data[i].end() && s->v_to == itr->second) {
      return cost;
    }
  }

  if (s->v_from == s->v_to) {
    cost += 1e-3;
  } else {
    cost += dist(s->v_from, s->v_to);
  }
  cost += dist_table.get(i, s->v_to);
  return cost;
}

bool LaCAM::is_goal_config(Config& Q)
{
  for (auto i = 0; i < ins->N; ++i) {
    auto& s = Q[i];
    if (s->step != s->total_step) return false;
    if (s->v_to != ins->goals[i]) return false;
  }
  return true;
}

bool LaCAM::validate_solution()
{
  return state_manager->validate_plan(ins, solution);
}

std::ostream& operator<<(std::ostream& os, const Objective objective)
{
  if (objective == Objective::FLOWTIME) {
    os << "flowtime";
  } else if (objective == Objective::MAKESPAN) {
    os << "makespan";
  } else if (objective == Objective::TOTAL_DIST) {
    os << "total_dist";
  } else {
    os << "none";
  }
  return os;
}

void LaCAM::save(const std::string& output_filename, int comp_time_ms)
{
  std::ofstream log;
  log.open(output_filename, std::ios::out);
  log << "planner=lacam\n";
  log << *state_manager;
  log << "comp_time_ms="
      << (comp_time_ms < 0 ? deadline.elapsed_ms() : comp_time_ms) << "\n";
  log << "time_limit_ms=" << time_limit_ms << "\n";
  log << "seed=" << seed << "\n";
  log << "objective=" << objective << "\n";
  log << "solved=" << !solution.empty() << "\n";
  log << "unsolvable=" << flg_unsolvable << "\n";
  log << "optimal=" << flg_optimal << "\n";
  log << "solution_cost=" << solution_cost << "\n";
  log << "makespan=" << solution.size() - 1 << "\n";
  float d_sum = 0;
  for (auto i = 0; i < ins->N; ++i)
    d_sum += dist(ins->starts[i], ins->goals[i]);
  log << "solution_cost_normalized=" << solution_cost / d_sum << "\n";
  log << "solution\n";
  log << solution;
  log.close();

  ins->G->save(output_filename + ".graph");
  save_plan_with_path_format(solution, output_filename + ".paths");
}

void LaCAM::set_verbose(int _verbose)
{
  verbose = _verbose;
  suo.verbose = _verbose;
}
