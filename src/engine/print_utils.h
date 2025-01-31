#ifndef CGAL_PRINT_UTILS_H
#define CGAL_PRINT_UTILS_H

#include <iostream>
#include <CGAL/Polygon_with_holes_2.h>


//-----------------------------------------------------------------------------
// Pretty-print a CGAL polygon.
//
template<class Kernel, class Container>
void print_polygon(const CGAL::Polygon_2<Kernel, Container>& P)
{
    typename CGAL::Polygon_2<Kernel, Container>::Vertex_const_iterator  vit;

    std::cout << "[ " << P.size() << " vertices:";
    for (vit = P.vertices_begin(); vit != P.vertices_end(); ++vit)
        std::cout << " (" << *vit << ')';
    std::cout << " ]" << std::endl;

    return;
}

//-----------------------------------------------------------------------------
// Pretty-print a polygon with holes.
//
template<class Kernel, class Container>
void print_polygon_with_holes
(const CGAL::Polygon_with_holes_2<Kernel, Container>& pwh)
{
    if (!pwh.is_unbounded())
    {
        std::cout << "{ Outer boundary = ";
        print_polygon(pwh.outer_boundary());
    }
    else
        std::cout << "{ Unbounded polygon." << std::endl;

    typename CGAL::Polygon_with_holes_2<Kernel, Container>::
        Hole_const_iterator  hit;
    unsigned int                                                     k = 1;

    std::cout << "  " << pwh.number_of_holes() << " holes:" << std::endl;
    for (hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit, ++k)
    {
        std::cout << "    Hole #" << k << " = ";
        print_polygon(*hit);
    }
    std::cout << " }" << std::endl;

    return;
}

template<typename GeneralPolygon>
void print_general_polygon(const GeneralPolygon& polygon) {
    using CurveIterator = typename GeneralPolygon::Curve_const_iterator;

    std::cout << "[ " << polygon.size() << " curves:" << std::endl;

    for (CurveIterator cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
        // 각 커브의 정보 출력
        std::cout << "  Curve: (";
        std::cout << cit->source() << " -> " << cit->target() << ")";

        if (cit->is_circular()) {
            std::cout << " [circular]";
            if (cit->orientation() == CGAL::COUNTERCLOCKWISE) {
                std::cout << " CCW";
            }
            else {
                std::cout << " CW";
            }
        }
        else {
            std::cout << " [linear]";
        }
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

template<typename GeneralPolygonWithHoles>
void print_general_polygon_with_holes(const GeneralPolygonWithHoles& polygon_with_holes) {
    // 외곽 다각형 출력
    if (!polygon_with_holes.is_unbounded()) {
        std::cout << "Outer boundary:" << std::endl;
        print_general_polygon(polygon_with_holes.outer_boundary());
    }
    else {
        std::cout << "Outer boundary is unbounded.\n";
    }

    // 구멍(holes) 출력
    typename GeneralPolygonWithHoles::Hole_const_iterator hole_it;
    std::cout << polygon_with_holes.number_of_holes() << " holes:" << std::endl;
    for (hole_it = polygon_with_holes.holes_begin(); hole_it != polygon_with_holes.holes_end(); ++hole_it) {
        std::cout << "  Hole:" << std::endl;
        print_general_polygon(*hole_it);
    }
}

#endif
