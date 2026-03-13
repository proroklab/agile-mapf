#include "../include/state.hpp"

#include <stdexcept>

namespace
{

  Vertex* find_vertex_by_pos(Graph* G, const Pos& pos, const float eps = 1e-4f)
  {
    for (auto* v : G->V) {
      if (dist_sqrd(v->pos, pos) <= eps * eps) return v;
    }
    return nullptr;
  }

  const State* load_state_by_transition_string(const std::string& s, Graph* G,
                                               StateManager* state_manager,
                                               const float time)
  {
    const auto r_state = std::regex(
        R"(\(\[(.+?),(.+?),(.+?)\]->\[(.+?),(.+?),(.+?)\],(\d+)\/(\d+)\))");
    std::smatch results;
    if (!std::regex_match(s, results, r_state)) {
      throw std::runtime_error("failed to parse state: " + s);
    }

    const auto pos_from =
        Pos(std::stof(results[1].str()), std::stof(results[2].str()),
            std::stof(results[3].str()));
    const auto pos_to =
        Pos(std::stof(results[4].str()), std::stof(results[5].str()),
            std::stof(results[6].str()));
    auto* v_from = find_vertex_by_pos(G, pos_from);
    auto* v_to = find_vertex_by_pos(G, pos_to);
    if (v_from == nullptr || v_to == nullptr) {
      throw std::runtime_error("failed to map state to graph vertices: " + s);
    }

    const auto step = std::stoi(results[7].str());
    const auto total_step = std::stoi(results[8].str());
    Vertices past_locs(state_manager->length_past_locs, v_from);
    return state_manager->get_state(v_from, v_to, step, total_step, time,
                                    past_locs, 0.0f, 0.0f);
  }

}  // namespace

State::State(Vertex* _v_from, Vertex* _v_to, int _step, int _total_step,
             float _time, Vertices _past_locs, float _space_deviation,
             float _time_deviation)
    : v_from(_v_from),
      v_to(_v_to),
      step(_step),
      total_step(_total_step),
      time(_time),
      past_locs(_past_locs),
      space_deviation(_space_deviation),
      time_deviation(_time_deviation),
      time_lower_bound(std::max((float)0, time - time_deviation)),
      time_upper_bound(time + time_deviation)
{
}

State::~State(){};

// to ensure uniqueness
bool State::operator<(const State& other) const
{
  if (v_from != other.v_from) return v_from < other.v_from;
  if (v_to != other.v_to) return v_to < other.v_to;
  if (step != other.step) return step < other.step;
  if (total_step != other.total_step) return total_step < other.total_step;

  // past_locs
  if (past_locs.size() != other.past_locs.size()) {
    return past_locs.size() < other.past_locs.size();
  }
  for (size_t k = 0; k < past_locs.size(); ++k) {
    if (past_locs[k] != other.past_locs[k]) {
      return past_locs[k] < other.past_locs[k];
    }
  }

  if (time != other.time) return time < other.time;
  if (space_deviation != other.space_deviation)
    return space_deviation < other.space_deviation;
  return time_deviation < other.time_deviation;
}

StateManager::StateManager()
    : action_model(new ActionModel()),
      agent_rad(0.25),
      agent_rad2(agent_rad * 2),
      Hz(10),
      length_past_locs(1),
      space_deviation_sigma_level(0),
      time_deviation_sigma_level(0),
      flg_del_action_model(true)
{
}

StateManager::StateManager(ActionModel* _action_model)
    : action_model(_action_model),
      agent_rad(0.25),
      agent_rad2(agent_rad * 2),
      Hz(10),
      length_past_locs(action_model->length_past_locs),
      space_deviation_sigma_level(0),
      time_deviation_sigma_level(0),
      flg_del_action_model(false)
{
}

StateManager::~StateManager()
{
  if (flg_del_action_model) delete action_model;
}

void StateManager::set_agent_rad(float r)
{
  agent_rad = r;
  agent_rad2 = r * 2;
}

const State* StateManager::get_state(Vertex* v_init)
{
  Vertices past_locs;
  for (auto k = 0; k < length_past_locs; ++k) past_locs.push_back(v_init);
  return get_state(v_init,     // v_from
                   v_init,     // v_to
                   1,          // step
                   1,          // total_step
                   0,          // time
                   past_locs,  // past_locs
                   0,          // space deviation
                   0           // time deviation
  );
}

