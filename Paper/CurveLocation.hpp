#ifndef PAPER_CURVELOCATION_HPP
#define PAPER_CURVELOCATION_HPP

#include <Paper/BasicTypes.hpp>

namespace paper
{
    class Curve;

    class STICK_API CurveLocation
    {
        friend class Curve;

    public:

        CurveLocation();

        explicit operator bool() const;

        bool operator == (const CurveLocation & _other) const;

        bool operator != (const CurveLocation & _other) const;

        bool isSynonymous(const CurveLocation & _other);

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

    struct STICK_API Intersection
    {
        CurveLocation location;
        Vec2f position;
    };
}

#endif //PAPER_CURVELOCATION_HPP
