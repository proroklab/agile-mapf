#include "../include/suo.hpp"

SUO::SUO(const Instance* _ins, StateManager* _state_manager,
         DistTable* _dist_table, int time_limit_ms, int seed)
    : ins(_ins),
      state_manager(_state_manager),
      dist_table(_dist_table),
      deadline(Deadline(time_limit_ms)),
      MT(seed),
      verbose(0),
      cost_margin(1.05),
      scatter_data(ins->N)
{
}

SUO::~SUO() {}

void SUO::construct()
{
  solver_info(1, "start constructing, cost_margin=", cost_margin);

  auto paths = std::vector<States>(ins->N);

  // vertex, cost-to-come, cost-to-go, collision, depth, parent
  using ScatterNode =
      std::tuple<const State*, float, float, int, int, const State*>;
  auto cmp = [&](ScatterNode& a, ScatterNode& b) {
    // collision
    if (std::get<3>(a) != std::get<3>(b))
      return std::get<3>(a) > std::get<3>(b);
    auto g_a = std::get<1>(a);
    auto g_b = std::get<1>(b);
    auto h_a = std::get<2>(a);
    auto h_b = std::get<2>(b);
    auto f_a = g_a + h_a;
    auto f_b = g_b + h_b;
    if (f_a != f_b) return f_a > f_b;
    if (g_a != g_b) return g_a < g_b;
    return std::get<0>(a)->v_from->id < std::get<0>(b)->v_from->id;
  };

  auto order = std::vector<int>(ins->N, 0);
  std::iota(order.begin(), order.end(), 0);

  int collision_cnt_total_last = INT_MAX;
  int collision_cnt_total = 0;

  while (!is_expired(deadline)) {
    std::shuffle(order.begin(), order.end(), MT);

    for (auto _i = 0; _i < ins->N; ++_i) {
      // single agent path finding for agent-i
      const auto i = order[_i];

      // update collision count
      if (!paths[i].empty()) {
        collision_cnt_total -= count_collision(paths, i);
        paths[i].clear();
      }

      const auto f_val_ub =
          (dist_table->get(i, ins->starts[i]) + 1.0 / state_manager->Hz) *
          cost_margin;

      // setup A*
      auto OPEN = std::priority_queue<ScatterNode, std::vector<ScatterNode>,
                                      decltype(cmp)>(cmp);
      auto CLOSED = std::unordered_map<const State*, const State*>();

      // set init node
      auto s_init = state_manager->get_state(ins->starts[i]);
      OPEN.emplace(s_init, s_init->time, dist_table->get(i, s_init->v_to), 0, 0,
                   nullptr);

      // main loop
      while (!OPEN.empty() && !is_expired(deadline)) {
        auto node = OPEN.top();
        OPEN.pop();
        auto s_from = std::get<0>(node);  // state
        const auto num_collisions_from = std::get<3>(node);

        // check closed list
        auto itr = CLOSED.find(s_from);
        if (itr != CLOSED.end()) continue;
        CLOSED[s_from] = std::get<5>(node);  // store parent

        // check goal condition
        if (s_from->step == s_from->total_step &&
            s_from->v_to == ins->goals[i]) {
          solver_info(4, "agent-", i, ", OPEN=", OPEN.size(),
                      ", CLOSED=", CLOSED.size());

          // backtrack
          auto s = s_from;
          while (s != nullptr) {
            paths[i].push_back(s);
            s = CLOSED[s];
          }
          std::reverse(paths[i].begin(), paths[i].end());

          // update collision count
          collision_cnt_total += count_collision(paths, i);
          break;
        }

        // expand
        const auto depth_from = std::get<4>(node);
        auto successors = state_manager->get_successors(s_from);
        std::shuffle(successors.begin(), successors.end(), MT);
        // prune several successors
        for (int k = 0; k < std::min((int)successors.size(), 8); ++k) {
          auto&& s_to = successors[k];
          auto g_val_to = s_to->time;
          auto h_val_to = dist_table->get(i, s_to->v_to);

          // check upper bound
          if (g_val_to + h_val_to > f_val_ub) continue;

          // check closed list
          itr = CLOSED.find(s_to);
          if (itr != CLOSED.end()) continue;

          auto depth_to = depth_from + 1;

          // identify number of collisions
          int num_collisions = num_collisions_from;
          for (auto j = 0; j < ins->N; ++j) {
            if (j == i || paths[j].empty()) continue;
            auto&& path_j = paths[j];
            auto T = (int)path_j.size() - 1;
            auto s_j_from = path_j[std::min(depth_from, T)];
            auto s_j_to = path_j[std::min(depth_to, T)];
            if (state_manager->is_colliding(s_from, s_to, s_j_from, s_j_to)) {
              ++num_collisions;
            }
          }

          // insert
          OPEN.emplace(s_to, g_val_to, h_val_to, num_collisions, depth_to,
                       s_from);
        }
      }
    }

    solver_info(2, "collision_cnt_total=", collision_cnt_total);
    if (collision_cnt_total == 0 ||
        collision_cnt_total >= collision_cnt_total_last) {
      break;
    }
    collision_cnt_total_last = collision_cnt_total;
  }

  for (auto i = 0; i < ins->N; ++i) {
    for (auto&& s : paths[i]) {
      if (s->v_from != s->v_to) scatter_data[i][s->v_from] = s->v_to;
    }
  }

  solver_info(1, "finish, collision_cnt_total=", collision_cnt_total);
}

int SUO::count_collision(std::vector<States>& paths, const int i)
{
  int cnt = 0;
  auto&& path_i = paths[i];
  auto T_i = (int)path_i.size() - 1;
  for (auto j = 0; j < ins->N; ++j) {
    if (j == i || paths[j].empty()) continue;
    auto&& path_j = paths[j];
    auto T_j = (int)path_j.size() - 1;
    auto T = std::max(T_i, T_j);
    for (auto t = 1; t <= T; ++t) {
      auto s_i_from = path_i[std::min(t - 1, T_i)];
      auto s_i_to = path_i[std::min(t, T_i)];
      auto s_j_from = path_j[std::min(t - 1, T_j)];
      auto s_j_to = path_j[std::min(t, T_j)];
      if (state_manager->is_colliding(s_i_from, s_i_to, s_j_from, s_j_to)) {
        ++cnt;
      }
    }
  }
  return cnt;
}
