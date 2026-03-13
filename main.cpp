#include <argparse/argparse.hpp>
#include <lacam.hpp>
#include <planner.hpp>

namespace {

struct HelpEntry {
  const char* names;
  const char* description;
};

bool wants_custom_help(int argc, char* argv[])
{
  for (auto i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
    if (arg == "-h" || arg == "--help") return true;
  }
  return false;
}

void print_help_section(const char* title,
                        std::initializer_list<HelpEntry> entries)
{
  constexpr size_t kHelpNameWidth = 34;
  std::cout << "\n" << title << ":\n";
  for (const auto& entry : entries) {
    const auto names = std::string(entry.names);
    std::cout << "  " << names;
    if (names.size() < kHelpNameWidth) {
      std::cout << std::string(kHelpNameWidth - names.size(), ' ');
    } else {
      std::cout << " ";
    }
    std::cout << entry.description << "\n";
  }
}

void print_custom_help()
{
  std::cout
      << "Usage: lacam [options]\n\n"
      << "Continuous multi-agent path finding solver.\n";

  print_help_section(
      "I/O and Execution",
      {
          {"-h, --help", "show this categorized help and exit"},
          {"-v, --verbose", "logging verbosity (default: 0)"},
          {"-s, --seed", "random seed (default: 0)"},
          {"--output", "output file path (default: ./build/result.txt)"},
          {"--no_validation", "skip solution validation"},
      });

  print_help_section(
      "Solver Parameters",
      {
          {"--action_model",
           "learned action model prefix or .pt/.jit.pt path; when set, "
           "--length_past_locs is inferred from the model"},
          {"--speed_default", "fallback speed without learned model (default: 1.0)"},
          {"--time_deviation_var_default",
           "fallback time deviation variance (default: 0.0)"},
          {"--space_deviation_mu_default",
           "fallback space deviation mean (default: 0.0)"},
          {"--space_deviation_var_default",
           "fallback space deviation variance (default: 0.0)"},
          {"-f, --Hz", "state discretization frequency (default: 4)"},
          {"-l, --length_past_locs",
           "action-model history length when --action_model is not set "
           "(default: 1)"},
          {"--time_sigma_level", "time uncertainty sigma level (default: 0.5)"},
          {"--space_sigma_level",
           "space uncertainty sigma level (default: 0.5)"},
          {"-t, --time_limit_sec", "search time limit in seconds (default: 3.0)"},
          {"-O, --objective",
           "0=first feasible, 1=flowtime, 2=makespan, 3=total distance"},
          {"--num_planners", "number of parallel LaCAM workers (default: 1)"},
          {"--random_insert_prob_init",
           "probability of reinserting the initial node"},
          {"--random_insert_prob_random",
           "probability of reinserting a random solution-path node"},
          {"--restart_prob", "probability of restarting from the initial node"},
          {"--num_monte_calro_sampling",
           "number of low-level Monte Carlo samples"},
          {"--max_space_deviation",
           "maximum allowed space deviation during planning"},
          {"--no_sort_low_level", "disable low-level successor sorting"},
      });

  print_help_section(
      "Problem Parameters",
      {
          {"-N, --num", "number of agents (default: 1)"},
          {"-a, --agent", "explicit start_id,goal_id pair; may be repeated"},
          {"-r, --agent_rad", "agent radius (default: 0.25)"},
          {"--graph_type", "synthetic graph type: grid or random"},
          {"--graph_file", "load a graph from file instead of generating one"},
          {"-o, --obstacle", "manual obstacle as x,y,z,r; may be repeated"},
          {"--num_random_obstacles",
           "number of randomly generated obstacles (default: 0)"},
          {"--obstacle_rad_min",
           "minimum random obstacle radius (default: 0.2)"},
          {"--obstacle_rad_max",
           "maximum random obstacle radius (default: 0.5)"},
          {"--x_min", "workspace minimum x (default: -2.0)"},
          {"--x_max", "workspace maximum x (default: 2.0)"},
          {"--y_min", "workspace minimum y (default: -1.5)"},
          {"--y_max", "workspace maximum y (default: 1.5)"},
          {"--z_min", "workspace minimum z (default: 0.0)"},
          {"--z_max", "workspace maximum z (default: 0.0)"},
          {"--num_vertices", "number of vertices for random graphs (default: 50)"},
          {"--step_size", "lattice spacing for grid graphs (default: 0.75)"},
          {"--connection_rad", "edge connection radius (default: 1.75)"},
      });
}

}  // namespace

