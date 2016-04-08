#ifndef PAPER_CONSTANTS_HPP
#define PAPER_CONSTANTS_HPP

#include <Paper/BasicTypes.hpp>
#include <Crunch/ExponentialFunc.hpp>
#include <limits>

namespace paper
{
    enum class STICK_API EntityType
    {
        Document,
        Group,
        Path
    };

    enum class STICK_API StrokeCap
    {
        Round,
        Square,
        Butt
    };

    enum class STICK_API StrokeJoin
    {
        Miter,
        Round,
        Bevel
    };

    enum class STICK_API WindingRule
    {
        EvenOdd,
        NonZero
    };

    enum class STICK_API Smoothing
    {
        Continuous,
        Asymmetric,
        CatmullRom,
        Geometric
    };

    namespace detail
    {
        class PaperConstants
        {
        public:

            static Float kappa()
            {
                // Kappa, see: http://www.whizkidtech.redprince.net/bezier/circle/kappa/
                static const Float s_kappa = 4.0 * (crunch::sqrt(2.0) - 1.0) / 3.0;
                return s_kappa;
            }

            static Float tolerance()
            {
                return static_cast<Float>(10e-6);
            }

            static Float epsilon()
            {
                return std::numeric_limits<Float>::epsilon();
            }

            static Float curveTimeEpsilon()
            {
                return static_cast<Float>(4e-7);
            }

            static Float geometricEpsilon()
            {
                return static_cast<Float>(2e-7);
            }

            static Float windingEpsilon()
            {
                return static_cast<Float>(2e-7);
            }

            static Float trigonometricEpsilon()
            {
                return static_cast<Float>(1e-7);
            }

            static Float clippingEpsilon()
            {
                return static_cast<Float>(1e-9);
            }
        };
    }
}

#endif //PAPER_CONSTANTS_HPP
