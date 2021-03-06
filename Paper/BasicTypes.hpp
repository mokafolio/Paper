#ifndef PAPER_BASICTYPES_HPP
#define PAPER_BASICTYPES_HPP

#include <Brick/TypedEntity.hpp>
#include <Stick/DynamicArray.hpp>
#include <Stick/String.hpp>
#include <Stick/UniquePtr.hpp>
#include <Crunch/Vector2.hpp>
#include <Crunch/Line.hpp>
#include <Crunch/Matrix3.hpp>
#include <Crunch/Matrix4.hpp>
#include <Crunch/Rectangle.hpp>
#include <Crunch/Bezier.hpp>
#include <Crunch/Colors.hpp>

namespace paper
{
    using Float = stick::Float32;
    using Vec2f = crunch::Vector2<Float>;
    using Mat3f = crunch::Matrix3<Float>;
    using Mat4f = crunch::Matrix4<Float>;
    using Rect = crunch::Rectangle<Float>;
    using Bezier = crunch::BezierCubic<Vec2f>;
    using Line = crunch::Line<Vec2f>;
    using ColorRGB = crunch::ColorRGB;
    using ColorRGBA = crunch::ColorRGBA;
    using ColorHSB = crunch::ColorHSB;
    using ColorHSBA = crunch::ColorHSBA;
    class Segment;
    class Curve;
    using SegmentArray = stick::DynamicArray<stick::UniquePtr<Segment>>;
    using CurveArray = stick::DynamicArray<stick::UniquePtr<Curve>>;
    using DashArray = stick::DynamicArray<Float>;
}

#endif //PAPER_BASICTYPES_HPP