const State* StateManager::get_state(Vertex* v_from, Vertex* v_to, int step,
                                     int total_step, float time,
                                     Vertices past_locs, float space_deviation,
                                     float time_deviation)
{
  auto s = State(v_from, v_to, step, total_step, time, past_locs,
                 space_deviation, time_deviation);
  // adjust cache size
  while ((int)states_cache.size() <= v_from->id) {
    states_cache.push_back(std::set<State>());
  }
  // check duplicate
  auto itr = std::get<0>(states_cache[v_from->id].insert(s));
  return &(*itr);
}

States StateManager::get_successors(const State* s_from,
                                    const float max_space_deviation)
{
  // check cache
  {
    auto itr = successors_cache.find(s_from);
    if (itr != successors_cache.end()) return itr->second;
  }

  States successors;
  const float time = s_from->time + (1.0 / Hz);

  // shift past_locs
  Vertices past_locs;
  {
    for (auto k = 1; k < length_past_locs; ++k) {
      past_locs.push_back(s_from->past_locs[k]);
    }
    if (length_past_locs > 0) past_locs.push_back(s_from->v_from);
  }

  if (s_from->step < s_from->total_step) {
    // increment step only
    auto s_to = get_state(s_from->v_from, s_from->v_to, s_from->step + 1,
                          s_from->total_step, time, s_from->past_locs,
                          s_from->space_deviation, s_from->time_deviation);
    successors.push_back(s_to);

    // start taking other actions
  } else {
    // move to adjacent vertices
    auto&& actions =
        action_model->get_successor_actions(s_from->past_locs, s_from->v_to);
    for (auto&& action : actions) {
      auto u = std::get<0>(action);
      auto travel_time = std::max((float)0, std::get<1>(action));
      auto total_step = std::max(1, (int)std::round(travel_time * Hz));

      // compute space deviation
      auto space_deviation =
          std::get<3>(action) +
          space_deviation_sigma_level * std::sqrt(std::get<4>(action));
      if (space_deviation > max_space_deviation) {
        continue;
      }

      // compute time deviation
      float time_deviation = 0.0;
      if (time_deviation_sigma_level > 0) {
        auto parent_time_deviation_var =
            std::pow(s_from->time_deviation / time_deviation_sigma_level, 2);
        auto time_deviation_var =
            parent_time_deviation_var + std::get<2>(action);
        time_deviation =
            std::sqrt(time_deviation_var) * time_deviation_sigma_level;
      }

      auto s_to = get_state(s_from->v_to,       // v_from
                            u,                  // v_to
                            1,                  // step
                            total_step,         // total_step
                            time,               // time
                            s_from->past_locs,  // past_locs
                            space_deviation,    // space_deviation
                            time_deviation      // time_deviation
      );
      successors.push_back(s_to);
    }
    // stay actions
    auto s_stay = get_state(s_from->v_to,           // v_from
                            s_from->v_to,           // v_to
                            1,                      // step
                            1,                      // total_step
                            time,                   // time
                            past_locs,              // past_locs
                            0,                      // space_deviation
                            s_from->time_deviation  // time_deviation
    );
    successors.push_back(s_stay);
  }

  // register to cache
  successors_cache[s_from] = successors;
  return successors;
}

bool StateManager::is_colliding(const Vertex* v_i, const Vertex* v_j,
                                const float safe_dist)
{
  auto d = dist_sqrd(v_i->pos, v_j->pos);
  return d < safe_dist * safe_dist;
}

bool StateManager::is_colliding(const Vertex* v_i, const Vertex* v_j_from,
                                const Vertex* v_j_to, const float safe_dist)
{
  auto d = dist_between_line_segment_and_point_sqrd(v_j_from->pos, v_j_to->pos,
                                                    v_i->pos);
  return d < safe_dist * safe_dist;
}

