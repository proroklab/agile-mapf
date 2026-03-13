#include "../include/action_model.hpp"

ActionModel::ActionModel()
    : speed_default(1.0),
      time_deviation_var_default(0.01),
      space_deviation_mu_default(0.01),
      space_deviation_var_default(0.01)
{
}

ActionModel::~ActionModel() {}

ArrTimeSpaceMeanVar ActionModel::get_successor_actions(
    const Vertices& past_locs, Vertex* v_from)
{
  ArrTimeSpaceMeanVar entries;
  for (auto v : v_from->neighbors) {
    auto time_mu = dist(v_from, v) / speed_default;
    entries.emplace_back(v, time_mu, time_deviation_var_default,
                         space_deviation_mu_default,
                         space_deviation_var_default);
  }
  return entries;
}
