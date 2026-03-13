#include "../include/dist_table.hpp"

#include <iostream>
#include <queue>

constexpr float INF = std::numeric_limits<float>::max();

DistTable::DistTable(const Instance *_ins)
    : ins(_ins), table(ins->N, std::vector<float>(ins->G->V.size(), INF))
{
}

void DistTable::set()
{
  for (auto i = 0; i < ins->N; ++i) set(i);
}

void DistTable::set(const int i)
{
  OPEN.emplace_back();
  OPEN[i].push(std::make_tuple(0.0, ins->goals[i]));
}

float DistTable::get(const int i, const Vertex *v_target)
{
  if (table[i][v_target->id] < INF) return table[i][v_target->id];

  while (!OPEN[i].empty()) {
    auto N = OPEN[i].top();
    OPEN[i].pop();
    auto g_v = std::get<0>(N);
    auto v = std::get<1>(N);
    if (table[i][v->id] <= g_v) continue;

    table[i][v->id] = g_v;
    for (auto u : v->neighbors) {
      auto g_u = g_v + dist(u, v);
      if (table[i][u->id] <= g_u) continue;
      OPEN[i].push(std::make_tuple(g_u, u));
    }

    if (v == v_target) return table[i][v_target->id];
  }

  return INF;
}
