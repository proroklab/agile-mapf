#pragma once
#include "utils.hpp"

struct Pos {
  float x;
  float y;
  float z;

  Pos(float _x, float _y, float _z);
  ~Pos();

  Pos operator+(const Pos& other) const;
  Pos operator-(const Pos& other) const;
  float operator*(const Pos& other) const;  // inner product
  Pos operator*(const float& other) const;
  bool operator==(const Pos& other) const;
  bool operator!=(const Pos& other) const;

  float norm() const;
  float norm_sqrd() const;
};

std::ostream& operator<<(std::ostream& os, const Pos& pos);

float dist_sqrd(const Pos& a, const Pos& b);

float dist(const Pos& a, const Pos& b);

float dist_between_line_segment_and_point_sqrd(const Pos& a, const Pos& b,
                                               const Pos& c);

float dist_between_line_segment_and_point(const Pos& a, const Pos& b,
                                          const Pos& c);

float dist_between_line_segments_sqrd(const Pos& p1_1, const Pos& p1_2,
                                      const Pos& p2_1, const Pos& p2_2);

float dist_between_line_segments(const Pos& p1_1, const Pos& p1_2,
                                 const Pos& p2_1, const Pos& p2_2);
