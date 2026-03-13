#include <cassert>
#include <lacam.hpp>

int main()
{
  auto base_options = ProblemBuilderOptions{
      .seed = 7,
      .num_agents = 1,
      .agent_rad = 0.25f,
      .graph_type = "grid",
      .graph_file = "",
      .obstacles_raw = {},
      .num_random_obstacles = 0,
      .obstacle_rad_min = 0.4f,
      .obstacle_rad_max = 0.4f,
      .x_min = -1.0f,
      .x_max = 1.0f,
      .y_min = -1.0f,
      .y_max = 1.0f,
      .z_min = 0.0f,
      .z_max = 0.0f,
      .num_vertices = 25,
      .step_size = 0.5f,
      .connection_rad = 0.5f,
      .agents_raw = {},
  };

  auto graph_without_obstacles = build_graph(base_options);
  assert(graph_without_obstacles.size() == 25);

  auto options_with_random_obstacles = base_options;
  options_with_random_obstacles.num_random_obstacles = 1;

  auto graph_with_random_obstacles_a = build_graph(options_with_random_obstacles);
  auto graph_with_random_obstacles_b = build_graph(options_with_random_obstacles);

  assert(graph_with_random_obstacles_a.size() < graph_without_obstacles.size());
  assert(graph_with_random_obstacles_a.size() == graph_with_random_obstacles_b.size());
  for (int i = 0; i < graph_with_random_obstacles_a.size(); ++i) {
    assert(graph_with_random_obstacles_a.V[i]->pos ==
           graph_with_random_obstacles_b.V[i]->pos);
  }

  return 0;
}
