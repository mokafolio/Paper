#ifndef PAPER_CONSTANTS_HPP
#define PAPER_CONSTANTS_HPP

#include <Paper/BasicTypes.hpp>
#include <Crunch/ExponentialFunc.hpp>
#include <limits>

namespace paper
{
    STICK_API_ENUM_CLASS(EntityType)
    {
        Document,
        Group,
        Path,
        PlacedSymbol,
        Unknown
    };

    STICK_API_ENUM_CLASS(StrokeCap)
    {
        Round,
        Square,
        Butt
    };

    STICK_API_ENUM_CLASS(StrokeJoin)
    {
        Miter,
        Round,
        Bevel
    };

    STICK_API_ENUM_CLASS(WindingRule)
    {
        EvenOdd,
        NonZero
    };

    STICK_API_ENUM_CLASS(Smoothing)
    {
        Continuous,
        Asymmetric,
        CatmullRom,
        Geometric
    };

    STICK_API_ENUM_CLASS(PaintType)
    {
        None,
        Color,
        LinearGradient,
        CircularGradient
    };

    namespace detail
    {
        //@TODO: Adjust these for Float32 / Float64
        //@TODO: These need a lot more work, checking and tests :(
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
                return static_cast<Float>(1e-4);
            }

            static Float epsilon()
            {
                return std::numeric_limits<Float>::epsilon();
            }

            static Float curveTimeEpsilon()
            {
                return static_cast<Float>(4e-4);
            }

            static Float geometricEpsilon()
            {
                return static_cast<Float>(2e-4);
            }

            static Float windingEpsilon()
            {
                return static_cast<Float>(2e-4);
            }

            static Float trigonometricEpsilon()
            {
                return static_cast<Float>(1e-5);
            }

            static Float clippingEpsilon()
            {
                return static_cast<Float>(1e-7);
            }
        };
    }
}

#endif //PAPER_CONSTANTS_HPP
