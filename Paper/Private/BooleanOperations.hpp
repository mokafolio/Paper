#ifndef PAPER_PRIVATE_BOOLEANOPERATIONS_HPP
#define PAPER_PRIVATE_BOOLEANOPERATIONS_HPP

#include <Paper/Components.hpp>

namespace paper
{
    class Path;

    namespace detail
    {
        struct STICK_LOCAL MonoCurve
        {
            Bezier bezier;
            stick::Int32 winding;
        };

        using MonoCurveArray = stick::DynamicArray<MonoCurve>;

        struct STICK_LOCAL MonoCurveLoop
        {
            Mat3f inverseTransform;
            bool bTransformed;
            MonoCurveArray monoCurves;
            MonoCurve last;
        };

        using MonoCurveLoopArray = stick::DynamicArray<MonoCurveLoop>;

        namespace comps
        {
            using MonoCurves = brick::Component<ComponentName("MonoCurves"), MonoCurveLoopArray>;
        }

        STICK_LOCAL const MonoCurveLoopArray & monoCurves(Path & _path);

        STICK_LOCAL stick::Int32 winding(const Vec2f & _point, const MonoCurveLoopArray & _loops, bool _bHorizontal);
    }
}

#endif //PAPER_PRIVATE_BOOLEANOPERATIONS_HPP
