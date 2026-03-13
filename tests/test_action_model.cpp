#include <cassert>
#include <iostream>
#include <lacam.hpp>

int main()
{
  {
    auto G = Graph();
    auto v1 = new Vertex(0, 0);
    auto v2 = new Vertex(1.0, 0);
    auto v3 = new Vertex(2.2535, 0);
    v1->neighbors.push_back(v2);
    v2->neighbors.push_back(v1);
    v2->neighbors.push_back(v3);
    v3->neighbors.push_back(v2);
    G.V.push_back(v1);
    G.V.push_back(v2);
    G.V.push_back(v3);

    auto fpath = std::string("assets/robomaster_example_model/best.jit.pt");
    auto action_model = ActionModelTorch(fpath);

    auto past_locs = Vertices({v1, v1});
    auto v_from = v2;
    auto timer1 = Deadline(3000);
    auto&& successors = action_model.get_successor_actions(past_locs, v_from);
    std::cout << timer1.elapsed_ns() * 1e-6 << " ms" << std::endl;
    for (auto&& action : successors) {
      std::cout << v_from->id << " -> " << std::get<0>(action)->id << "\t"
                << std::get<1>(action) << "\t" << std::get<2>(action) << "\t"
                << std::get<3>(action) << "\t" << std::get<4>(action)
                << std::endl;
    }

    // with cache
    auto timer2 = Deadline(3000);
    auto&& successors_second_time =
        action_model.get_successor_actions(past_locs, v_from);
    std::cout << timer2.elapsed_ns() * 1e-6 << " ms" << std::endl;
    assert(successors.size() == successors_second_time.size());

    auto prefix =
        std::string("assets/robomaster_example_model/best");
    auto action_model_from_prefix = ActionModelTorch(prefix);
    auto&& successors_from_prefix =
        action_model_from_prefix.get_successor_actions(past_locs, v_from);
    assert(successors.size() == successors_from_prefix.size());
    for (size_t i = 0; i < successors.size(); ++i) {
      assert(std::get<0>(successors[i])->id == std::get<0>(successors_from_prefix[i])->id);
      assert(std::get<1>(successors[i]) == std::get<1>(successors_from_prefix[i]));
      assert(std::get<2>(successors[i]) == std::get<2>(successors_from_prefix[i]));
      assert(std::get<3>(successors[i]) == std::get<3>(successors_from_prefix[i]));
      assert(std::get<4>(successors[i]) == std::get<4>(successors_from_prefix[i]));
    }
  }

  return 0;
}
