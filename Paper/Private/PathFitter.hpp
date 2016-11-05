#ifndef PAPER_PRIVATE_PATHFITTER_HPP
#define PAPER_PRIVATE_PATHFITTER_HPP

#include <Paper/Path.hpp>

namespace paper
{
    class Path;

    namespace detail
    {
        // From paper.js source
        // An Algorithm for Automatically Fitting Digitized Curves
        // by Philip J. Schneider
        // from "Graphics Gems", Academic Press, 1990
        // Modifications and optimisations of original algorithm by Juerg Lehni.

        class PathFitter
        {
        public:

            struct MaxError
            {
                Float value;
                stick::Size index;
            };

            struct SegmentDesc
            {
                Vec2f position;
                Vec2f handleIn;
                Vec2f handleOut;
            };

            using SegmentDescArray = stick::DynamicArray<SegmentDesc>;
            using PositionArray = stick::DynamicArray<Vec2f>;

            PathFitter(const Path & _p, Float _error);

            void fit();

            void fitCubic(stick::Size _first, stick::Size _last, const Vec2f & _tan1, const Vec2f & _tan2);

            void addCurve(const Vec2f & _pointOne, const Vec2f & _handleOne,
                          const Vec2f & _handleTwo, const Vec2f & _pointTwo);

            Bezier generateBezier(stick::Size _first, stick::Size _last,
                                  const stick::DynamicArray<Float> & _uPrime,
                                  const Vec2f & _tan1, const Vec2f & _tan2);

            // Evaluate a Bezier curve at a particular parameter value
            Vec2f evaluate(stick::Int32 _degree, const Bezier & _curve, Float _t);

            // Given set of points and their parameterization, try to find
            // a better parameterization.
            void reparameterize(stick::Size _first, stick::Size _last,
                                stick::DynamicArray<Float> & _u, const Bezier & _curve);

            // Use Newton-Raphson iteration to find better root.
            Float findRoot(const Bezier & _curve, const Vec2f & _point, Float _u);

            // Assign parameter values to digitized points
            // using relative distances between points.
            void chordLengthParameterize(stick::Size _first, stick::Size _last,
                                         stick::DynamicArray<Float> & _outResult);

            MaxError findMaxError(stick::Size _first, stick::Size _last,
                                  const Bezier & _curve, const stick::DynamicArray<Float> & _u);

        private:

            Path m_path;
            Float m_error;
            SegmentDescArray m_newSegments;
            PositionArray m_positions;
        };
    }
}

#endif //PAPER_PRIVATE_PATHFITTER_HPP
