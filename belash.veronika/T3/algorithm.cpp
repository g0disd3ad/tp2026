#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <istream>
#include <numeric>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>

using namespace std::placeholders;

struct Point {
    int x, y;
};

struct Polygon {
    std::vector<Point> points;
};

bool operator==(const Point& a, const Point& b) {
    return (a.x == b.x) && (a.y == b.y);
}

bool operator!=(const Point& a, const Point& b) {
    return !(a == b);
}

bool operator==(const Polygon& a, const Polygon& b) {
    if (a.points.size() == b.points.size()) {
        return std::equal(a.points.begin(), a.points.end(),
            b.points.begin());
    }
    return false;
}

std::istream& operator>>(std::istream& in, Point& dest) {
    std::istream::sentry sentry(in);
    if (!sentry) return in;
    char c1, c2, c3;
    int x, y;
    if (in >> c1 >> x >> c2 >> y >> c3
        && c1 == '(' && c2 == ';' && c3 == ')') {
        dest = { x, y };
    }
    else {
        in.setstate(std::ios::failbit);
    }
    return in;
}

std::istream& operator>>(std::istream& in, Polygon& dest) {
    std::istream::sentry sentry(in);
    if (!sentry) return in;
    std::size_t n;
    if (!(in >> n) || n < 3) {
        in.setstate(std::ios::failbit);
        return in;
    }
    Polygon tmp;
    tmp.points.resize(n);
    std::copy_n(std::istream_iterator<Point>(in), n,
        tmp.points.begin());
    if (in) dest = std::move(tmp);
    return in;
}

struct GaussSum {
    const std::vector<Point>& p;
    mutable std::size_t i;
    explicit GaussSum(const std::vector<Point>& points)
        : p(points), i(0) {
    }
    long long operator()(long long acc, const Point& curr) const {
        const Point& next = p[(i + 1) % p.size()];
        long long term = static_cast<long long>(curr.x) * next.y
            - static_cast<long long>(next.x) * curr.y;
        ++i;
        return acc + term;
    }
};

double area(const Polygon& poly) {
    long long sum = std::accumulate(poly.points.begin(),
        poly.points.end(), 0LL,
        GaussSum(poly.points));
    return std::abs(sum) / 2.0;
}

struct SumArea {
    double operator()(double acc, const Polygon& p) const {
        return acc + area(p);
    }
};

struct IsEven {
    bool operator()(const Polygon& p) const {
        return p.points.size() % 2 == 0;
    }
};

struct SumIfEven {
    double operator()(double acc, const Polygon& p) const {
        return IsEven()(p) ? acc + area(p) : acc;
    }
};

struct SumIfOdd {
    double operator()(double acc, const Polygon& p) const {
        return !IsEven()(p) ? acc + area(p) : acc;
    }
};

struct AreaIfVertexCount {
    std::size_t n;
    explicit AreaIfVertexCount(std::size_t n) : n(n) {}
    double operator()(double acc, const Polygon& p) const {
        return (p.points.size() == n) ? acc + area(p) : acc;
    }
};

struct AreaCompare {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return area(a) < area(b);
    }
};

struct NumOfVertexesCompare {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.points.size() < b.points.size();
    }
};

struct IsNumOfVertexes {
    std::size_t n;
    explicit IsNumOfVertexes(std::size_t n) : n(n) {}
    bool operator()(const Polygon& p) const {
        return p.points.size() == n;
    }
};

struct PointLess {
    bool operator()(const Point& a, const Point& b) const {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    }
};

struct ShiftPoint {
    int dx, dy;
    ShiftPoint(int x, int y) : dx(x), dy(y) {}
    Point operator()(const Point& p) const {
        return Point{ p.x - dx, p.y - dy };
    }
};

struct CyclicShiftEqual {
    const std::vector<Point>& target;
    explicit CyclicShiftEqual(const std::vector<Point>& t) : target(t) {}

    bool operator()(const std::vector<Point>& other) const {
        if (target.size() != other.size()) return false;
        if (target.empty()) return true;

        auto it = std::find(other.begin(), other.end(), target[0]);
        if (it == other.end()) return false;

        std::size_t start = std::distance(other.begin(), it);
        std::size_t n = target.size();

        for (std::size_t i = 0; i < n; ++i) {
            if (!(target[i] == other[(start + i) % n])) {
                return false;
            }
        }
        return true;
    }
};

struct IsSameCompatible {
    Polygon target;
    mutable std::vector<Point> targetNorm;

    explicit IsSameCompatible(const Polygon& t) : target(t) {
        auto minIt = std::min_element(target.points.begin(), target.points.end(),
            PointLess());
        int minX = minIt->x;
        int minY = minIt->y;
        targetNorm.resize(target.points.size());
        std::transform(target.points.begin(), target.points.end(), targetNorm.begin(),
            ShiftPoint(minX, minY));
    }

    bool operator()(const Polygon& other) const {
        if (target.points.size() != other.points.size()) return false;

        auto minIt = std::min_element(other.points.begin(), other.points.end(),
            PointLess());
        int minX = minIt->x;
        int minY = minIt->y;
        std::vector<Point> otherNorm(other.points.size());
        std::transform(other.points.begin(), other.points.end(), otherNorm.begin(),
            ShiftPoint(minX, minY));

        return CyclicShiftEqual(targetNorm)(otherNorm);
    }
};

