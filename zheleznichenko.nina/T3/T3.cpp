#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <iomanip>
#include <numeric>

struct Point {
  int x, y;
};

struct Polygon {
  std::vector<Point> points;
};

double getArea(const Polygon& p) {
  if (p.points.size() < 3) return 0.0;
  double area = 0.0;
  for (size_t i = 0; i < p.points.size(); ++i) {
    const Point& p1 = p.points[i];
    const Point& p2 = p.points[(i + 1) % p.points.size()];
    area += (p1.x * p2.y - p2.x * p1.y);
  }
  return std::abs(area) / 2.0;
}

bool hasRightAngle(const Polygon& p) {
  size_t n = p.points.size();
  for (size_t i = 0; i < n; ++i) {
    Point a = p.points[i];
    Point b = p.points[(i + 1) % n];
    Point c = p.points[(i + 2) % n];

    long long v1x = a.x - b.x;
    long long v1y = a.y - b.y;
    long long v2x = c.x - b.x;
    long long v2y = c.y - b.y;
    if (v1x * v2x + v1y * v2y == 0) return true;
  }
  return false;
}

std::istream& operator>>(std::istream& is, Point& p) {
  char ch = 0;
  is >> std::noskipws;
  is >> ch; if (ch != '(') is.setstate(std::ios::failbit);
  is >> std::skipws >> p.x;
  is >> std::noskipws >> ch; if (ch != ';') is.setstate(std::ios::failbit);
  is >> std::skipws >> p.y;
  is >> std::noskipws >> ch; if (ch != ')') is.setstate(std::ios::failbit);
  return is;
}

bool parsePolygon(const std::string& line, Polygon& poly) {
  std::istringstream iss(line);
  size_t n;
  if (!(iss >> n) || n < 3) return false;
  poly.points.clear();
  for (size_t i = 0; i < n; ++i) {
    Point p;
    if (!(iss >> std::skipws >> p)) return false;
    poly.points.push_back(p);
  }
  std::string extra;
  if (iss >> extra) return false;
  return true;
}

bool equalPoints(const Point& a, const Point& b) { return a.x == b.x && a.y == b.y; }
bool pointLess(const Point& a, const Point& b) { return a.x < b.x || (a.x == b.x && a.y < b.y); }

bool isPerm(const Polygon& a, const Polygon& b) {
  if (a.points.size() != b.points.size()) return false;
  std::vector<Point> pa = a.points, pb = b.points;
  std::sort(pa.begin(), pa.end(), pointLess);
  std::sort(pb.begin(), pb.end(), pointLess);
  return std::equal(pa.begin(), pa.end(), pb.begin(), equalPoints);
}

int main(int argc, char* argv[]) {
  if (argc < 2) return 1;
  std::ifstream file(argv[1]);
  if (!file) return 1;

  std::vector<Polygon> data;
  std::string line;
  while (std::getline(file, line)) {
    Polygon p;
    if (parsePolygon(line, p)) data.push_back(p);
  }

  std::cout << std::fixed << std::setprecision(1);
  std::string cmd;
  while (std::cin >> cmd) {
    if (cmd == "AREA") {
      std::string arg; std::cin >> arg;
      double sum = 0;
      if (arg == "EVEN") {
        for (const auto& p : data) if (p.points.size() % 2 == 0) sum += getArea(p);
        std::cout << sum << "\n";
      }
      else if (arg == "ODD") {
        for (const auto& p : data) if (p.points.size() % 2 != 0) sum += getArea(p);
        std::cout << sum << "\n";
      }
      else if (arg == "MEAN") {
        if (data.empty()) { std::cout << "<INVALID COMMAND>\n"; continue; }
        for (const auto& p : data) sum += getArea(p);
        std::cout << sum / data.size() << "\n";
      }
      else {
        try {
          size_t n = std::stoul(arg);
          if (n < 3) throw std::invalid_argument("");
          for (const auto& p : data) if (p.points.size() == n) sum += getArea(p);
          std::cout << sum << "\n";
        }
        catch (...) { std::cout << "<INVALID COMMAND>\n"; }
      }
    }
    else if (cmd == "MAX") {
      std::string arg; std::cin >> arg;
      if (data.empty()) { std::cout << "<INVALID COMMAND>\n"; continue; }
      if (arg == "AREA") {
        double maxA = 0;
        for (const auto& p : data) maxA = std::max(maxA, getArea(p));
        std::cout << maxA << "\n";
      }
      else if (arg == "VERTEXES") {
        size_t maxV = 0;
        for (const auto& p : data) maxV = std::max(maxV, p.points.size());
        std::cout << maxV << "\n";
      }
      else std::cout << "<INVALID COMMAND>\n";
    }
    else if (cmd == "COUNT") {
      std::string arg; std::cin >> arg;
      if (arg == "EVEN") std::cout << std::count_if(data.begin(), data.end(), [](const Polygon& p) {return p.points.size() % 2 == 0;}) << "\n";
      else if (arg == "ODD") std::cout << std::count_if(data.begin(), data.end(), [](const Polygon& p) {return p.points.size() % 2 != 0;}) << "\n";
      else {
        try {
          size_t n = std::stoul(arg);
          if (n < 3) throw std::invalid_argument("");
          std::cout << std::count_if(data.begin(), data.end(), [n](const Polygon& p) {return p.points.size() == n;}) << "\n";
        }
        catch (...) { std::cout << "<INVALID COMMAND>\n"; }
      }
    }
    else if (cmd == "PERMS") {
      std::string rest; std::getline(std::cin, rest);
      Polygon target;
      if (parsePolygon(rest, target))
        std::cout << std::count_if(data.begin(), data.end(), std::bind(isPerm, std::placeholders::_1, target)) << "\n";
      else std::cout << "<INVALID COMMAND>\n";
    }
    else if (cmd == "RIGHTSHAPES") {
      std::cout << std::count_if(data.begin(), data.end(), hasRightAngle) << "\n";
    }
    else {
      std::cout << "<INVALID COMMAND>\n";
      std::cin.ignore(10000, '\n');
    }
  }
  return 0;
}
