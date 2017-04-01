#ifndef PAPER_CURVE_HPP
#define PAPER_CURVE_HPP

#include <Paper/Path.hpp>

namespace paper
{
    class CurveLocation;
    class Segment;

    class Curve
    {
        friend class Path;
        friend class Segment;

    public:

        Curve();

        Curve(const Path & _path, stick::Size _segmentA, stick::Size _segmentB);

        Path path() const;

        void setPositionOne(const Vec2f & _vec);

        void setHandleOne(const Vec2f & _vec);

        void setPositionTwo(const Vec2f & _vec);

        void setHandleTwo(const Vec2f & _vec);

        const Vec2f & positionOne() const;

        const Vec2f & positionTwo() const;

        const Vec2f & handleOne() const;

        Vec2f handleOneAbsolute() const;

        const Vec2f & handleTwo() const;

        Vec2f handleTwoAbsolute() const;

        Segment & segmentOne();

        const Segment & segmentOne() const;

        Segment & segmentTwo();

        const Segment & segmentTwo() const;


        Vec2f positionAt(Float _offset) const;

        Vec2f normalAt(Float _offset) const;

        Vec2f tangentAt(Float _offset) const;

        Float curvatureAt(Float _offset) const;

        Float angleAt(Float _offset) const;

        stick::Maybe<Curve&> divideAt(Float _offset);

        Vec2f positionAtParameter(Float _t) const;

        Vec2f normalAtParameter(Float _t) const;

        Vec2f tangentAtParameter(Float _t) const;

        Float curvatureAtParameter(Float _t) const;

        Float angleAtParameter(Float _t) const;

        stick::Maybe<Curve&> divideAtParameter(Float _t);

        Float parameterAtOffset(Float _offset) const;

        Float closestParameter(const Vec2f & _point) const;

        Float closestParameter(const Vec2f & _point, Float & _outDistance) const;

        Float lengthBetween(Float _tStart, Float _tEnd) const;

        Float pathOffset() const;

        void peaks(stick::DynamicArray<Float> & _peaks) const;

        void extrema(stick::DynamicArray<Float> & _extrema) const;


        CurveLocation closestCurveLocation(const Vec2f & _point) const;

        CurveLocation curveLocationAt(Float _offset) const;

        CurveLocation curveLocationAtParameter(Float _t) const;


        bool isLinear() const;

        bool isStraight() const;

        bool isArc() const;

        bool isOrthogonal(const Curve & _other) const;

        bool isCollinear(const Curve & _other) const;

        Float length() const;

        Float area() const;

        const Rect & bounds() const;

        Rect bounds(Float _padding) const;

        const Bezier & bezier() const;


    private:

        void markDirty();


        Path m_path;
        stick::Size m_segmentA;
        stick::Size m_segmentB;
        Bezier m_curve;
        mutable bool m_bLengthCached;
        mutable bool m_bBoundsCached;
        mutable Float m_length;
        mutable Rect m_bounds;
    };
}

#endif //PAPER_CURVE_HPP
