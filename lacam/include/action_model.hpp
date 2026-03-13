#pragma once

#include "graph.hpp"
#include "utils.hpp"

using TimeSpaceMeanVar = std::tuple<Vertex*, float, float, float, float>;
using ArrTimeSpaceMeanVar = std::vector<TimeSpaceMeanVar>;

struct ActionModel {
  float speed_default;
  float time_deviation_var_default;
  float space_deviation_mu_default;
  float space_deviation_var_default;
  int length_past_locs;

  ActionModel();
  virtual ~ActionModel();

  virtual ArrTimeSpaceMeanVar get_successor_actions(const Vertices& past_locs,
                                                    Vertex* v_from);
};
