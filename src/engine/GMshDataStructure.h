#ifndef GMSH_DATA_STRUCTURE_H
#define GMSH_DATA_STRUCTURE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <unordered_set>

// GObject 기본 클래스
class GObject {
public:
    int id;

    GObject(int id) : id(id) {}
    virtual ~GObject() = default;

    virtual void print() const = 0; // 순수 가상 함수
};

// GPoint 클래스
class GPoint : public GObject {
public:
    double x, y, z;
    double size; // 로컬 메쉬 크기

    GPoint(int id, double x, double y, double z, double size = 1.0)
        : GObject(id), x(x), y(y), z(z), size(size) {
    }

    bool operator==(const GPoint& other) const {
        return x == other.x &&
            y == other.y &&
            z == other.z &&
            size == other.size;
    }

    void print() const override {
        std::cout << "GPoint(ID: " << id << ", Coordinates: (" << x << ", " << y << ", " << z
            << "), Size: " << size << ")" << std::endl;
    }
};

// GCurve 클래스
class GCurve : public GObject {
public:
    enum class CurveType {
        Line,
        Circle,
        Ellipse,
        Spline,
        BSpline,
        Bezier,
        Polyline
    };

    CurveType type;
    std::vector<int> point_ids;

    GCurve(int id, CurveType type, const std::vector<int>& point_ids)
        : GObject(id), type(type), point_ids(point_ids) {
    }

    bool operator==(const GCurve& other) const {
        if (type != other.type) return false;
        if (point_ids.size() != other.point_ids.size()) return false;
        return point_ids == other.point_ids;
    }

    void reverse() {
        std::reverse(point_ids.begin(), point_ids.end());
    }

    std::string getTypeName() const {
        switch (type) {
        case CurveType::Line: return "Line";
        case CurveType::Circle: return "Circle";
        case CurveType::Ellipse: return "Ellipse";
        case CurveType::Spline: return "Spline";
        case CurveType::BSpline: return "BSpline";
        case CurveType::Bezier: return "Bezier";
        case CurveType::Polyline: return "Polyline";
        default: return "Unknown";
        }
    }

    void print() const override {
        std::cout << "GCurve(ID: " << id << ", Type: " << getTypeName() << ", Points: [";
        for (size_t i = 0; i < point_ids.size(); ++i) {
            std::cout << point_ids[i];
            if (i != point_ids.size() - 1) std::cout << ", ";
        }
        std::cout << "])" << std::endl;
    }
};

// GLoop 클래스
class GLoop : public GObject {
public:
    std::vector<int> curve_ids;

    GLoop(int id, const std::vector<int>& curve_ids)
        : GObject(id), curve_ids(curve_ids) {
    }

    void reverse() {
        std::reverse(curve_ids.begin(), curve_ids.end());
    }

    bool isCircularEqual(const std::vector<int>& a, const std::vector<int>& b) const {
        if (a.size() != b.size()) return false;
        std::vector<int> doubled_a = a;
        doubled_a.insert(doubled_a.end(), a.begin(), a.end());
        return std::search(doubled_a.begin(), doubled_a.end(), b.begin(), b.end()) != doubled_a.end();
    }

    bool operator==(const GLoop& other) const {
        return isCircularEqual(curve_ids, other.curve_ids);
    }

    void print() const override {
        std::cout << "GLoop(ID: " << id << ", Curves: [";
        for (size_t i = 0; i < curve_ids.size(); ++i) {
            std::cout << curve_ids[i];
            if (i != curve_ids.size() - 1) std::cout << ", ";
        }
        std::cout << "])" << std::endl;
    }
};

// GSurface 클래스
class GSurface : public GObject {
public:
    int outer_loop_id;
    std::vector<int> inner_loop_ids;

    GSurface() : GObject(-1) {};
    GSurface(int id, int outer_loop_id, const std::vector<int>& inner_loop_ids = {})
        : GObject(id), outer_loop_id(outer_loop_id), inner_loop_ids(inner_loop_ids) {
    }

    bool isSetEqual(const std::vector<int>& a, const std::vector<int>& b) const {
        return std::unordered_set<int>(a.begin(), a.end()) == std::unordered_set<int>(b.begin(), b.end());
    }

    bool operator==(const GSurface& other) const {
        if (outer_loop_id != other.outer_loop_id) return false;
        return isSetEqual(inner_loop_ids, other.inner_loop_ids);
    }

    void print() const override {
        std::cout << "GSurface(ID: " << id << ", Outer Loop: " << outer_loop_id
            << ", Inner Loops: [";
        for (size_t i = 0; i < inner_loop_ids.size(); ++i) {
            std::cout << inner_loop_ids[i];
            if (i != inner_loop_ids.size() - 1) std::cout << ", ";
        }
        std::cout << "])" << std::endl;
    }
};

#endif // GMSH_DATA_STRUCTURE_H
