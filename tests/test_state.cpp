#include <cassert>
#include <iostream>
#include <lacam.hpp>

int main()
{
  {
    /*
     * 0 - 1 - 2
     *     |
     *     3
     */
    auto G = Graph();
    auto v0 = new Vertex(0, 0);
    auto v1 = new Vertex(1, 0);
    auto v2 = new Vertex(2, 0);
    auto v3 = new Vertex(1, -1);
    auto add_edge = [&](Vertex* u, Vertex* v) {
      u->neighbors.push_back(v);
      v->neighbors.push_back(u);
    };
    add_edge(v0, v1);
    add_edge(v1, v2);
    add_edge(v1, v3);
    G.V = Vertices({v0, v1, v2, v3});

    auto state_manager = StateManager();
    state_manager.Hz = 2;
    state_manager.length_past_locs = 1;
    auto s_init = state_manager.get_state(v0);

    assert(s_init->v_from == v0);
    assert(s_init->v_to == v0);
    assert(s_init->past_locs.size() == 1);
    assert(s_init->past_locs[0] == v0);

    auto successors = state_manager.get_successors(s_init);
    std::cout << "The state " << s_init << " creates successors: " << std::endl;
    for (auto s : successors) std::cout << "- " << s << std::endl;
    assert(successors.size() == 2);

    const State* s_next = nullptr;
    for (auto s : successors)
      if (s->v_to == v1) s_next = s;
    assert(s_next != nullptr);
    successors = state_manager.get_successors(s_next);
    std::cout << "The state " << s_next << " creates successors: " << std::endl;
    for (auto s : successors) std::cout << "- " << s << std::endl;
    assert(successors.size() == 1);

    s_next = successors[0];
    successors = state_manager.get_successors(s_next);
    std::cout << "The state " << s_next << " creates successors: " << std::endl;
    for (auto s : successors) std::cout << "- " << s << std::endl;
    assert(successors.size() == 4);
  }

  return 0;
}
