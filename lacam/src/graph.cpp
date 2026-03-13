#include "../include/graph.hpp"

int Vertex::UUID = 0;

Vertex::Vertex(float _x, float _y, float _z)
    : id(UUID++), pos(Pos(_x, _y, _z)), neighbors()
{
}

Vertex::~Vertex(){};

bool operator==(const Vertices& Q1, const Vertices& Q2)
{
  if (Q1.size() != Q2.size()) return false;
  for (size_t k = 0; k < Q1.size(); ++k) {
    if (Q1[k] != Q2[k]) return false;
  }
  return true;
}

Graph::Graph() : V(), max_edge_dist(0) { Vertex::UUID = 0; }

Graph::~Graph()
{
  for (auto& v : V)
    if (v != nullptr) delete v;
}

int Graph::size() const { return V.size(); }

float dist(Vertex* u, Vertex* v) { return dist(u->pos, v->pos); }

Graph get_lattice_roadmap(const Obstacles& obstacles, const float step_size,
                          const float x_min, const float x_max,
                          const float y_min, const float y_max,
                          const float z_min, const float z_max,
                          const float connection_rad, const float agent_rad)
{
  // vertex creation
  Graph G;
  for (auto x = x_min; x <= x_max; x += step_size) {
    for (auto y = y_min; y <= y_max; y += step_size) {
      for (auto z = z_min; z <= z_max; z += step_size) {
        auto pos = Pos(x, y, z);
        auto flg_collision_free = true;
        for (auto o : obstacles) {
          auto o_pos = std::get<0>(o);
          auto o_rad = std::get<1>(o);
          if ((o_pos - pos).norm() < agent_rad + o_rad) {
            flg_collision_free = false;
            break;
          }
        }
        if (flg_collision_free) G.V.push_back(new Vertex(x, y, z));
      }
    }
  }

  add_edges(G, obstacles, connection_rad, agent_rad);
  return G;
}

Graph get_random_roadmap(const Obstacles& obstacles, const int num_vertices,
                         const int seed, const float x_min, const float x_max,
                         const float y_min, const float y_max,
                         const float z_min, const float z_max,
                         const float connection_rad, const float agent_rad)
{
  Graph G;
  auto MT = std::mt19937(seed);

  while ((int)G.V.size() < num_vertices) {
    auto x = get_random_float(MT, x_min, x_max);
    auto y = get_random_float(MT, y_min, y_max);
    auto z = get_random_float(MT, z_min, z_max);
    auto pos = Pos(x, y, z);
    auto flg_collision_free = true;
    for (auto o : obstacles) {
      auto o_pos = std::get<0>(o);
      auto o_rad = std::get<1>(o);
      if ((o_pos - pos).norm() < agent_rad + o_rad) {
        flg_collision_free = false;
        break;
      }
    }
    if (flg_collision_free) G.V.push_back(new Vertex(x, y, z));
  }

  add_edges(G, obstacles, connection_rad, agent_rad);
  return G;
}

void add_edges(Graph& G, const Obstacles& obstacles, const float connection_rad,
               const float agent_rad)
{
  // clean up
  for (auto v : G.V) v->neighbors.clear();

  // add edges
  for (auto i = 0; i < G.size(); ++i) {
    for (auto j = i + 1; j < G.size(); ++j) {
      auto u = G.V[i];
      auto v = G.V[j];
      auto d = dist(u, v);
      if (d > connection_rad) continue;
      auto flg_collision_free = true;
      for (auto o : obstacles) {
        auto o_pos = std::get<0>(o);
        auto o_rad = std::get<1>(o);
        auto d = dist_between_line_segment_and_point(u->pos, v->pos, o_pos);
        if (d < agent_rad + o_rad) {
          flg_collision_free = false;
          break;
        }
      }
      if (flg_collision_free) {
        u->neighbors.push_back(v);
        v->neighbors.push_back(u);
        G.max_edge_dist = std::max(d, G.max_edge_dist);
      }
    }
  }
}

Graph Graph::load(const std::string& filename)
{
  std::ifstream file(filename);
  assert(file);
  std::string line;
  std::smatch m, results;

  Graph G;
  const std::regex r_num_vertices = std::regex(R"(num_vertices=(\d+))");
  const std::regex r_vertex =
      std::regex(R"(^id=(\d+),pos=\[(.+?),(.+?),(.+?)\],neighbor_id=)");
  const std::regex r_neighbor_id = std::regex(R"((\d+),)");
  std::vector<std::vector<int>> arr_neighbors;

  // parse
  while (getline(file, line)) {
    if (std::regex_match(line, results, r_num_vertices)) {
      G.V = Vertices(std::stoi(results[1].str()), nullptr);
    } else if (std::regex_search(line, results, r_vertex)) {
      auto id = std::stoi(results[1].str());
      auto x = std::stof(results[2].str());
      auto y = std::stof(results[3].str());
      auto z = std::stof(results[4].str());
      G.V[id] = new Vertex(x, y, z);

      std::vector<int> neighbors;
      auto&& str_neighbor_id = line.substr(results.str().length());
      str_neighbor_id = str_neighbor_id.substr(1, str_neighbor_id.length() - 2);
      auto iter = str_neighbor_id.cbegin();
      while (
          std::regex_search(iter, str_neighbor_id.cend(), m, r_neighbor_id)) {
        iter = m[0].second;
        neighbors.push_back(std::stoi(m[1].str()));
      }
      arr_neighbors.push_back(neighbors);
    }
  }
  file.close();

  // add edges
  for (auto i = 0; i < G.size(); ++i) {
    for (auto j : arr_neighbors[i]) {
      G.V[i]->neighbors.push_back(G.V[j]);
      G.max_edge_dist = std::max(dist(G.V[i], G.V[j]), G.max_edge_dist);
    }
  }

  return G;
}

std::ostream& operator<<(std::ostream& os, const Vertices& arr)
{
  for (auto v : arr) os << v << std::endl;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Graph* G)
{
  os << "num_vertices=" << G->size() << "\n";
  for (auto v : G->V) os << v << "\n";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Vertex* v)
{
  os << "id=" << v->id << ",pos=" << v->pos << ",neighbor_id=[";
  for (auto u : v->neighbors) os << u->id << ",";
  os << "]";
  return os;
}

void Graph::save(const std::string& output_filename)
{
  std::ofstream log;
  log.open(output_filename, std::ios::out);
  log << this;
  log.close();
}