bool StateManager::is_colliding(const Vertex* v_i_from, const Vertex* v_i_to,
                                const Vertex* v_j_from, const Vertex* v_j_to,
                                const float safe_dist,
                                const float dist_sqrd_safe_trivial)
{
  bool equal_i = v_i_from == v_i_to;
  bool equal_j = v_j_from == v_j_to;
  float d = 0;
  if (equal_i && equal_j) {
    d = dist_sqrd(v_i_from->pos, v_j_from->pos);
  } else if (equal_i) {
    d = dist_between_line_segment_and_point_sqrd(v_j_from->pos, v_j_to->pos,
                                                 v_i_from->pos);
  } else if (equal_j) {
    d = dist_between_line_segment_and_point_sqrd(v_i_from->pos, v_i_to->pos,
                                                 v_j_from->pos);
  } else {
    if (dist_sqrd_safe_trivial > 0) {
      // exclude trivial case
      auto d_sqrd_pairwise = std::min({
          dist_sqrd(v_i_from->pos, v_j_from->pos),
          dist_sqrd(v_i_from->pos, v_j_to->pos),
          dist_sqrd(v_i_to->pos, v_j_from->pos),
          dist_sqrd(v_i_to->pos, v_j_to->pos),
      });
      if (d_sqrd_pairwise > dist_sqrd_safe_trivial) return false;
    }
    d = dist_between_line_segments_sqrd(v_i_from->pos, v_i_to->pos,
                                        v_j_from->pos, v_j_to->pos);
  }
  return d < safe_dist * safe_dist;
}

bool StateManager::is_colliding(const Vertex* v_i_from, const Vertex* v_i_to,
                                const Vertex* v_j_from, const Vertex* v_j_to)
{
  return is_colliding(v_i_from, v_i_to, v_j_from, v_j_to, agent_rad2);
}

bool StateManager::is_colliding(const State* s_i_from, const State* s_i_to,
                                const State* s_j_from, const State* s_j_to,
                                const float dist_sqrd_safe_trivial)
{
  if (s_i_from == nullptr || s_i_to == nullptr || s_j_from == nullptr ||
      s_j_to == nullptr)
    return false;
  const float safe_dist =
      agent_rad2 + s_i_to->space_deviation + s_j_to->space_deviation;

  bool equal_i = s_i_to->v_from == s_i_to->v_to;
  bool equal_j = s_j_to->v_from == s_j_to->v_to;
  if (equal_i && equal_j) {
    return is_colliding(s_i_to->v_to, s_j_to->v_to, safe_dist);
  } else if (equal_i) {
    return is_colliding(s_i_to->v_to, s_j_to->v_from, s_j_to->v_to, safe_dist);
  } else if (equal_j) {
    return is_colliding(s_j_to->v_to, s_i_to->v_from, s_i_to->v_to, safe_dist);
  }

  return is_colliding(s_i_to->v_from, s_i_to->v_to, s_j_to->v_from,
                      s_j_to->v_to, safe_dist, dist_sqrd_safe_trivial);
}

bool StateManager::is_colliding(Config& Q_from, Config& Q_to)
{
  auto N = Q_from.size();
  for (size_t i = 0; i < N; ++i) {
    for (auto j = i + 1; j < N; ++j) {
      if (is_colliding(Q_from[i], Q_to[i], Q_from[j], Q_to[j])) return true;
    }
  }
  return false;
}

bool StateManager::is_colliding(Config& Q_from, Config& Q_to, const int i,
                                const State* s_i_to)
{
  auto N = Q_from.size();
  for (size_t j = 0; j < N; ++j) {
    if ((int)j == i) continue;
    if (Q_from[j] == nullptr || Q_to[j] == nullptr) continue;
    if (is_colliding(Q_from[i], s_i_to, Q_from[j], Q_to[j])) return true;
  }
  return false;
}

