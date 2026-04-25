#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <functional>
#include <sstream>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>

struct Point {
    int x, y;
    
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    
    bool operator<(const Point& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

std::ostream& operator<<(std::ostream& out, const Point& p) {
    out << "(" << p.x << ";" << p.y << ")";
    return out;
}

struct Polygon {
    std::vector<Point> points;
    
    bool operator==(const Polygon& other) const {
        if (points.size() != other.points.size()) return false;
        for (size_t i = 0; i < points.size(); ++i) {
            if (points[i] != other.points[i]) return false;
        }
        return true;
    }
    
    bool operator!=(const Polygon& other) const {
        return !(*this == other);
    }
    
    size_t getVertexCount() const {
        return points.size();
    }
    
    double getArea() const {
        if (points.size() < 3) return 0.0;
        
        double area = 0.0;
        for (size_t i = 0; i < points.size(); ++i) {
            const Point& p1 = points[i];
            const Point& p2 = points[(i + 1) % points.size()];
            area += p1.x * p2.y - p2.x * p1.y;
        }
        return std::abs(area) / 2.0;
    }
    
    Point getMinPoint() const {
        if (points.empty()) return Point(0, 0);
        auto it = std::min_element(points.begin(), points.end(),
            [](const Point& a, const Point& b) {
                if (a.x != b.x) return a.x < b.x;
                return a.y < b.y;
            });
        return *it;
    }
    
    Point getMaxPoint() const {
        if (points.empty()) return Point(0, 0);
        auto it = std::max_element(points.begin(), points.end(),
            [](const Point& a, const Point& b) {
                if (a.x != b.x) return a.x < b.x;
                return a.y < b.y;
            });
        return *it;
    }
    
    bool isInsideBoundingBox(const Point& minBound, const Point& maxBound) const {
        Point minP = getMinPoint();
        Point maxP = getMaxPoint();
        return minP.x >= minBound.x && minP.y >= minBound.y &&
               maxP.x <= maxBound.x && maxP.y <= maxBound.y;
    }
};

std::ostream& operator<<(std::ostream& out, const Polygon& poly) {
    out << poly.points.size();
    for (const auto& p : poly.points) {
        out << " " << p;
    }
    return out;
}

bool parsePoint(const std::string& token, Point& p) {
    if (token.empty() || token.front() != '(' || token.back() != ')') {
        return false;
    }
    std::string inner = token.substr(1, token.length() - 2);
    size_t semicolonPos = inner.find(';');
    if (semicolonPos == std::string::npos) {
        return false;
    }
    try {
        p.x = std::stoi(inner.substr(0, semicolonPos));
        p.y = std::stoi(inner.substr(semicolonPos + 1));
        return true;
    } catch (...) {
        return false;
    }
}

bool parsePolygon(const std::string& line, Polygon& poly) {
    std::istringstream iss(line);
    int vertexCount;
    if (!(iss >> vertexCount)) {
        return false;
    }
    if (vertexCount < 3) {
        return false;
    }
    
    std::vector<Point> points;
    for (int i = 0; i < vertexCount; ++i) {
        std::string token;
        if (!(iss >> token)) {
            return false;
        }
        Point p;
        if (!parsePoint(token, p)) {
            return false;
        }
        points.push_back(p);
    }
    
    std::string extra;
    if (iss >> extra) {
        return false;
    }
    
    poly.points = points;
    return true;
}

bool parsePolygonFromArgs(const std::vector<std::string>& args, size_t start, Polygon& poly) {
    if (start >= args.size()) return false;
    
    int vertexCount;
    try {
        vertexCount = std::stoi(args[start]);
    } catch (...) {
        return false;
    }
    
    if (vertexCount < 3 || start + 1 + vertexCount > args.size()) {
        return false;
    }
    
    std::vector<Point> points;
    for (int i = 0; i < vertexCount; ++i) {
        Point p;
        if (!parsePoint(args[start + 1 + i], p)) {
            return false;
        }
        points.push_back(p);
    }
    
    poly.points = points;
    return true;
}

template<bool Odd>
struct SumAreaByParity {
    double operator()(double sum, const Polygon& poly) const {
        bool isOdd = (poly.getVertexCount() % 2 == 1);
        if (isOdd == Odd) {
            return sum + poly.getArea();
        }
        return sum;
    }
};

struct SumAreaByVertexCount {
    int target_;
    SumAreaByVertexCount(int target) : target_(target) {}
    double operator()(double sum, const Polygon& poly) const {
        if (static_cast<int>(poly.getVertexCount()) == target_) {
            return sum + poly.getArea();
        }
        return sum;
    }
};

struct SumArea {
    double operator()(double sum, const Polygon& poly) const {
        return sum + poly.getArea();
    }
};

template<bool Odd>
struct CountByParity {
    bool operator()(const Polygon& poly) const {
        bool isOdd = (poly.getVertexCount() % 2 == 1);
        return isOdd == Odd;
    }
};

struct CountByVertexCount {
    int target_;
    CountByVertexCount(int target) : target_(target) {}
    bool operator()(const Polygon& poly) const {
        return static_cast<int>(poly.getVertexCount()) == target_;
    }
};

struct CompareArea {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.getArea() < b.getArea();
    }
};

struct CompareVertexCount {
    bool operator()(const Polygon& a, const Polygon& b) const {
        return a.getVertexCount() < b.getVertexCount();
    }
};

struct CompareMinPoint {
    bool operator()(const Polygon& a, const Polygon& b) const {
        Point minA = a.getMinPoint();
        Point minB = b.getMinPoint();
        if (minA.x != minB.x) return minA.x < minB.x;
        return minA.y < minB.y;
    }
};

struct CompareMaxPoint {
    bool operator()(const Polygon& a, const Polygon& b) const {
        Point maxA = a.getMaxPoint();
        Point maxB = b.getMaxPoint();
        if (maxA.x != maxB.x) return maxA.x < maxB.x;
        return maxA.y < maxB.y;
    }
};

class PolygonCollection {
private:
    std::vector<Polygon> polygons_;
    
public:
    void addPolygon(const Polygon& poly) {
        polygons_.push_back(poly);
    }
    
