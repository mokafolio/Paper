#include <Paper/Document.hpp>
#include <Paper/CurveLocation.hpp>

#include <Crunch/StringConversion.hpp>

namespace paper
{
    Curve::Curve()
    {

    }

    Curve::Curve(const Path & _path, stick::Size _segmentA, stick::Size _segmentB) :
        m_path(_path),
        m_segmentA(_segmentA),
        m_segmentB(_segmentB),
        m_bLengthCached(false),
        m_bBoundsCached(false),
        m_length(0)
    {
        m_curve = Bezier(positionOne(), handleOneAbsolute(), handleTwoAbsolute(), positionTwo());
    }

    Path Curve::path() const
    {
        return m_path;
    }

    void Curve::setPositionOne(const Vec2f & _vec)
    {
        segmentOne().setPosition(_vec);
    }

    void Curve::setHandleOne(const Vec2f & _vec)
    {
        segmentOne().setHandleOut(_vec);
    }

    void Curve::setPositionTwo(const Vec2f & _vec)
    {
        segmentTwo().setPosition(_vec);
    }

    void Curve::setHandleTwo(const Vec2f & _vec)
    {
        segmentTwo().setHandleIn(_vec);
    }

    Segment & Curve::segmentOne()
    {
        return *m_path.segmentArray()[m_segmentA];
    }

    const Segment & Curve::segmentOne() const
    {
        return *m_path.segmentArray()[m_segmentA];
    }

    Segment & Curve::segmentTwo()
    {
        return *m_path.segmentArray()[m_segmentB];
    }

    const Segment & Curve::segmentTwo() const
    {
        return *m_path.segmentArray()[m_segmentB];
    }

    const Vec2f & Curve::positionOne() const
    {
        return segmentOne().position();
    }

    const Vec2f & Curve::positionTwo() const
    {
        return segmentTwo().position();
    }

    const Vec2f & Curve::handleOne() const
    {
        return segmentOne().handleOut();
    }

    Vec2f Curve::handleOneAbsolute() const
    {
        return segmentOne().handleOutAbsolute();
    }

    const Vec2f & Curve::handleTwo() const
    {
        return segmentTwo().handleIn();
    }

    Vec2f Curve::handleTwoAbsolute() const
    {
        return segmentTwo().handleInAbsolute();
    }

    Vec2f Curve::positionAt(Float _offset) const
    {
        return m_curve.positionAt(parameterAtOffset(_offset));
    }

    Vec2f Curve::normalAt(Float _offset) const
    {
        return m_curve.normalAt(parameterAtOffset(_offset));
    }

    Vec2f Curve::tangentAt(Float _offset) const
    {
        return m_curve.tangentAt(parameterAtOffset(_offset));
    }

    Float Curve::curvatureAt(Float _offset) const
    {
        return m_curve.curvatureAt(parameterAtOffset(_offset));
    }

    Float Curve::angleAt(Float _offset) const
    {
        return m_curve.angleAt(parameterAtOffset(_offset));
    }

    stick::Maybe<Curve &> Curve::divideAt(Float _offset)
    {
        return divideAtParameter(parameterAtOffset(_offset));
    }

    Vec2f Curve::positionAtParameter(Float _t) const
    {
        return m_curve.positionAt(_t);
    }

    Vec2f Curve::normalAtParameter(Float _t) const
    {
        return m_curve.normalAt(_t);
    }

    Vec2f Curve::tangentAtParameter(Float _t) const
    {
        return m_curve.tangentAt(_t);
    }

    Float Curve::curvatureAtParameter(Float _t) const
    {
        return m_curve.curvatureAt(_t);
    }

    Float Curve::angleAtParameter(Float _t) const
    {
        return m_curve.angleAt(_t);
    }

    stick::Maybe<Curve &> Curve::divideAtParameter(Float _t)
    {
        // return empty maybe if _t is out of range
        if (_t >= 1 || _t <= 0)
            return stick::Maybe<Curve &>();

        // split the bezier
        auto splitResult = m_curve.subdivide(_t);

        //adjust the exiting segments of this curve
        segmentOne().m_handleOut = splitResult.first.handleOne() - splitResult.first.positionOne();
        segmentTwo().m_handleIn = splitResult.second.handleTwo() - splitResult.second.positionTwo();
        segmentTwo().m_position = splitResult.second.positionTwo();

        // create the new segment
        stick::Size sindex = segmentOne().m_index + 1;
        auto it = m_segmentB != 0 ? m_path.segmentArray().begin() + m_segmentB : m_path.segmentArray().end();
        m_path.segmentArray().insert(it,
                                     stick::UniquePtr<Segment>(m_path.document().allocator().create<Segment>(m_path, splitResult.first.positionTwo(),
                                             splitResult.first.handleTwo() - splitResult.first.positionTwo(),
                                             splitResult.second.handleOne() - splitResult.second.positionOne(), sindex)));


        // adjust the segment indices
        auto & segs = m_path.get<comps::Segments>();
        for (stick::Size i = sindex + 1; i < segs.count(); ++i)
        {
            segs[i]->m_index += 1;
        }

        // create the new curve in the correct index
        auto cit = sindex < m_path.curveArray().count() ? m_path.curveArray().begin() + sindex : m_path.curveArray().end();
        stick::Size sindex2 = cit == m_path.curveArray().end() ? 0 : sindex + 1;
        Curve * c = m_path.document().allocator().create<Curve>(m_path, sindex, sindex2);
        auto rit = m_path.curveArray().insert(cit, stick::UniquePtr<Curve>(c));

        // update curve segment indices
        auto & curves = m_path.curveArray();
        if (sindex2 != 0)
        {
            for (stick::Size i = m_segmentB + 1; i < curves.count(); ++i)
            {
                curves[i]->m_segmentA += 1;
                if (curves[i]->m_segmentB != 0 || i < curves.count() - 1)
                    curves[i]->m_segmentB = curves[i]->m_segmentA + 1;
            }
        }
        else
        {
            //only update the previous closing curves indices
            curves[curves.count() - 2]->m_segmentB = sindex;
        }

        //mark the affected curves dirty
        m_segmentB = sindex;
        markDirty();
        c->markDirty();

        // return the new curve
        return *c;
    }