bool StateManager::validate_plan(Instance* ins, Plan& plan)
{
  if (plan.empty()) {
    info(0, 0, "solution is empty");
    return false;
  }
  // check starts and goals
  for (auto i = 0; i < ins->N; ++i) {
    auto s = plan[0][i]->v_from;
    if (ins->starts[i] != s) {
      info(0, 0, "validate_solution: invalid starts");
      return false;
    }
    auto g = plan.back()[i]->v_to;
    if (ins->goals[i] != g) {
      info(0, 0, "validate_solution: invalid goals");
      return false;
    }
    if (plan.back()[i]->step != plan.back()[i]->total_step) {
      info(0, 0, "validate_solution: invalid goals");
      return false;
    }
  }

  for (size_t t = 1; t < plan.size(); ++t) {
    for (auto i = 0; i < ins->N; ++i) {
      auto s_i_from = plan[t - 1][i];
      auto s_i_to = plan[t][i];

      auto v_i_prev_to = s_i_from->v_to;
      auto v_i_from = s_i_to->v_from;
      auto v_i_to = s_i_to->v_to;
      // check continuity
      if ((v_i_prev_to == v_i_from || v_i_prev_to == v_i_to) &&
          v_i_from != v_i_to && !is_contained(v_i_to, v_i_from->neighbors)) {
        info(0, 0, "validate_solution: invalid transition:", v_i_from, " -> ",
             v_i_to);
        return false;
      }

      // check collision
      for (auto j = i + 1; j < ins->N; ++j) {
        for (auto t_j = t; t_j > 0; --t_j) {
          auto s_j_from = plan[t_j - 1][j];
          auto s_j_to = plan[t_j][j];
          if (s_j_to->get_time_upper_bound() < s_i_from->get_time_lower_bound())
            break;
          if (is_colliding(s_i_from, s_i_to, s_j_from, s_j_to)) {
            info(0, 0, "including collision betweeen \n- ", i,
                 "_from: ", s_i_from, "\n- ", i, "_to  : ", s_i_to, "\n- ", j,
                 "_from: ", s_j_from, "\n- ", j, "_to  : ", s_j_to);
            return false;
          }
        }
      }
    }
  }
  return true;
}

uint ConfigHasher::operator()(const Config& Q) const
{
  uint hash = Q.size();
  auto h = [&hash](int a) {
    hash ^= (a + 0x9e3779b9 + (hash << 6) + (hash >> 2));
  };
  for (auto& s : Q) {
    h(s->v_from->id);
    h(s->v_to->id);
    h(s->step);
    h(s->total_step);
  }
  return hash;
}

bool operator==(const Config& Q1, const Config& Q2)
{
  if (Q1.size() != Q2.size()) return false;
  for (size_t k = 0; k < Q1.size(); ++k) {
    if (Q1[k]->v_from != Q2[k]->v_from) return false;
    if (Q1[k]->v_to != Q2[k]->v_to) return false;
    if (Q1[k]->step != Q2[k]->step) return false;
    if (Q1[k]->total_step != Q2[k]->total_step) return false;
    if (Q1[k]->past_locs != Q2[k]->past_locs) return false;
  }
  return true;
}

std::ostream& operator<<(std::ostream& os, const State* s)
{
  if (s == nullptr) return os;
  os << "<";
  os << "v_from=" << s->v_from->id << ",";
  os << "v_to=" << s->v_to->id << ",";
  os << "step=" << s->step << ",";
  os << "total_step=" << s->total_step << ",";
  os << "time=" << s->time << ",";
  os << "past_locs=[";
  for (auto v : s->past_locs) os << v->id << ",";
  os << "],";
  os << "space_deviation=" << s->space_deviation << ",";
  os << "time_deviation=" << s->time_deviation;
  os << ">";
  return os;
}

const State* State::load(std::string s, Graph* G, StateManager* state_manager)
{
  auto r_state = std::regex(
      R"(<v_from=(.+),v_to=(.+),step=(.+),total_step=(.+),time=(.+),past_locs=\[(.*)\],space_deviation=(.+),time_deviation=(.+)>)");
  auto r_loc_id = std::regex(R"((.+?),)");
  std::smatch results, m;
  std::regex_match(s, results, r_state);
  assert(results.size() > 0);
  auto v_from = G->V[std::stoi(results[1].str())];
  auto v_to = G->V[std::stoi(results[2].str())];
  auto step = std::stoi(results[3].str());
  auto total_step = std::stoi(results[4].str());
  auto time = std::stof(results[5].str());
  Vertices past_locs;
  {
    auto past_locs_str = results[6].str();
    auto iter = past_locs_str.cbegin();
    while (std::regex_search(iter, past_locs_str.cend(), m, r_loc_id)) {
      iter = m[0].second;
      past_locs.push_back(G->V[std::stoi(m[1].str())]);
    }
  }
  auto space_deviaiton = std::stof(results[7].str());
  auto time_deviaiton = std::stof(results[8].str());
  return state_manager->get_state(v_from, v_to, step, total_step, time,
                                  past_locs, space_deviaiton, time_deviaiton);
}

