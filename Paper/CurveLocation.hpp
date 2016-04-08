#ifndef PAPER_CURVELOCATION_HPP
#define PAPER_CURVELOCATION_HPP

#include <Paper/BasicTypes.hpp>

namespace paper
{
    class Curve;

    class CurveLocation
    {
        friend class Curve;

    public:

        CurveLocation();

        explicit operator bool() const;

        Vec2f position() const;

        Vec2f normal() const;

        Vec2f tangent() const;

        Float curvature() const;

        Float angle() const;

        Float parameter() const;

        Float offset() const;

        bool isValid() const;

        const Curve & curve() const;

    private:

        CurveLocation(Curve & _c, Float _parameter, Float _offset);

        Curve * m_curve;
        Float m_parameter;
        Float m_offset;
    };
}

#endif //PAPER_CURVELOCATION_HPP
