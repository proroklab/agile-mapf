#include <cassert>
#include <filesystem>
#include <iostream>
#include <lacam.hpp>

int main()
{
  std::filesystem::create_directories(".local");

  {
    // construct graph
    auto G = Graph();
    auto v1 = new Vertex(0, 0);
    auto v2 = new Vertex(1, 0);
    v1->neighbors.push_back(v2);
    v2->neighbors.push_back(v1);
    G.V.push_back(v1);
    G.V.push_back(v2);

    assert(G.size() == 2);
  }

  {
    Obstacles obstacles;
    auto G = get_lattice_roadmap(obstacles, 0.5, -1, 1, -1, 1, 0, 0, 0.5, 0.25);
    assert(G.size() == 25);
    assert(G.V[0]->neighbors.size() == 2);
  }

  std::string filename = ".local/graph-3.txt";
  {
    Obstacles obstacles;
    obstacles.push_back(std::make_tuple(Pos(0, 0, 0), 0.26));
    auto G = get_lattice_roadmap(obstacles, 0.5, -1, 1, -1, 1, 0, 0, 0.5, 0.25);
    assert(G.size() == 20);
    G.save(filename);
  }

  {
    auto G = Graph::load(filename);
    G.save(filename + ".copy.txt");
  }

  {
    Obstacles obstacles;
    auto G = get_random_roadmap(obstacles, 30);
    assert(G.size() == 30);
    G.save(".local/graph-4.txt");
  }

  return 0;
}
