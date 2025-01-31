#ifndef BZMAG_ENGINE_GEOMETRICENTITY_H
#define BZMAG_ENGINE_GEOMETRICENTITY_H

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/General_polygon_2.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/Circular_kernel_2.h>
#include <CGAL/Algebraic_kernel_for_circles_2_2.h>

namespace bzmag
{
    namespace engine
    {
        using L = CGAL::Exact_predicates_exact_constructions_kernel;
        using LK = CGAL::Exact_predicates_inexact_constructions_kernel;
        using A = CGAL::Algebraic_kernel_for_circles_2_2<L::FT>;
        using CircK = CGAL::Circular_kernel_2<L, A>;

        typedef CGAL::Aff_transformation_2<L>            Transformation;
        typedef CGAL::Vector_2<L>                        Vector_2;
        typedef CGAL::Gps_circle_segment_traits_2<L>     Traits_2;
        typedef CGAL::General_polygon_set_2<Traits_2>    Polygon_set_2;
        typedef CGAL::Arrangement_2<Traits_2>            Arrangement;

        typedef L::FT                                    NT;
        typedef L::Point_2                               Point_2;
        typedef L::Line_2                                Line_2;
        typedef L::Circle_2                              Circle_2;
        typedef L::Iso_rectangle_2                       Rectangle_2;

        typedef Traits_2::General_polygon_2              Polygon_2;
        typedef Traits_2::General_polygon_with_holes_2   Polygon_with_holes_2;
        typedef Traits_2::Curve_2                        Curve_2;
        typedef Traits_2::X_monotone_curve_2             X_monotone_curve_2;
        typedef Traits_2::CoordNT                        CoordNT;
    }
}

#endif //BZMAG_ENGINE_GEOMETRICENTITY_H