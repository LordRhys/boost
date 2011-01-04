// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_FOR_EACH_HPP
#define BOOST_GEOMETRY_TEST_FOR_EACH_HPP

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/for_each.hpp>

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>
#include <boost/geometry/util/write_dsv.hpp>


template<typename Point>
inline void translate_x_function(Point& p)
{
    bg::set<0>(p, bg::get<0>(p) + 100.0);
}

template<typename Point>
struct scale_y_functor
{
    inline void operator()(Point& p)
    {
        bg::set<1>(p, bg::get<1>(p) * 100.0);
    }
};

template<typename Point>
struct sum_x_functor
{
    int sum;

    sum_x_functor()
        : sum(0)
    {}

    inline void operator()(Point const& p)
    {
        sum += int(bg::get<0>(p));
    }
};

// Per segment
static std::ostringstream g_out;

template<typename Segment>
inline void stream_segment(Segment const& s)
{
    g_out << bg::dsv(s) << " ";
}

template<typename Segment>
struct sum_segment_length
{
    double sum;

    sum_segment_length()
        : sum(0)
    {}
    inline void operator()(Segment const& s)
    {
        sum += bg::distance(s.first, s.second);
    }
};

template<typename Segment>
inline void modify_segment(Segment& s)
{
    if (bg::math::equals(bg::get<0,0>(s), 1.0))
    {
        bg::set<0,0>(s, 10.0);
    }
}


template <typename Geometry>
void test_per_point_const(Geometry const& geometry, int expected)
{
    typedef typename bg::point_type<Geometry>::type point_type;

    sum_x_functor<point_type> functor;
    functor = bg::for_each_point(geometry, functor);
    BOOST_CHECK_EQUAL(functor.sum, expected);
}

template <typename Geometry>
void test_per_point_non_const(Geometry& geometry,
    std::string const& expected1,
    std::string const& expected2)
{
    typedef typename bg::point_type<Geometry>::type point_type;

    // function
    bg::for_each_point(geometry, translate_x_function<point_type>);
    std::ostringstream out1;
    out1 << bg::wkt(geometry);

    BOOST_CHECK_MESSAGE(out1.str() == expected1,
        "for_each_point: "
        << " expected " << expected1
        << " got " << bg::wkt(geometry));

    // functor
    bg::for_each_point(geometry, scale_y_functor<point_type>());

    std::ostringstream out2;
    out2 << bg::wkt(geometry);

    BOOST_CHECK_MESSAGE(out2.str() == expected2,
        "for_each_point: "
        << " expected " << expected2
        << " got " << bg::wkt(geometry));
}


template <typename Geometry>
void test_per_point(std::string const& wkt
    , int expected_sum_x
    , std::string const& expected1
    , std::string const& expected2
    )
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    test_per_point_const(geometry, expected_sum_x);
    test_per_point_non_const(geometry, expected1, expected2);
}



template <typename Geometry>
void test_per_segment_const(Geometry const& geometry,
        std::string const& expected_dsv,
        double expected_length)
{
    typedef typename bg::point_type<Geometry>::type point_type;

    // function
    g_out.str("");
    g_out.clear();
    bg::for_each_segment(geometry,
            stream_segment<bg::model::referring_segment<point_type const> >);
    std::string out = g_out.str();
    boost::trim(out);
    BOOST_CHECK_EQUAL(out, expected_dsv);

    // functor
    sum_segment_length<bg::model::referring_segment<point_type const> > functor;
    functor = bg::for_each_segment(geometry, functor);

    BOOST_CHECK_EQUAL(functor.sum, expected_length);
}


template <typename Geometry>
void test_per_segment_non_const(Geometry& geometry,
        std::string const& expected_wkt)
{
    typedef typename bg::point_type<Geometry>::type point_type;

    // function
    bg::for_each_segment(geometry,
            modify_segment<bg::model::referring_segment<point_type> >);

    std::ostringstream out;
    out << bg::wkt(geometry);

    BOOST_CHECK_MESSAGE(out.str() == expected_wkt,
        "for_each_segment: "
        << " expected " << expected_wkt
        << " got " << bg::wkt(geometry));

    // function is working here, functor works for all others,
    // it will also work here.
}


template <typename Geometry>
void test_per_segment(std::string const& wkt
        , std::string const& expected_dsv
        , double expected_length
        , std::string const& expected_wkt
        )
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    test_per_segment_const(geometry, expected_dsv, expected_length);
    test_per_segment_non_const(geometry, expected_wkt);
}



template <typename Geometry>
void test_geometry(std::string const& wkt
    , int expected_sum_x
    , std::string const& expected1
    , std::string const& expected2
    , std::string const& expected_dsv
    , double expected_length
    , std::string const& expected_wkt
    )
{
    test_per_point<Geometry>(wkt, expected_sum_x, expected1, expected2);
    test_per_segment<Geometry>(wkt, expected_dsv, expected_length, expected_wkt);
}


#endif