int main(int argc, char* argv[])
{
  if (wants_custom_help(argc, argv)) {
    print_custom_help();
    return 0;
  }

  auto deadline = Deadline();
  constexpr const char* kIoPrefix = "[io] ";
  constexpr const char* kSolverPrefix = "[solver] ";
  constexpr const char* kProblemPrefix = "[problem] ";
  constexpr const char* kObjectiveHelp =
      "[solver] optimization target: 0=first feasible solution (stop on first "
      "goal), 1=flowtime, 2=makespan, 3=total distance";
  constexpr const char* kTimeLimitHelp =
      "[solver] search time limit in seconds; with objectives 1-3 the solver "
      "keeps the best solution found so far until this limit or until the "
      "search space is exhausted";
  constexpr const char* kActionModelHelp =
      "[solver] learned action model prefix or .pt/.jit.pt path; when set, "
      "--length_past_locs is inferred from the model definition";
  constexpr const char* kLengthPastLocsHelp =
      "[solver] action-model history length; effective only when "
      "--action_model is not set";
  constexpr const char* kHelpEpilog =
      "Option categories: [io] input/output and execution control, "
      "[solver] search and action-model parameters, [problem] graph and "
      "instance generation.";

  argparse::ArgumentParser program("lacam", "0.1.0");
  program.add_epilog(kHelpEpilog);

  program.add_argument("-v", "--verbose")
      .help("[io] logging verbosity")
      .default_value(0)
      .scan<'i', int>();
  program.add_argument("-s", "--seed")
      .help("[io] random seed")
      .default_value(0)
      .scan<'i', int>();
  program.add_argument("-N", "--num")
      .help("[problem] number of agents")
      .default_value(1)
      .scan<'i', int>();
  program.add_argument("--output")
      .help("[io] output file")
      .default_value(std::string("./build/result.txt"));
  program.add_argument("-a", "--agent")
      .default_value(std::vector<std::string>({}))
      .append()
      .help("[problem] explicit start_id,goal_id");
  program.add_argument("--no_validation")
      .help("[io] skip solution validation")
      .flag();

  program.add_argument("--action_model")
      .help(kActionModelHelp)
      .default_value(std::string(""));
  program.add_argument("--speed_default")
      .help("[solver] fallback speed when no learned action model is used")
      .default_value(1.0f)
      .scan<'g', float>();
  program.add_argument("--time_deviation_var_default")
      .help("[solver] fallback time deviation variance without learned model")
      .default_value(0.0f)
      .scan<'g', float>();
  program.add_argument("--space_deviation_mu_default")
      .help("[solver] fallback space deviation mean without learned model")
      .default_value(0.0f)
      .scan<'g', float>();
  program.add_argument("--space_deviation_var_default")
      .help("[solver] fallback space deviation variance without learned model")
      .default_value(0.0f)
      .scan<'g', float>();

  program.add_argument("-f", "--Hz")
      .help("[solver] state discretization frequency")
      .default_value(4)
      .scan<'i', int>();
  program.add_argument("-r", "--agent_rad")
      .help("[problem] agent radius")
      .default_value(0.25f)
      .scan<'g', float>();
  program.add_argument("-l", "--length_past_locs")
      .help(kLengthPastLocsHelp)
      .default_value(1)
      .scan<'i', int>();
  program.add_argument("--time_sigma_level")
      .help("[solver] time uncertainty sigma level")
      .default_value(0.5f)
      .scan<'g', float>();
  program.add_argument("--space_sigma_level")
      .help("[solver] space uncertainty sigma level")
      .default_value(0.5f)
      .scan<'g', float>();

  program.add_argument("-t", "--time_limit_sec")
      .help(kTimeLimitHelp)
      .default_value(3.0f)
      .scan<'g', float>();
  program.add_argument("-O", "--objective")
      .help(kObjectiveHelp)
      .default_value(0)
      .scan<'i', int>();
  program.add_argument("--num_planners")
      .help("[solver] number of parallel LaCAM workers")
      .default_value(1)
      .scan<'i', int>();
  program.add_argument("--random_insert_prob_init")
      .help("[solver] probability of reinserting the initial node")
      .default_value(0.0025f)
      .scan<'g', float>();
  program.add_argument("--random_insert_prob_random")
      .help("[solver] probability of reinserting a random solution-path node")
      .default_value(0.0025f)
      .scan<'g', float>();
  program.add_argument("--restart_prob")
      .help("[solver] probability of restarting from the initial node")
      .default_value(0.001f)
      .scan<'g', float>();
  program.add_argument("--num_monte_calro_sampling")
      .help("[solver] number of low-level Monte Carlo samples")
      .default_value(10)
      .scan<'i', int>();
  program.add_argument("--max_space_deviation")
      .help("[solver] maximum allowed space deviation during planning")
      .default_value(10.0f)
      .scan<'g', float>();
  program.add_argument("--no_sort_low_level")
      .help("[solver] disable low-level successor sorting")
      .flag();

  program.add_argument("--graph_type")
      .help("[problem] synthetic graph type: grid or random")
      .default_value(std::string("grid"));
  program.add_argument("--graph_file")
      .help("[problem] load a graph from file instead of generating one")
      .default_value(std::string(""));
  program.add_argument("-o", "--obstacle")
      .default_value(std::vector<std::string>({}))
      .append()
      .help("[problem] manual obstacle as x,y,z,r");
  program.add_argument("--num_random_obstacles")
      .help("[problem] number of randomly generated obstacles")
      .default_value(0)
      .scan<'i', int>();
  program.add_argument("--obstacle_rad_min")
      .help("[problem] minimum random obstacle radius")
      .default_value(0.2f)
      .scan<'g', float>();
  program.add_argument("--obstacle_rad_max")
      .help("[problem] maximum random obstacle radius")
      .default_value(0.5f)
      .scan<'g', float>();
  program.add_argument("--x_min")
      .help("[problem] workspace minimum x")
      .default_value(-2.0f)
      .scan<'g', float>();
  program.add_argument("--x_max")
      .help("[problem] workspace maximum x")
      .default_value(2.0f)
      .scan<'g', float>();
  program.add_argument("--y_min")
      .help("[problem] workspace minimum y")
      .default_value(-1.5f)
      .scan<'g', float>();
  program.add_argument("--y_max")
      .help("[problem] workspace maximum y")
      .default_value(1.5f)
      .scan<'g', float>();
  program.add_argument("--z_min")
      .help("[problem] workspace minimum z")
      .default_value(0.0f)
      .scan<'g', float>();
  program.add_argument("--z_max")
      .help("[problem] workspace maximum z")
      .default_value(0.0f)
      .scan<'g', float>();
  program.add_argument("--num_vertices")
      .help("[problem] number of vertices for random graphs")
      .default_value(50)
      .scan<'i', int>();
  program.add_argument("--step_size")
      .help("[problem] lattice spacing for grid graphs")
      .default_value(0.75f)
      .scan<'g', float>();
  program.add_argument("--connection_rad")
      .help("[problem] edge connection radius")
      .default_value(1.75f)
      .scan<'g', float>();

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  const auto verbose = program.get<int>("--verbose");
  const auto seed = program.get<int>("--seed");
  const auto agents_raw = program.get<std::vector<std::string>>("--agent");

  const auto model_fpath = program.get<std::string>("--action_model");
  const auto speed_default = program.get<float>("--speed_default");
  const auto time_deviation_var_default =
      program.get<float>("--time_deviation_var_default");
  const auto space_deviation_mu_default =
      program.get<float>("--space_deviation_mu_default");
  const auto space_deviation_var_default =
      program.get<float>("--space_deviation_var_default");

  const auto Hz = program.get<int>("--Hz");
  const auto agent_rad = program.get<float>("--agent_rad");
  const auto length_past_locs = program.get<int>("--length_past_locs");

  const auto time_limit_sec = program.get<float>("--time_limit_sec");
  const auto num_planners = program.get<int>("--num_planners");
  const auto no_sort_low_level = program.get<bool>("--no_sort_low_level");

  const auto graph_file = program.get<std::string>("--graph_file");
  const auto obstacles_raw = program.get<std::vector<std::string>>("--obstacle");

  const auto problem_options = ProblemBuilderOptions{
      .seed = seed,
      .num_agents = program.get<int>("--num"),
      .agent_rad = agent_rad,
      .graph_type = program.get<std::string>("--graph_type"),
      .graph_file = graph_file,
      .obstacles_raw = obstacles_raw,
      .num_random_obstacles = program.get<int>("--num_random_obstacles"),
      .obstacle_rad_min = program.get<float>("--obstacle_rad_min"),
      .obstacle_rad_max = program.get<float>("--obstacle_rad_max"),
      .x_min = program.get<float>("--x_min"),
      .x_max = program.get<float>("--x_max"),
      .y_min = program.get<float>("--y_min"),
      .y_max = program.get<float>("--y_max"),
      .z_min = program.get<float>("--z_min"),
      .z_max = program.get<float>("--z_max"),
      .num_vertices = program.get<int>("--num_vertices"),
      .step_size = program.get<float>("--step_size"),
      .connection_rad = program.get<float>("--connection_rad"),
      .agents_raw = agents_raw,
  };

  Graph G = build_graph(problem_options);
  Instance ins = build_instance(&G, problem_options);

  auto planner_options = PlannerOptions{
      .model_fpath = model_fpath,
      .verbose = verbose,
      .seed = seed,
      .speed_default = speed_default,
      .time_deviation_var_default = time_deviation_var_default,
      .space_deviation_mu_default = space_deviation_mu_default,
      .space_deviation_var_default = space_deviation_var_default,
      .Hz = Hz,
      .agent_rad = agent_rad,
      .length_past_locs = length_past_locs,
      .time_sigma_level = program.get<float>("--time_sigma_level"),
      .space_sigma_level = program.get<float>("--space_sigma_level"),
      .time_limit_sec = time_limit_sec,
      .objective = static_cast<Objective>(program.get<int>("--objective")),
      .num_planners = num_planners,
      .random_insert_prob_init =
          program.get<float>("--random_insert_prob_init"),
      .random_insert_prob_random =
          program.get<float>("--random_insert_prob_random"),
      .restart_prob = program.get<float>("--restart_prob"),
      .num_monte_calro_sampling =
          program.get<int>("--num_monte_calro_sampling"),
      .max_space_deviation = program.get<float>("--max_space_deviation"),
      .sort_low_level = !no_sort_low_level,
  };

  auto planner = Planner(&ins, planner_options);

  synchronized_info(1, verbose, format_elapsed_ms(deadline.elapsed_ms()),
                    ", setup done");
  planner.solve();

  // save results
  planner.save(program.get<std::string>("--output"), deadline.elapsed_ms());
  synchronized_info(1, verbose, format_elapsed_ms(deadline.elapsed_ms()),
                    ", save result");
  if (planner.solved()) {
    synchronized_info(1, verbose, "best_score=", planner.solution_cost());
  }

  // validation
  if (!program.get<bool>("--no_validation")) {
    synchronized_info(1, verbose, "validation: ", planner.validate_solution());
  }

  synchronized_info(1, verbose, format_elapsed_ms(deadline.elapsed_ms()),
                    ", success");

  return 0;
}
