#include "../include/problem_builder.hpp"

#include <regex>

namespace {

Obstacles parse_obstacles(const std::vector<std::string>& obstacles_raw)
{
  Obstacles obstacles;
  const std::regex r_obs = std::regex(R"((.+),(.+),(.+),(.+))");
  std::smatch results;
  for (auto&& obs_str : obstacles_raw) {
    if (std::regex_match(obs_str, results, r_obs)) {
      obstacles.emplace_back(
          Pos(std::stof(results[1]), std::stof(results[2]),
              std::stof(results[3])),
          std::stof(results[4]));
    }
  }
  return obstacles;
}

float sample_obstacle_coord(std::mt19937& mt, float min_value, float max_value,
                            float radius)
{
  if (max_value - min_value <= radius * 2) {
    return get_random_float(mt, min_value, max_value);
  }
  return get_random_float(mt, min_value + radius, max_value - radius);
}

Obstacles generate_random_obstacles(const ProblemBuilderOptions& options)
{
  Obstacles obstacles;
  if (options.num_random_obstacles <= 0) return obstacles;

  auto mt = std::mt19937(options.seed);
  const auto max_retries = std::max(10, options.num_random_obstacles * 20);
  auto retries = 0;

  while ((int)obstacles.size() < options.num_random_obstacles &&
         retries < max_retries) {
    ++retries;
    const auto radius = get_random_float(mt, options.obstacle_rad_min,
                                         options.obstacle_rad_max);
    const auto pos = Pos(sample_obstacle_coord(mt, options.x_min, options.x_max,
                                               radius),
                         sample_obstacle_coord(mt, options.y_min, options.y_max,
                                               radius),
                         sample_obstacle_coord(mt, options.z_min, options.z_max,
                                               radius));

    auto overlaps = false;
    for (const auto& obstacle : obstacles) {
      const auto& other_pos = std::get<0>(obstacle);
      const auto other_radius = std::get<1>(obstacle);
      if (dist(pos, other_pos) < radius + other_radius) {
        overlaps = true;
        break;
      }
    }
    if (overlaps) continue;

    obstacles.emplace_back(pos, radius);
  }

  return obstacles;
}

std::pair<Vertices, Vertices> parse_agents(Graph* graph,
                                           const std::vector<std::string>& raw)
{
  Vertices starts, goals;
  const std::regex r_agent = std::regex(R"((\d+),(\d+))");
  std::smatch results;
  for (auto&& agent_str : raw) {
    if (std::regex_match(agent_str, results, r_agent)) {
      starts.push_back(graph->V[std::stoi(results[1])]);
      goals.push_back(graph->V[std::stoi(results[2])]);
    }
  }
  return {starts, goals};
}

}  // namespace

Graph build_graph(const ProblemBuilderOptions& options)
{
  if (!options.graph_file.empty()) return Graph::load(options.graph_file);

  auto obstacles = parse_obstacles(options.obstacles_raw);
  auto random_obstacles = generate_random_obstacles(options);
  obstacles.insert(obstacles.end(), random_obstacles.begin(),
                   random_obstacles.end());
  if (options.graph_type == "grid") {
    return get_lattice_roadmap(
        obstacles, options.step_size, options.x_min, options.x_max,
        options.y_min, options.y_max, options.z_min, options.z_max,
        options.connection_rad, options.agent_rad);
  }
  return get_random_roadmap(
      obstacles, options.num_vertices, options.seed, options.x_min,
      options.x_max, options.y_min, options.y_max, options.z_min,
      options.z_max, options.connection_rad, options.agent_rad);
}

Instance build_instance(Graph* graph, const ProblemBuilderOptions& options)
{
  if (!options.agents_raw.empty()) {
    auto [starts, goals] = parse_agents(graph, options.agents_raw);
    return Instance(graph, starts, goals);
  }
  return create_random_instance_from_graph(graph, options.num_agents,
                                           options.seed, options.agent_rad);
}
