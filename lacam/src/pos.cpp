#include "../include/pos.hpp"

Pos::Pos(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

Pos::~Pos() {}

Pos Pos::operator+(const Pos& other) const
{
  return Pos(x + other.x, y + other.y, z + other.z);
}

Pos Pos::operator-(const Pos& other) const
{
  return Pos(x - other.x, y - other.y, z - other.z);
}

float Pos::operator*(const Pos& other) const
{
  return x * other.x + y * other.y + z * other.z;
}

Pos Pos::operator*(const float& other) const
{
  return Pos(x * other, y * other, z * other);
}

bool Pos::operator==(const Pos& other) const
{
  return x == other.x && y == other.y && z == other.z;
}

bool Pos::operator!=(const Pos& other) const { return !(operator==(other)); }

float Pos::norm_sqrd() const { return x * x + y * y + z * z; }

float Pos::norm() const { return sqrt(norm_sqrd()); }

std::ostream& operator<<(std::ostream& os, const Pos& pos)
{
  os << "[" << pos.x << "," << pos.y << "," << pos.z << "]";
  return os;
}

float dist_sqrd(const Pos& a, const Pos& b)
{
  const auto x = a.x - b.x;
  const auto y = a.y - b.y;
  const auto z = a.z - b.z;
  return x * x + y * y + z * z;
}

float dist(const Pos& a, const Pos& b) { return (a - b).norm(); }

float dist_between_line_segment_and_point_sqrd(const Pos& a, const Pos& b,
                                               const Pos& c)
{
  // from line[a, b] to point[c]
  auto ab = a - b;
  float df_0 = ab * (b - c);
  float df_1 = ab * (a - c);
  float e = 1;
  if (df_0 * df_1 < 0) {
    e = -df_0 / (ab * ab);
  } else if (df_0 >= 0) {
    e = 0;
  }
  return (a * e + b * (1 - e) - c).norm_sqrd();
}

float dist_between_line_segment_and_point(const Pos& a, const Pos& b,
                                          const Pos& c)
{
  return sqrt(dist_between_line_segment_and_point_sqrd(a, b, c));
}

float dist_between_line_segments_sqrd(const Pos& p1_1, const Pos& p1_2,
                                      const Pos& p2_1, const Pos& p2_2)
{
  auto v1 = p1_2 - p1_1;
  auto v2 = p2_2 - p2_1;
  auto D1 = (p2_1 - p1_1) * v1;
  auto D2 = (p2_1 - p1_1) * v2;

  const float Dv = v1 * v2;
  const float V1 = v1 * v1;
  const float V2 = v2 * v2;
  const float D = V1 * V2 - Dv * Dv;
  if (D > 0) {
    float t1 = (D1 * V2 - D2 * Dv) / D;
    float t2 = (D1 * Dv - D2 * V1) / D;
    if (0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1) {
      auto Q1 = p1_1 + v1 * t1;
      auto Q2 = p2_1 + v2 * t2;
      return dist_sqrd(Q1, Q2);
    }
  }
  return std::min({dist_between_line_segment_and_point_sqrd(p1_1, p1_2, p2_1),
                   dist_between_line_segment_and_point_sqrd(p1_1, p1_2, p2_2),
                   dist_between_line_segment_and_point_sqrd(p2_1, p2_2, p1_1),
                   dist_between_line_segment_and_point_sqrd(p2_1, p2_2, p1_2)});
}

float dist_between_line_segments(const Pos& p1_1, const Pos& p1_2,
                                 const Pos& p2_1, const Pos& p2_2)
{
  return sqrt(dist_between_line_segments_sqrd(p1_1, p1_2, p2_1, p2_2));
}
