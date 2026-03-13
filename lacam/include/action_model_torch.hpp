#pragma once

#include <Eigen/Dense>
#include <cmath>  // exp

#include "action_model.hpp"
#include "graph.hpp"
#include "utils.hpp"

using namespace Eigen;

struct ActionModelTorch : ActionModel {
  std::unordered_map<std::vector<int>,
                     std::tuple<VectorXf, VectorXf, VectorXf, VectorXf>,
                     IntVectorHash, IntVectorEqual>
      cache_mu_var;
  int dim_workspace;

  // model parameters
  std::vector<MatrixXf> weights;
  std::vector<VectorXf> biases;

  ActionModelTorch(const std::string& model_fpath = "");
  ~ActionModelTorch();

  ArrTimeSpaceMeanVar get_successor_actions(const Vertices& past_locs,
                                            Vertex* v_from) override;
};
