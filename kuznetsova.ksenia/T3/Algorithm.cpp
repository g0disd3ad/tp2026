#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>

struct Point {
    int x, y;
};

struct Polygon {
    std::vector<Point> points;
};

bool operator==(const Point& a, const Point& b) {
    return (a.x == b.x) && (a.y == b.y);
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

//edit
struct IsRectangle {
    bool operator()(const Polygon& p) const {
        if (p.points.size() != 4) return false;

        std::vector<bool> results(4);
        std::vector<std::size_t> indices = { 0, 1, 2, 3 };

        std::transform(indices.begin(), indices.end(), results.begin(),
            [&p](std::size_t i) -> bool {
                int ax = p.points[(i + 1) % 4].x - p.points[i].x;
                int ay = p.points[(i + 1) % 4].y - p.points[i].y;
                int bx = p.points[(i + 2) % 4].x - p.points[(i + 1) % 4].x;
                int by = p.points[(i + 2) % 4].y - p.points[(i + 1) % 4].y;
                return (ax * bx + ay * by) == 0;
            });

        return std::all_of(results.begin(), results.end(),
            [](bool val) { return val; });
    }
};

//edit
struct IsSameByShift {
    const Polygon& target;
    explicit IsSameByShift(const Polygon& t) : target(t) {}

    bool operator()(const Polygon& p) const {
        if (p.points.size() != target.points.size()) return false;
        if (p.points.empty()) return true;

        int dx = target.points[0].x - p.points[0].x;
        int dy = target.points[0].y - p.points[0].y;

        Polygon shifted;
        shifted.points.resize(p.points.size());
        std::transform(p.points.begin(), p.points.end(),
            shifted.points.begin(),
            [dx, dy](const Point& pt) -> Point {
                return { pt.x + dx, pt.y + dy };
            });

        return std::equal(shifted.points.begin(), shifted.points.end(),
            target.points.begin());
    }
};

struct SameCounter {
    const Polygon& target;
    explicit SameCounter(const Polygon& t) : target(t) {}
    std::size_t operator()(std::size_t acc, const Polygon& p) const {
        return acc + (IsSameByShift(target)(p) ? 1 : 0);
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
                if (num_stream >> n && num_stream.eof() && n >= 3) {
                    std::cout << std::accumulate(polygons.begin(),
                        polygons.end(), 0.0,
                        AreaIfVertexCount(n)) << "\n";
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
                    std::bind(IsEven(), std::placeholders::_1));
                std::cout << std::count_if(polygons.begin(),
                    polygons.end(), odd_pred) << "\n";
            }
            else {
                std::istringstream num_stream(sub);
                std::size_t n;
                if (num_stream >> n && num_stream.eof() && n >= 3) {
                    std::cout << std::count_if(polygons.begin(),
                        polygons.end(),
                        IsNumOfVertexes(n)) << "\n";
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }
        else if (main_cmd == "RECTS") {
            std::cout << std::count_if(polygons.begin(),
                polygons.end(), IsRectangle()) << "\n";
        }
        else if (main_cmd == "SAME") {
            Polygon target;
            if (!(iss >> target)) {
                std::cout << "<INVALID COMMAND>\n";
            }
            else {
                iss >> std::ws;
                if (iss.eof()) {
                    std::size_t count = std::accumulate(polygons.begin(),
                        polygons.end(), std::size_t(0),
                        SameCounter(target));
                    std::cout << count << "\n";
                }
                else {
                    std::cout << "<INVALID COMMAND>\n";
                }
            }
        }
        else {
            std::cout << "<INVALID COMMAND>\n";
        }
    }
    return 0;
}
