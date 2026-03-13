#include "../include/action_model_torch.hpp"

#include <yaml-cpp/yaml.h>

#include <cmath>  // exp
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "../../third_party/cnpy/cnpy.h"

namespace
{

  bool has_suffix(const std::string& value, const std::string& suffix)
  {
    return value.size() >= suffix.size() &&
           value.compare(value.size() - suffix.size(), suffix.size(), suffix) ==
               0;
  }

  std::string strip_suffix(const std::string& value, const std::string& suffix)
  {
    if (has_suffix(value, suffix)) {
      return value.substr(0, value.size() - suffix.size());
    }
    return "";
  }

  std::string get_model_prefix(const std::string& model_fpath)
  {
    auto prefix = strip_suffix(model_fpath, ".jit.pt");
    if (!prefix.empty()) return prefix;

    prefix = strip_suffix(model_fpath, ".pt");
    if (!prefix.empty()) return prefix;

    if (has_suffix(model_fpath, ".npz") ||
        has_suffix(model_fpath, ".pt_hypra.yaml")) {
      throw std::runtime_error(
          "unsupported action model path: " + model_fpath +
          " (pass a model prefix directly, or a matching .jit.pt/.pt path)");
    }

    return model_fpath;
  }

  void require_file(const std::string& fpath, const std::string& model_fpath)
  {
    if (std::filesystem::exists(fpath)) return;

    throw std::runtime_error(
        "missing action model sidecar file: " + fpath + " for model " +
        model_fpath +
        " (expected .pt_hypra.yaml and .npz alongside the model file)");
  }

}  // namespace

ActionModelTorch::ActionModelTorch(const std::string& model_fpath)
{
  const auto model_prefix = get_model_prefix(model_fpath);
  const auto model_fpath_hypra = model_prefix + ".pt_hypra.yaml";
  const auto model_fpath_npz = model_prefix + ".npz";

  require_file(model_fpath_hypra, model_fpath);
  require_file(model_fpath_npz, model_fpath);

  auto config = YAML::LoadFile(model_fpath_hypra);
  dim_workspace = config["dim_workspace"].as<int>();

  // load model
  auto model_params = cnpy::npz_load(model_fpath_npz);
  for (auto&& itr = model_params.begin(); itr != model_params.end(); ++itr) {
    auto&& lname = itr->first;
    auto&& arr = itr->second;
    auto&& data = arr.data<float>();
    if (lname[lname.size() - 1] == 's') {
      // bias
      biases.emplace_back(Map<VectorXf>(data, arr.shape[0]));
    } else if (lname[lname.size() - 1] == 't') {
      // weight
      weights.emplace_back(Map<Matrix<float, Dynamic, Dynamic, RowMajor>>(
          data, arr.shape[0], arr.shape[1]));
      if (length_past_locs == 0) {
        length_past_locs = (arr.shape[1] / dim_workspace) - 2;
      }
    }
  }
}

ActionModelTorch::~ActionModelTorch() {}

ArrTimeSpaceMeanVar ActionModelTorch::get_successor_actions(
    const Vertices& past_locs, Vertex* v_from)
{
  ArrTimeSpaceMeanVar entries;
  auto&& neighbors = v_from->neighbors;
  auto num_neighbors = (int)neighbors.size();

  // create key
  std::vector<int> key(num_neighbors * (length_past_locs + 1) * dim_workspace);
  int key_idx = 0;
  auto update_key = [&](Vertex* v) {
    key[key_idx++] = (int)((v->pos.x - v_from->pos.x) * 10);
    key[key_idx++] = (int)((v->pos.y - v_from->pos.y) * 10);
    if (dim_workspace == 3) {
      key[key_idx++] = (int)((v->pos.z - v_from->pos.z) * 10);
    }
  };
  for (auto k = 0; k < num_neighbors; ++k) {
    for (auto l = 0; l < length_past_locs; ++l) {
      auto v = neighbors[k];
      update_key(v);
    }
    auto v_to = v_from->neighbors[k];
    update_key(v_to);
  }

  // check cache
  auto itr_mu_var = cache_mu_var.find(key);

  if (itr_mu_var == cache_mu_var.end()) {
    // [feature, batch];
    MatrixXf x =
        MatrixXf::Zero((length_past_locs + 2) * dim_workspace, num_neighbors);
    for (auto k = 0; k < num_neighbors; ++k) {
      for (auto l = 0; l < length_past_locs; ++l) {
        auto v = neighbors[k];
        x(l * dim_workspace, k) = v->pos.x - v_from->pos.x;
        x(l * dim_workspace + 1, k) = v->pos.y - v_from->pos.y;
        if (dim_workspace == 3) {
          x(l * dim_workspace + 2, k) = v->pos.z - v_from->pos.z;
        }
      }
      auto v_to = v_from->neighbors[k];
      x((length_past_locs + 1) * dim_workspace, k) =
          v_to->pos.x - v_from->pos.x;
      x((length_past_locs + 1) * dim_workspace + 1, k) =
          v_to->pos.y - v_from->pos.y;
      if (dim_workspace == 3) {
        x((length_past_locs + 1) * dim_workspace + 2, k) =
            v_to->pos.z - v_from->pos.z;
      }
    }

    // forward
    const auto L = weights.size();
    for (size_t i = 0; i < L; ++i) {
      // weights & bias
      x = (weights[i] * x).colwise() + biases[i];
      // ReLU
      if (i < L - 1) x = x.cwiseMax(0.0f);
    }

    itr_mu_var =
        cache_mu_var
            .emplace(key, std::make_tuple(x.row(0).cwiseMax(0.0f),  // mu
                                          x.row(2).cwiseMax(0.0f),
                                          x.row(1).array().exp(),  // var
                                          x.row(3).array().exp()))
            .first;
  }

  // prepare response
  for (auto k = 0; k < num_neighbors; ++k) {
    entries.emplace_back(neighbors[k], std::get<0>(itr_mu_var->second)(k),
                         std::get<1>(itr_mu_var->second)(k),
                         std::get<2>(itr_mu_var->second)(k),
                         std::get<3>(itr_mu_var->second)(k));
  }

  return entries;
}