    const std::vector<Polygon>& getPolygons() const {
        return polygons_;
    }
    
    size_t size() const {
        return polygons_.size();
    }
    
    double getAreaSumByVertexCount(int vertexCount) const {
        return std::accumulate(polygons_.begin(), polygons_.end(), 0.0, SumAreaByVertexCount(vertexCount));
    }
    
    double getAreaSumByParity(bool odd) const {
        if (odd) {
            return std::accumulate(polygons_.begin(), polygons_.end(), 0.0, SumAreaByParity<true>());
        } else {
            return std::accumulate(polygons_.begin(), polygons_.end(), 0.0, SumAreaByParity<false>());
        }
    }
    
    double getMeanArea() const {
        if (polygons_.empty()) {
            throw std::runtime_error("No shapes");
        }
        double sum = std::accumulate(polygons_.begin(), polygons_.end(), 0.0, SumArea());
        return sum / polygons_.size();
    }
    
    double getMaxArea() const {
        if (polygons_.empty()) {
            throw std::runtime_error("No shapes");
        }
        auto it = std::max_element(polygons_.begin(), polygons_.end(), CompareArea());
        return it->getArea();
    }
    
    size_t getMaxVertexCount() const {
        if (polygons_.empty()) {
            throw std::runtime_error("No shapes");
        }
        auto it = std::max_element(polygons_.begin(), polygons_.end(), CompareVertexCount());
        return it->getVertexCount();
    }
    
    double getMinArea() const {
        if (polygons_.empty()) {
            throw std::runtime_error("No shapes");
        }
        auto it = std::min_element(polygons_.begin(), polygons_.end(), CompareArea());
        return it->getArea();
    }
    
    size_t getMinVertexCount() const {
        if (polygons_.empty()) {
            throw std::runtime_error("No shapes");
        }
        auto it = std::min_element(polygons_.begin(), polygons_.end(), CompareVertexCount());
        return it->getVertexCount();
    }
    
    size_t countByVertexCount(int vertexCount) const {
        return std::count_if(polygons_.begin(), polygons_.end(), CountByVertexCount(vertexCount));
    }
    
    size_t countByParity(bool odd) const {
        if (odd) {
            return std::count_if(polygons_.begin(), polygons_.end(), CountByParity<true>());
        } else {
            return std::count_if(polygons_.begin(), polygons_.end(), CountByParity<false>());
        }
    }
    
    Point getGlobalMinPoint() const {
        if (polygons_.empty()) {
            return Point(0, 0);
        }
        auto it = std::min_element(polygons_.begin(), polygons_.end(), CompareMinPoint());
        return it->getMinPoint();
    }
    
    Point getGlobalMaxPoint() const {
        if (polygons_.empty()) {
            return Point(0, 0);
        }
        auto it = std::max_element(polygons_.begin(), polygons_.end(), CompareMaxPoint());
        return it->getMaxPoint();
    }
    
    int removeConsecutiveDuplicates(const Polygon& target) {
        if (polygons_.empty()) return 0;
        
        int removed = 0;
        std::vector<Polygon> result;
        
        for (size_t i = 0; i < polygons_.size(); ++i) {
            if (polygons_[i] == target) {
                if (i > 0 && result.back() == target) {
                    removed++;
                    continue;
                }
            }
            result.push_back(polygons_[i]);
        }
        
        polygons_ = result;
        return removed;
    }
    
