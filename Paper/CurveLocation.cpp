#include <Paper/CurveLocation.hpp>
#include <Paper/Curve.hpp>

namespace paper
{
    CurveLocation::CurveLocation() :
    m_curve(nullptr)
    {

    }

    CurveLocation::CurveLocation(Curve & _c, Float _parameter, Float _offset) :
        m_curve(&_c),
        m_parameter(_parameter),
        m_offset(_offset)
    {

    }

    CurveLocation::operator bool() const
    {
        return m_curve;
    }

    Vec2f CurveLocation::position() const
    {
        STICK_ASSERT(m_curve);
        return m_curve->positionAtParameter(m_parameter);
    }

    Vec2f CurveLocation::normal() const
    {
        STICK_ASSERT(m_curve);
        return m_curve->normalAtParameter(m_parameter);
    }

    Vec2f CurveLocation::tangent() const
    {
        STICK_ASSERT(m_curve);
        return m_curve->tangentAtParameter(m_parameter);
    }

    Float CurveLocation::curvature() const
    {
        STICK_ASSERT(m_curve);
        return m_curve->curvatureAtParameter(m_parameter);
    }

    Float CurveLocation::angle() const
    {
        STICK_ASSERT(m_curve);
        return m_curve->angleAtParameter(m_parameter);
    }

    Float CurveLocation::parameter() const
    {
        return m_parameter;
    }

    Float CurveLocation::offset() const
    {
        return m_offset;
    }

    bool CurveLocation::isValid() const
    {
        return m_curve;
    }

    const Curve & CurveLocation::curve() const
    {
        STICK_ASSERT(m_curve);
        return *m_curve;
    }
}
