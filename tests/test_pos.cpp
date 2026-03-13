#include <cassert>
#include <lacam.hpp>

int main()
{
  {
    auto p1_1 = Pos(-1.0, -2.0, 0.0);
    auto p1_2 = Pos(1.0, 2.0, 0.0);
    auto p2_1 = Pos(1.0, -2.0, 0.0);
    auto p2_2 = Pos(-1.0, 2.0, 0.0);

    assert(dist_between_line_segments(p1_1, p1_2, p2_1, p2_2) == 0);
  }

  return 0;
}