    bool isInsideGlobalBoundingBox(const Polygon& poly) const {
        if (polygons_.empty()) {
            return true;
        }
        Point minBound = getGlobalMinPoint();
        Point maxBound = getGlobalMaxPoint();
        return poly.isInsideBoundingBox(minBound, maxBound);
    }
    
    void print() const {
        for (const auto& poly : polygons_) {
            std::cout << poly << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: filename not specified" << std::endl;
        return 1;
    }
    
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << argv[1] << std::endl;
        return 1;
    }
    
    PolygonCollection collection;
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        Polygon poly;
        if (parsePolygon(line, poly)) {
            collection.addPolygon(poly);
        }
    }
    
    file.close();
    
    std::string cmdLine;
    while (std::getline(std::cin, cmdLine)) {
        if (cmdLine.empty()) continue;
        
        std::istringstream iss(cmdLine);
        std::string command;
        iss >> command;
        
        try {
            if (command == "AREA") {
                std::string param;
                iss >> param;
                
                if (param == "EVEN") {
                    double sum = collection.getAreaSumByParity(false);
                    std::cout << std::fixed << std::setprecision(1) << sum << std::endl;
                } else if (param == "ODD") {
                    double sum = collection.getAreaSumByParity(true);
                    std::cout << std::fixed << std::setprecision(1) << sum << std::endl;
                } else if (param == "MEAN") {
                    double mean = collection.getMeanArea();
                    std::cout << std::fixed << std::setprecision(1) << mean << std::endl;
                } else {
                    int vertexCount;
                    try {
                        vertexCount = std::stoi(param);
                        double sum = collection.getAreaSumByVertexCount(vertexCount);
                        std::cout << std::fixed << std::setprecision(1) << sum << std::endl;
                    } catch (...) {
                        std::cout << "<INVALID COMMAND>" << std::endl;
                    }
                }
            } else if (command == "MAX") {
                std::string param;
                iss >> param;
                
                if (param == "AREA") {
                    double maxArea = collection.getMaxArea();
                    std::cout << std::fixed << std::setprecision(1) << maxArea << std::endl;
                } else if (param == "VERTEXES") {
                    size_t maxVert = collection.getMaxVertexCount();
                    std::cout << maxVert << std::endl;
                } else {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                }
            } else if (command == "MIN") {
                std::string param;
                iss >> param;
                
                if (param == "AREA") {
                    double minArea = collection.getMinArea();
                    std::cout << std::fixed << std::setprecision(1) << minArea << std::endl;
                } else if (param == "VERTEXES") {
                    size_t minVert = collection.getMinVertexCount();
                    std::cout << minVert << std::endl;
                } else {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                }
            } else if (command == "COUNT") {
                std::string param;
                iss >> param;
                
                if (param == "EVEN") {
                    size_t cnt = collection.countByParity(false);
                    std::cout << cnt << std::endl;
                } else if (param == "ODD") {
                    size_t cnt = collection.countByParity(true);
                    std::cout << cnt << std::endl;
                } else {
                    int vertexCount;
                    try {
                        vertexCount = std::stoi(param);
                        size_t cnt = collection.countByVertexCount(vertexCount);
                        std::cout << cnt << std::endl;
                    } catch (...) {
                        std::cout << "<INVALID COMMAND>" << std::endl;
                    }
                }
            } else if (command == "RMECHO") {
                std::vector<std::string> args;
                std::string arg;
                while (iss >> arg) {
                    args.push_back(arg);
                }
                
                if (args.empty()) {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                    continue;
                }
                
                Polygon target;
                if (!parsePolygonFromArgs(args, 0, target)) {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                    continue;
                }
                
                int removed = collection.removeConsecutiveDuplicates(target);
                std::cout << removed << std::endl;
            } else if (command == "INFRAME") {
                std::vector<std::string> args;
                std::string arg;
                while (iss >> arg) {
                    args.push_back(arg);
                }
                
                if (args.empty()) {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                    continue;
                }
                
                Polygon target;
                if (!parsePolygonFromArgs(args, 0, target)) {
                    std::cout << "<INVALID COMMAND>" << std::endl;
                    continue;
                }
                
                bool result = collection.isInsideGlobalBoundingBox(target);
                std::cout << (result ? "<TRUE>" : "<FALSE>") << std::endl;
            } else {
                std::cout << "<INVALID COMMAND>" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "<INVALID COMMAND>" << std::endl;
        }
    }
    
    return 0;
}
