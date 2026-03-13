#pragma once
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using Time = std::chrono::steady_clock;

// time manager
struct Deadline {
  const Time::time_point t_s;
  const double time_limit_ms;

  Deadline(double _time_limit_ms = 0);
  double elapsed_ms() const;
  double elapsed_ns() const;
};

double elapsed_ms(const Deadline *deadline);
double elapsed_ns(const Deadline *deadline);
bool is_expired(const Deadline *deadline);
bool is_expired(const Deadline &deadline);

float get_random_float(std::mt19937 &MT, float from = 0, float to = 1);
float get_random_float(std::mt19937 *MT, float from = 0, float to = 1);
int get_random_int(std::mt19937 &MT, int from = 0, int to = 1);
int get_random_int(std::mt19937 *MT, int from = 0, int to = 1);

template <typename T>
T random_choose(const std::vector<T> &arr, std::mt19937 &MT)
{
  return arr[get_random_int(MT, 0, arr.size() - 1)];
}

template <typename T>
bool is_contained(const T a, const std::vector<T> &arr)
{
  auto itr = std::find(arr.begin(), arr.end(), a);
  return itr != arr.end();
}

template <typename Head, typename... Tail>
void info(const int level, const int verbose, Head &&head, Tail &&...tail);

void info(const int level, const int verbose);
std::mutex &log_mutex();
std::string format_elapsed_ms(double elapsed_ms);

template <typename Head, typename... Tail>
void info(const int level, const int verbose, Head &&head, Tail &&...tail)
{
  if (verbose < level) return;
  std::cout << head;
  info(level, verbose, std::forward<Tail>(tail)...);
}

template <typename... Body>
void info(const int level, const int verbose, const Deadline *deadline,
          Body &&...body)
{
  if (verbose < level) return;
  std::cout << format_elapsed_ms(elapsed_ms(deadline)) << "  ";
  info(level, verbose, (body)...);
}

template <typename... Body>
void synchronized_info(const int level, const int verbose, Body &&...body)
{
  if (verbose < level) return;
  auto oss = std::ostringstream();
  (oss << ... << std::forward<Body>(body));
  auto lock = std::lock_guard<std::mutex>(log_mutex());
  std::cout << oss.str() << std::endl;
}

struct IntVectorHash {
  size_t operator()(const std::vector<int> &v) const
  {
    size_t h = v.size();
    for (auto &i : v) h ^= i + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
  }
};

struct IntVectorEqual {
  bool operator()(const std::vector<int> &lhs,
                  const std::vector<int> &rhs) const
  {
    if (lhs.size() != rhs.size()) return false;
    for (size_t k = 0; k < lhs.size(); ++k)
      if (lhs[k] != rhs[k]) return false;
    return true;
  }
};

std::ostream &operator<<(std::ostream &os, const std::vector<int> &arr);
std::ostream &operator<<(std::ostream &os, const std::list<int> &arr);
std::ostream &operator<<(std::ostream &os, const std::set<int> &arr);
