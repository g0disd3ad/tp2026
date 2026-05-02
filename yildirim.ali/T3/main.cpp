#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iomanip>
#include <string>
#include <cmath>
#include <iterator>
#include <cctype>

struct Point {
    int x, y;
};
struct Polygon {
    std::vector<Point> points;
};
bool operator==(const Point& a, const Point& b) {
    return a.x == b.x && a.y == b.y;
}
bool operator==(const Polygon& a, const Polygon& b) {
    if (a.points.size() != b.points.size()) return false;
    return std::equal(a.points.begin(), a.points.end(), b.points.begin());
}
std::istream& operator>>(std::istream& in, Point& dest) {
    std::istream::sentry sentry(in);
    if (!sentry) return in;
    char c1, c2, c3;
    int x, y;
    if (in >> c1 >> x >> c2 >> y >> c3 &&
        c1 == '(' && c2 == ';' && c3 == ')') {
        dest = {x, y};
    } else {
        in.setstate(std::ios::failbit);
    }
    return in;
}
std::istream& operator>>(std::istream& in, Polygon& dest) {
    std::istream::sentry sentry(in);
    if (!sentry) return in;
    size_t n;
    if (!(in >> n) || n < 3) {
        in.setstate(std::ios::failbit);
        return in;
    }
    Polygon tmp;
    tmp.points.resize(n);
    std::copy_n(std::istream_iterator<Point>(in), n, tmp.points.begin());
    if (in) {
        in >> std::ws;
        if (in.eof()) {
            dest = tmp;
        } else {
            in.setstate(std::ios::failbit);
        }
    }
    return in;
}
struct GaussSum {
    const std::vector<Point>& p;
    mutable size_t i;
    explicit GaussSum(const std::vector<Point>& points) : p(points), i(0) {}
    long long operator()(long long acc, const Point& curr) const {
        const Point& next = p[(i + 1) % p.size()];
        long long term = static_cast<long long>(curr.x) * next.y -
                         static_cast<long long>(next.x) * curr.y;
        ++i;
        return acc + term;
    }
};
double area(const Polygon& poly) {
    if (poly.points.empty()) return 0.0;
    long long sum = std::accumulate(poly.points.begin(), poly.points.end(),
                                    0LL, GaussSum(poly.points));
    return std::abs(sum) / 2.0;
}
struct AreaCalculator {
    double operator()(const Polygon& p) const {
        return area(p);
    }
};
struct IsEvenVertex {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 0;
    }
};
struct IsOddVertex {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 1;
    }
};
struct IsVertexCount {
    size_t target;
    IsVertexCount(size_t n) : target(n) {}
    bool operator()(const Polygon& p) const {
        return p.points.size() == target;
    }
};
struct SumAreaIfEven {
    double operator()(double acc, const Polygon& p) const {
        return IsEvenVertex()(p) ? acc + area(p) : acc;
    }
};
struct SumAreaIfOdd {
    double operator()(double acc, const Polygon& p) const {
        return IsOddVertex()(p) ? acc + area(p) : acc;
    }
};
struct SumAreaIfVertex {
    size_t target;
    SumAreaIfVertex(size_t n) : target(n) {}
    double operator()(double acc, const Polygon& p) const {
        return p.points.size() == target ? acc + area(p) : acc;
    }
};
bool isPermutation(const Polygon& a, const Polygon& b) {
    if (a.points.size() != b.points.size()) return false;
    return std::is_permutation(a.points.begin(), a.points.end(), b.points.begin());
}
struct IsPermutation {
    Polygon target;
    IsPermutation(const Polygon& t) : target(t) {}
    bool operator()(const Polygon& p) const {
        return isPermutation(p, target);
    }
};
struct IsSame {
    Polygon target;
    IsSame(const Polygon& t) : target(t) {}
    bool operator()(const Polygon& p) const {
        return p == target;
    }
};
struct MaxSeqState {
    size_t cur;
    size_t best;
};
struct MaxSeqAccumulator {
    const Polygon& target;
    explicit MaxSeqAccumulator(const Polygon& t) : target(t) {}
    MaxSeqState operator()(MaxSeqState state, const Polygon& p) const {
        if (p == target) {
            state.cur++;
            if (state.cur > state.best) state.best = state.cur;
        } else {
            state.cur = 0;
        }
        return state;
    }
};
std::vector<Polygon> readPolygons(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return {};
    }
    std::vector<Polygon> polygons;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        Polygon p;
        if (iss >> p) {
            iss >> std::ws;
            if (iss.eof()) {
                polygons.push_back(p);
            }
        }
    }
    return polygons;
}
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::vector<Polygon> polygons = readPolygons(argv[1]);
    std::cout << std::fixed << std::setprecision(1);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd.empty()) continue;

        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        if (cmd == "PERMS") {
            Polygon target;
            iss >> target;
            if (iss.fail()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            int cnt = std::count_if(polygons.begin(), polygons.end(),
                std::bind(IsPermutation(target), std::placeholders::_1));
            std::cout << cnt << std::endl;
        }
        else if (cmd == "MAXSEQ") {
            Polygon target;
            iss >> target;
            if (iss.fail()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            MaxSeqAccumulator acc(target);
            MaxSeqState state = std::accumulate(polygons.begin(), polygons.end(),
                MaxSeqState{0, 0}, acc);
            std::cout << state.best << std::endl;
        }
        else if (cmd == "AREA") {
            std::string sub;
            iss >> sub;
            if (sub.empty()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            iss >> std::ws;
            if (!iss.eof()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            std::transform(sub.begin(), sub.end(), sub.begin(), ::toupper);
            if (sub == "MEAN") {
                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                    continue;
                }
                double total = std::accumulate(polygons.begin(), polygons.end(), 0.0,
                    std::bind(std::plus<double>(),
                        std::placeholders::_1,
                        std::bind(AreaCalculator(), std::placeholders::_2)));
                std::cout << total / polygons.size() << std::endl;
            }
            else if (sub == "EVEN") {
                double total = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumAreaIfEven());
                std::cout << total << std::endl;
            }
            else if (sub == "ODD") {
                double total = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumAreaIfOdd());
                std::cout << total << std::endl;
            }
            else {
                std::istringstream num_stream(sub);
                size_t n;
                if (num_stream >> n && num_stream.eof() && n >= 3) {
                    double total = std::accumulate(polygons.begin(), polygons.end(), 0.0, SumAreaIfVertex(n));
                    std::cout << total << std::endl;
                } else {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                }
            }
        }
        else if (cmd == "COUNT") {
            std::string sub;
            iss >> sub;
            if (sub.empty()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            iss >> std::ws;
            if (!iss.eof()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            if (sub == "EVEN") {
                int cnt = std::count_if(polygons.begin(), polygons.end(), IsEvenVertex());
                std::cout << cnt << std::endl;
            }
            else if (sub == "ODD") {
                int cnt = std::count_if(polygons.begin(), polygons.end(), IsOddVertex());
                std::cout << cnt << std::endl;
            }
            else {
                std::istringstream num_stream(sub);
                size_t n;
                if (num_stream >> n && num_stream.eof() && n >= 3) {
                    int cnt = std::count_if(polygons.begin(), polygons.end(), IsVertexCount(n));
                    std::cout << cnt << std::endl;
                } else {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                }
            }
        }
        else if (cmd == "MIN") {
            std::string sub;
            iss >> sub;
            if (sub.empty()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            iss >> std::ws;
            if (!iss.eof()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            std::transform(sub.begin(), sub.end(), sub.begin(), ::toupper);
            if (sub == "AREA") {
                auto it = std::min_element(polygons.begin(), polygons.end(),
                    std::bind(std::less<double>(),
                        std::bind(AreaCalculator(), std::placeholders::_1),
                        std::bind(AreaCalculator(), std::placeholders::_2)));
                std::cout << area(*it) << std::endl;
            }
            else if (sub == "VERTEXES") {
                auto it = std::min_element(polygons.begin(), polygons.end(),
                    [](const Polygon& a, const Polygon& b) {
                        return a.points.size() < b.points.size();
                    });
                std::cout << it->points.size() << std::endl;
            }
            else {
                std::cout << "<INVALID COMMAND>" << std::endl;
            }
        }
        else if (cmd == "MAX") {
            std::string sub;
            iss >> sub;
            if (sub.empty()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            iss >> std::ws;
            if (!iss.eof()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>" << std::endl;
                continue;
            }
            std::transform(sub.begin(), sub.end(), sub.begin(), ::toupper);
            if (sub == "AREA") {
                auto it = std::max_element(polygons.begin(), polygons.end(),
                    std::bind(std::less<double>(),
                        std::bind(AreaCalculator(), std::placeholders::_1),
                        std::bind(AreaCalculator(), std::placeholders::_2)));
                std::cout << area(*it) << std::endl;
            }
            else if (sub == "VERTEXES") {
                auto it = std::max_element(polygons.begin(), polygons.end(),
                    [](const Polygon& a, const Polygon& b) {
                        return a.points.size() < b.points.size();
                    });
                std::cout << it->points.size() << std::endl;
            }
            else {
                std::cout << "<INVALID COMMAND>" << std::endl;
            }
        }
        else {
            std::cout << "<INVALID COMMAND>" << std::endl;
        }
    }
    return 0;
}
//
