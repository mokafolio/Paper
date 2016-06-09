#ifndef PAPER_PATH_HPP
#define PAPER_PATH_HPP

#include <Paper/Item.hpp>
#include <Stick/Error.hpp>

namespace paper
{
    class CurveLocation;

    class Path : public Item
    {
        friend class Curve;
        friend class Segment;
        friend class Item;

    public:

        using SegmentIter = typename SegmentArray::Iter;
        using SegmentConstIter = typename SegmentArray::ConstIter;
        using CurveIter = typename CurveArray::Iter;
        using CurveConstIter = typename CurveArray::ConstIter;


        Path();

        Path(const brick::Entity & _e);

        //absolute post-script style drawing commands
        void addPoint(const Vec2f & _to);

        void cubicCurveTo(const Vec2f & _handleOne, const Vec2f & _handleTwo, const Vec2f & _to);

        void quadraticCurveTo(const Vec2f & _handle, const Vec2f & _to);

        void curveTo(const Vec2f & _through, const Vec2f & _to, stick::Float32 _parameter = 0.5);

        stick::Error arcTo(const Vec2f & _through, const Vec2f & _to);

        stick::Error arcTo(const Vec2f & _to, bool _bClockwise = true);


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

        void removeSegment(stick::Size _index);

        void removeSegments(stick::Size _from);

        void removeSegments(stick::Size _from, stick::Size _to);

        void removeSegments();

        const SegmentArray & segments() const;

        SegmentArray & segments();

        const CurveArray & curves() const;

        CurveArray & curves();


        Vec2f positionAt(Float _offset) const;

        Vec2f normalAt(Float _offset) const;

        Vec2f tangentAt(Float _offset) const;

        Float curvatureAt(Float _offset) const;

        Float angleAt(Float _offset) const;

        void reverse();

        void setClockwise(bool _b);

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


        Float length() const;

        Float area() const;


        bool isClosed() const;

        bool isPolygon() const;

        bool isClockwise() const;

        bool contains(const Vec2f & _p) const;

        Path clone() const;


    private:

        Segment & createSegment(const Vec2f & _pos, const Vec2f & _handleIn, const Vec2f & _handleOut);

        //called from Segment
        void segmentChanged(const Segment & _seg);

        SegmentArray & segmentArray();

        CurveArray & curveArray();

        const CurveArray & curveArray() const;

        const SegmentArray & segmentArray() const;

        void rebuildCurves();

        void updateSegmentIndices(stick::Size _from, stick::Size _to);

        BoundsResult computeBoundsImpl(Float _padding, const Mat3f * _transform);

        BoundsResult computeBounds(const Mat3f * _transform);

        BoundsResult computeHandleBounds(const Mat3f * _transform);

        BoundsResult computeStrokeBounds(const Mat3f * _transform);

        void applyTransform(const Mat3f & _transform);
    };
}

#endif //PAPER_PATH_HPP