    Float Curve::parameterAtOffset(Float _offset) const
    {
        return m_curve.parameterAtOffset(_offset);
    }

    Float Curve::closestParameter(const Vec2f & _point) const
    {
        return m_curve.closestParameter(_point);
    }

    Float Curve::closestParameter(const Vec2f & _point, Float & _outDistance) const
    {
        return m_curve.closestParameter(_point, _outDistance, 0, 1, 0);
    }

    Float Curve::lengthBetween(Float _tStart, Float _tEnd) const
    {
        return m_curve.lengthBetween(_tStart, _tEnd);
    }

    Float Curve::pathOffset() const
    {
        //calculate the offset from the start to the curve location
        Float offset = 0;
        for (const auto & c : m_path.curveArray())
        {
            if (c.get() != this)
                offset += c->length();
            else
                break;
        }

        return offset;
    }

    CurveLocation Curve::closestCurveLocation(const Vec2f & _point) const
    {
        Float t = closestParameter(_point);
        return CurveLocation(const_cast<Curve &>(*this), t, pathOffset() + lengthBetween(0, t));
    }

    CurveLocation Curve::curveLocationAt(Float _offset) const
    {
        return CurveLocation(const_cast<Curve &>(*this), parameterAtOffset(_offset), pathOffset() + _offset);
    }

    CurveLocation Curve::curveLocationAtParameter(Float _t) const
    {
        return CurveLocation(const_cast<Curve &>(*this), _t, pathOffset() + lengthBetween(0, _t));
    }

    bool Curve::isLinear() const
    {
        return crunch::isClose(handleOne(), Vec2f(0)) && crunch::isClose(handleTwo(), Vec2f(0));
    }

    bool Curve::isStraight() const
    {
        if (isLinear())
            return true;

        Vec2f line = positionTwo() - positionOne();

        // Zero-length line, with some handles defined.
        if (crunch::isClose(line, Vec2f(0)))
            return false;

        if (crunch::isColinear(handleOne(), line) && crunch::isColinear(handleTwo(), line))
        {
            // Collinear handles. Project them onto line to see if they are
            // within the line's range:
            Float d = crunch::dot(line, line);
            Float p1 = crunch::dot(line, handleOne()) / d;
            Float p2 = crunch::dot(line, handleTwo()) / d;

            return p1 >= 0 && p1 <= 1 && p2 <= 0 && p2 >= -1;
        }

        return false;
    }

    bool Curve::isArc() const
    {
        if (crunch::isOrthogonal(handleOne(), handleTwo(), detail::PaperConstants::tolerance()))
        {
            Line aLine(positionOne(), handleOneAbsolute());
            Line bLine(positionTwo(), handleTwoAbsolute());

            crunch::IntersectionResult<Vec2f> result = crunch::intersect(aLine, bLine);
            if (!result.intersections().isEmpty())
            {
                Vec2f corner = result.intersections()[0];
                static Float kappa = detail::PaperConstants::kappa();
                static Float epsilon = detail::PaperConstants::epsilon();

                if (crunch::isClose(crunch::length(handleOne()) / crunch::length(corner - positionOne()) - kappa,
                                    0.0f, epsilon) &&
                        crunch::isClose(crunch::length(handleTwo()) / crunch::length(corner - positionTwo()) - kappa,
                                        0.0f, epsilon))
                    return true;
            }
        }
        return false;
    }

    bool Curve::isOrthogonal(const Curve & _other) const
    {
        if (isLinear() && _other.isLinear())
        {
            if (crunch::isOrthogonal(positionOne() - positionTwo(), _other.positionOne() - _other.positionTwo(), detail::PaperConstants::tolerance()))
                return true;
        }
        return false;
    }

    bool Curve::isColinear(const Curve & _other) const
    {
        if (isLinear() && _other.isLinear())
        {
            if (crunch::isColinear(positionOne() - positionTwo(), _other.positionOne() - _other.positionTwo(), detail::PaperConstants::tolerance()))
                return true;
        }
        return false;
    }

    Float Curve::length() const
    {
        if (!m_bLengthCached)
        {
            m_length = m_curve.length();
            m_bLengthCached = true;
        }
        return m_length;
    }

    Float Curve::area() const
    {
        return m_curve.area();
    }

    const Rect & Curve::bounds() const
    {
        if (!m_bBoundsCached)
        {
            m_bounds = m_curve.bounds();
            m_bBoundsCached = true;
        }
        return m_bounds;
    }

    Rect Curve::bounds(Float _padding) const
    {
        return m_curve.bounds(_padding);
    }

    void Curve::markDirty()
    {
        m_bLengthCached = false;
        m_bBoundsCached = false;
        m_curve = Bezier(positionOne(), handleOneAbsolute(), handleTwoAbsolute(), positionTwo());
    }

    const Bezier & Curve::bezier() const
    {
        return m_curve;
    }

    void Curve::peaks(stick::DynamicArray<Float> & _peaks) const
    {
        m_curve.peaks(_peaks);
    }
}
