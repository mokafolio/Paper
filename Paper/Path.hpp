#ifndef PAPER_PATH_HPP
#define PAPER_PATH_HPP

#include <Paper/Item.hpp>
#include <Stick/Error.hpp>
#include <Paper/Private/ContainerView.hpp>

namespace paper
{
    class CurveLocation;
    struct Intersection;
    
    using IntersectionArray = stick::DynamicArray<Intersection>;

    class STICK_API Path : public Item
    {
        friend class Curve;
        friend class Segment;
        friend class Item;

    public:

        static constexpr EntityType itemType = EntityType::Path;

        using SegmentView = detail::ContainerView<false, SegmentArray, Segment>;
        using SegmentViewConst = detail::ContainerView<true, SegmentArray, Segment>;
        using CurveView = detail::ContainerView<false, CurveArray, Curve>;
        using CurveViewConst = detail::ContainerView<true, CurveArray, Curve>;

        Path();

        //absolute post-script style drawing commands
        void addPoint(const Vec2f & _to);

        void cubicCurveTo(const Vec2f & _handleOne, const Vec2f & _handleTwo, const Vec2f & _to);

        void quadraticCurveTo(const Vec2f & _handle, const Vec2f & _to);

        void curveTo(const Vec2f & _through, const Vec2f & _to, stick::Float32 _parameter = 0.5);

        stick::Error arcTo(const Vec2f & _through, const Vec2f & _to);

        stick::Error arcTo(const Vec2f & _to, bool _bClockwise = true);

        stick::Error arcTo(const Vec2f & _to, const Vec2f & _radii, Float _rotation, bool _bClockwise, bool _bLarge);


        //relative post-script style drawing commands
        void cubicCurveBy(const Vec2f & _handleOne, const Vec2f & _handleTwo, const Vec2f & _by);

        void quadraticCurveBy(const Vec2f & _handle, const Vec2f & _by);

        void curveBy(const Vec2f & _through, const Vec2f & _by, stick::Float32 _parameter = 0.5);

        stick::Error arcBy(const Vec2f & _through, const Vec2f & _by);

        stick::Error arcBy(const Vec2f & _to, bool _bClockwise = true);


        void closePath();

        //TODO: Add the different smoothing versions / algorithms from more recent paper.js versions
        void smooth(Smoothing _type = Smoothing::Asymmetric);

        void smooth(stick::Int64 _from, stick::Int64 _to, Smoothing _type = Smoothing::Asymmetric);

        void simplify(Float _tolerance = 2.5);


        void addSegment(const Vec2f & _point, const Vec2f & _handleIn, const Vec2f & _handleOut);

        Segment & insertSegment(stick::Size _index, const Vec2f & _point,
                                const Vec2f & _handleIn = Vec2f(0.0),
                                const Vec2f & _handleOut = Vec2f(0.0));

        void removeSegment(stick::Size _index);

        void removeSegments(stick::Size _from);

        void removeSegments(stick::Size _from, stick::Size _to);

        void removeSegments();

        SegmentViewConst segments() const;

        SegmentView segments();

        CurveViewConst curves() const;

        CurveView curves();


        Vec2f positionAt(Float _offset) const;

        Vec2f normalAt(Float _offset) const;

        Vec2f tangentAt(Float _offset) const;

        Float curvatureAt(Float _offset) const;

        Float angleAt(Float _offset) const;

        void reverse();

        void setClockwise(bool _b);

        Curve & curve(stick::Size _index);

        const Curve & curve(stick::Size _index) const;

        Segment & segment(stick::Size _index);

        const Segment & segment(stick::Size _index) const;

        stick::Size curveCount() const;

        stick::Size segmentCount() const;

        //NOTE: This function is slightly different from paper.js
        //The paper.js version takes a maxDistance and spaces the resulting
        //segments evenly based on that max distance. This version takes a minDistance
        //which the segments try to keep at least, and spaces the segments non linearly
        //to match the original path as closesly as possible (i.e. linear parts of the path,
        //are subdivided only very little while curvy areas are subdivided a lot)
        void flatten(Float _angleTolerance = 0.25, Float _minDistance = 0.0, stick::Size _maxRecursion = 32);

        void flattenRegular(Float _maxDistance);

        struct OffsetAndSampleCount
        {
            Float offset;
            stick::Size sampleCount;
        };

        OffsetAndSampleCount regularOffsetAndSampleCount(Float _maxDistance);

        Float regularOffset(Float _maxDistance);

        Path splitAt(Float _offset);

        Path splitAt(const CurveLocation & _loc);

        CurveLocation closestCurveLocation(const Vec2f & _point, Float & _outDistance) const;

        CurveLocation closestCurveLocation(const Vec2f & _point) const;

        CurveLocation curveLocationAt(Float _offset) const;

        void peaks(stick::DynamicArray<CurveLocation> & _peaks) const;

        void extrema(stick::DynamicArray<CurveLocation> & _extrema) const;

        stick::DynamicArray<CurveLocation> extrema() const;


        Float length() const;

        Float area() const;


        bool isClosed() const;

        bool isPolygon() const;

        bool isClockwise() const;

        bool contains(const Vec2f & _p) const;

        Path clone() const;

        SegmentArray & segmentArray();

        CurveArray & curveArray();

        const CurveArray & curveArray() const;

        const SegmentArray & segmentArray() const;


        IntersectionArray intersections() const;

        IntersectionArray intersections(Path _other) const;


    private:

        Segment & createSegment(const Vec2f & _pos, const Vec2f & _handleIn, const Vec2f & _handleOut);

        //called from Segment
        void segmentChanged(const Segment & _seg);

        void rebuildCurves();

        void updateSegmentIndices(stick::Size _from, stick::Size _to);

        BoundsResult computeBoundsImpl(Float _padding, const Mat3f * _transform);

        BoundsResult computeBounds(const Mat3f * _transform);

        BoundsResult computeHandleBounds(const Mat3f * _transform);

        BoundsResult computeStrokeBounds(const Mat3f * _transform);

        stick::Error arcHelper(Float _extentDeg, Segment & _segment, const Vec2f & _direction, const Vec2f & _to,
                               const Vec2f & _center, const Mat3f * _transform);

        void applyTransform(const Mat3f & _transform);
    };
}

#endif //PAPER_PATH_HPP