struct RmechoAccumulator {
    const Polygon& target;
    mutable bool lastWasTarget = false;

    explicit RmechoAccumulator(const Polygon& t) : target(t) {}

    std::vector<Polygon> operator()(std::vector<Polygon> acc, const Polygon& p) const {
        if (p == target && lastWasTarget) {
        }
        else {
            acc.push_back(p);
            lastWasTarget = (p == target);
        }
        return acc;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: cannot open file " << argv[1] << "\n";
        return 1;
    }

    std::vector<Polygon> polygons;
    std::string raw;

    while (std::getline(file, raw)) {
        if (raw.empty()) continue;
        std::istringstream iss(raw);
        Polygon poly;
        if (iss >> poly) {
            iss >> std::ws;
            if (iss.eof()) polygons.push_back(std::move(poly));
        }
    }

    std::cout << std::fixed << std::setprecision(1);

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command.empty()) continue;
        std::istringstream iss(command);
        std::string main_cmd;
        iss >> main_cmd;

        if (main_cmd == "AREA") {
            std::string sub;
            iss >> sub;
            if (sub == "EVEN") {
                std::cout << std::accumulate(polygons.begin(),
                    polygons.end(), 0.0, SumIfEven()) << "\n";
            }
            else if (sub == "ODD") {
                std::cout << std::accumulate(polygons.begin(),
                    polygons.end(), 0.0, SumIfOdd()) << "\n";
            }
            else if (sub == "MEAN") {
                if (polygons.empty()) {
                    std::cout << "<INVALID COMMAND>\n";
                }
                else {
                    double total = std::accumulate(polygons.begin(),
                        polygons.end(), 0.0, SumArea());
                    std::cout << total / polygons.size() << "\n";
                }
            }
            else {
                std::istringstream num_stream(sub);
                std::size_t n;
                if (num_stream >> n && num_stream.eof()) {
                    if (n < 3) {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                    else {
                        std::cout << std::accumulate(polygons.begin(),
                            polygons.end(), 0.0,
                            AreaIfVertexCount(n)) << "\n";
                    }
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }
        else if (main_cmd == "MAX") {
            std::string sub;
            iss >> sub;
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>\n";
            }
            else if (sub == "AREA") {
                auto it = std::max_element(polygons.begin(),
                    polygons.end(), AreaCompare());
                std::cout << area(*it) << "\n";
            }
            else if (sub == "VERTEXES") {
                auto it = std::max_element(polygons.begin(),
                    polygons.end(), NumOfVertexesCompare());
                std::cout << it->points.size() << "\n";
            }
            else {
                std::cout << "<INVALID COMMAND>\n";
            }
        }
        else if (main_cmd == "MIN") {
            std::string sub;
            iss >> sub;
            if (polygons.empty()) {
                std::cout << "<INVALID COMMAND>\n";
            }
            else if (sub == "AREA") {
                auto it = std::min_element(polygons.begin(),
                    polygons.end(), AreaCompare());
                std::cout << area(*it) << "\n";
            }
            else if (sub == "VERTEXES") {
                auto it = std::min_element(polygons.begin(),
                    polygons.end(), NumOfVertexesCompare());
                std::cout << it->points.size() << "\n";
            }
            else {
                std::cout << "<INVALID COMMAND>\n";
            }
        }
        else if (main_cmd == "COUNT") {
            std::string sub;
            iss >> sub;
            if (sub == "EVEN") {
                std::cout << std::count_if(polygons.begin(),
                    polygons.end(), IsEven()) << "\n";
            }
            else if (sub == "ODD") {
                auto odd_pred = std::bind(std::logical_not<bool>(),
                    std::bind(IsEven(), _1));
                std::cout << std::count_if(polygons.begin(),
                    polygons.end(), odd_pred) << "\n";
            }
            else {
                std::istringstream num_stream(sub);
                std::size_t n;
                if (num_stream >> n && num_stream.eof()) {
                    if (n < 3) {
                        std::cout << "<INVALID COMMAND>\n";
                    }
                    else {
                        std::cout << std::count_if(polygons.begin(),
                            polygons.end(),
                            IsNumOfVertexes(n)) << "\n";
                    }
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }
        else if (main_cmd == "SAME") {
            Polygon target;
            if (!(iss >> target) || target.points.size() < 3) {
                std::cout << "<INVALID COMMAND>\n";
            }
            else {
                int count = std::count_if(polygons.begin(), polygons.end(),
                    IsSameCompatible(target));
                std::cout << count << "\n";
            }
        }
        else if (main_cmd == "RMECHO") {
            Polygon target;
            if (!(iss >> target) || target.points.size() < 3) {
                std::cout << "<INVALID COMMAND>\n";
            }
            else {
                std::size_t oldSize = polygons.size();
                polygons = std::accumulate(polygons.begin(), polygons.end(),
                    std::vector<Polygon>(), RmechoAccumulator(target));
                std::cout << (oldSize - polygons.size()) << "\n";
            }
        }
        else {
            std::cout << "<INVALID COMMAND>\n";
        }
    }
    return 0;
}
