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

    bool CurveLocation::operator == (const CurveLocation & _other) const
    {
        return m_curve == _other.m_curve && m_parameter == _other.m_parameter;
    }

    bool CurveLocation::operator != (const CurveLocation & _other) const
    {
        return !(*this == _other);
    }

    bool CurveLocation::isSynonymous(const CurveLocation & _other)
    {
        if (isValid() && _other.isValid())
        {
            if (_other.curve().path() == curve().path())
            {
                printf("SAME PATH\n");
                Float diff = std::abs(m_offset - _other.m_offset);
                if (diff < detail::PaperConstants::geometricEpsilon() ||
                        std::abs(curve().path().length() - diff) < detail::PaperConstants::geometricEpsilon())
                {
                    printf("SYNOONONASIOGHAKSH\n");
                    return true;
                }
            }
        }
        return false;
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
