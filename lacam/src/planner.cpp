#include "../include/planner.hpp"

#include <thread>

namespace {

constexpr float kNoSolutionScore = 10000000.0f;
const std::vector<float> kSuoMargins = {1.0f, 1.01f, 1.05f, 1.1f};

}  // namespace

Planner::Planner(Instance* _ins, const PlannerOptions& _options)
    : ins(_ins),
      options(_options),
      deadline(),
      action_models(options.num_planners),
      state_managers(options.num_planners),
      lacams(options.num_planners),
      best_planner_idx(0),
      best_score(std::numeric_limits<float>::max())
{
}

void Planner::solve()
{
  auto worker = [&](const int k) {
    worker_info(1, k, "start");

    auto action_model = (options.model_fpath.empty())
                            ? std::make_unique<ActionModel>()
                            : std::make_unique<ActionModelTorch>(
                                  options.model_fpath);
    if (k == 0 && !options.model_fpath.empty()) {
      worker_info(1, k, "loaded action model: ", options.model_fpath);
    }
    if (options.model_fpath.empty()) {
      action_model->speed_default = options.speed_default;
      action_model->time_deviation_var_default =
          options.time_deviation_var_default;
      action_model->space_deviation_mu_default =
          options.space_deviation_mu_default;
      action_model->space_deviation_var_default =
          options.space_deviation_var_default;
      action_model->length_past_locs = options.length_past_locs;
    }

    auto state_manager = std::make_unique<StateManager>(action_model.get());
    state_manager->set_agent_rad(options.agent_rad);
    state_manager->Hz = options.Hz;
    state_manager->space_deviation_sigma_level = options.space_sigma_level;
    state_manager->time_deviation_sigma_level = options.time_sigma_level;

    auto lacam = std::make_unique<LaCAM>(
        ins, state_manager.get(), options.time_limit_sec * 1000, options.seed + k);
    lacam->set_verbose(options.verbose);
    lacam->log_prefix = "planner[" + std::to_string(k) + "] ";
    lacam->suo.log_prefix = lacam->log_prefix;
    lacam->random_insert_prob_init = options.random_insert_prob_init;
    lacam->random_insert_prob_random = options.random_insert_prob_random;
    lacam->restart_prob = options.restart_prob;
    lacam->num_monte_calro_sampling = options.num_monte_calro_sampling;
    lacam->max_space_deviation = options.max_space_deviation;
    lacam->objective = options.objective;
    lacam->flg_sort_low_level = options.sort_low_level;

    if (k % (kSuoMargins.size() + 1) != kSuoMargins.size() - 1) {
      lacam->flg_suo = true;
      lacam->suo.cost_margin = kSuoMargins[k % (kSuoMargins.size() + 1)];
    } else {
      lacam->flg_suo = false;
    }

    lacam->solve();

    if (lacam->solution.empty()) {
      worker_info(1, k, "finished without solution");
    } else {
      worker_info(1, k, "finished with cost=", lacam->solution_cost);
    }

    action_models[k] = std::move(action_model);
    state_managers[k] = std::move(state_manager);
    lacams[k] = std::move(lacam);
  };

  auto threads = std::vector<std::thread>();
  threads.reserve(options.num_planners);
  for (auto k = 0; k < options.num_planners; ++k) {
    threads.emplace_back(worker, k);
  }
  for (auto& th : threads) th.join();

  best_score = std::numeric_limits<float>::max();
  best_planner_idx = 0;
  for (size_t k = 0; k < lacams.size(); ++k) {
    const auto& lacam = lacams[k];
    if (lacam == nullptr || lacam->solution.empty()) continue;
    if (lacam->solution_cost < best_score) {
      best_score = lacam->solution_cost;
      best_planner_idx = static_cast<int>(k);
    }
  }

  planner_info(1, "finish solvers");
  if (best_score < kNoSolutionScore) {
    worker_info(1, best_planner_idx, "selected as best, cost=", best_score);
  }
}

void Planner::save(const std::string& output_filename, int comp_time_ms)
{
  auto* lacam = best_lacam();
  if (lacam == nullptr) return;
  const auto elapsed_ms =
      (comp_time_ms < 0) ? deadline.elapsed_ms() : comp_time_ms;
  lacam->save(output_filename, elapsed_ms);
}

bool Planner::validate_solution()
{
  auto* lacam = best_lacam();
  return lacam != nullptr && lacam->validate_solution();
}

bool Planner::solved() const { return best_lacam() != nullptr; }

float Planner::solution_cost() const
{
  auto* lacam = best_lacam();
  return (lacam == nullptr) ? std::numeric_limits<float>::max()
                            : lacam->solution_cost;
}

LaCAM* Planner::best_lacam()
{
  if (best_planner_idx < 0 || best_planner_idx >= static_cast<int>(lacams.size()))
    return nullptr;
  auto& lacam = lacams[best_planner_idx];
  if (lacam == nullptr || lacam->solution.empty()) return nullptr;
  return lacam.get();
}

const LaCAM* Planner::best_lacam() const
{
  if (best_planner_idx < 0 || best_planner_idx >= static_cast<int>(lacams.size()))
    return nullptr;
  const auto& lacam = lacams[best_planner_idx];
  if (lacam == nullptr || lacam->solution.empty()) return nullptr;
  return lacam.get();
}