StateManager StateManager::load(const std::string& filename)
{
  std::ifstream file(filename);
  assert(file);
  std::string line;
  std::smatch results;

  auto r_agent_rad = std::regex(R"(^agent_rad=(.+))");
  auto r_Hz = std::regex(R"(^Hz=(.+))");
  auto r_length_past_locs = std::regex(R"(^length_past_locs=(.+))");
  auto r_space_deviation_sigma_level =
      std::regex(R"(^space_deviation_sigma_level=(.+))");
  auto r_time_deviation_sigma_level =
      std::regex(R"(^time_deviation_sigma_level=(.+))");

  auto state_manager = StateManager();
  while (getline(file, line)) {
    if (std::regex_match(line, results, r_agent_rad)) {
      state_manager.agent_rad = std::stof(results[1].str());
    } else if (std::regex_match(line, results, r_Hz)) {
      state_manager.Hz = std::stoi(results[1].str());
    } else if (std::regex_match(line, results, r_length_past_locs)) {
      state_manager.length_past_locs = std::stoi(results[1].str());
    } else if (std::regex_match(line, results, r_space_deviation_sigma_level)) {
      state_manager.space_deviation_sigma_level = std::stof(results[1].str());
    } else if (std::regex_match(line, results, r_time_deviation_sigma_level)) {
      state_manager.time_deviation_sigma_level = std::stof(results[1].str());
    }
  }
  return state_manager;
}

Plan load_plan(const std::string& filename, Graph* G,
               StateManager* state_manager)
{
  std::ifstream file(filename);
  assert(file);
  std::string line;
  std::smatch results;

  auto r_config = std::regex(R"(^(\d+):(.+))");
  auto r_state = std::regex(R"(<.+?>)");
  auto r_transition_state = std::regex(R"(\(\[.+?\]->\[.+?\],\d+\/\d+\))");

  Plan plan;
  while (getline(file, line)) {
    if (std::regex_match(line, results, r_config)) {
      Config Q;
      const auto t = std::stoi(results[1].str());
      const auto s = results[2].str();

      for (auto iter = std::sregex_iterator(s.begin(), s.end(), r_state);
           iter != std::sregex_iterator(); ++iter) {
        Q.push_back(State::load(iter->str(), G, state_manager));
      }

      if (Q.empty()) {
        for (auto iter =
                 std::sregex_iterator(s.begin(), s.end(), r_transition_state);
             iter != std::sregex_iterator(); ++iter) {
          const auto time = state_manager->Hz > 0
                                ? static_cast<float>(t) / state_manager->Hz
                                : static_cast<float>(t);
          Q.push_back(load_state_by_transition_string(iter->str(), G,
                                                      state_manager, time));
        }
      }

      if (!Q.empty()) {
        plan.push_back(Q);
      }
    }
  }
  return plan;
}

std::ostream& operator<<(std::ostream& os, const Config& Q)
{
  for (auto s : Q) os << s << ",";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Plan& plan)
{
  for (size_t t = 0; t < plan.size(); ++t) os << t << ":" << plan[t] << "\n";
  return os;
}

std::ostream& operator<<(std::ostream& os, const StateManager& state_manager)
{
  os << "agent_rad=" << state_manager.agent_rad << "\n"
     << "Hz=" << state_manager.Hz << "\n"
     << "length_past_locs=" << state_manager.length_past_locs << "\n"
     << "space_deviation_sigma_level="
     << state_manager.space_deviation_sigma_level << "\n"
     << "time_deviation_sigma_level="
     << state_manager.time_deviation_sigma_level << "\n";
  return os;
}

void save_plan_with_path_format(const Plan& plan,
                                const std::string& output_filename)
{
  std::ofstream log;
  log.open(output_filename, std::ios::out);

  if (!plan.empty()) {
    const auto N = plan[0].size();
    const auto T = plan.size();
    for (size_t i = 0; i < N; ++i) {
      Vertex* v_prev = nullptr;
      log << i << ":\n";
      for (size_t t = 0; t < T; ++t) {
        auto s = plan[t][i];
        if (s->step != s->total_step) continue;
        auto v = s->v_to;
        if (t < T - 1 && v == v_prev && v == plan[t + 1][i]->v_to) continue;
        if (t == T - 1 && v == v_prev) continue;
        log << "- loc: " << v->pos << "\n"
            << "  time: " << s->time << "\n"
            << "  id: " << v->id << "\n";
        v_prev = v;
      }
    }
  }
  log.close();
}
