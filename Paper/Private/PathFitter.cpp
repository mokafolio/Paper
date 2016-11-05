#include <Paper/Private/PathFitter.hpp>
#include <Paper/Segment.hpp>
#include <Paper/Constants.hpp>
#include <Crunch/Matrix2.hpp>

namespace paper
{
    namespace detail
    {
        PathFitter::PathFitter(const Path & _p, Float _error) :
            m_path(_p),
            m_error(_error)
        {
            auto & segs = m_path.segmentArray();
            m_positions.reserve(segs.count());

            Vec2f prev, point;
            // Copy over points from path and filter out adjacent duplicates.
            for (stick::Size i = 0; i < segs.count(); ++i)
            {
                point = segs[i]->position();
                if (i < 1 || prev != point)
                {
                    m_positions.append(point);
                    prev = point;
                }
            }

            if (m_path.isClosed())
            {
                //this is kinda unefficient :( we should propably just insert that point first
                //rather than shifting the whole array
                m_positions.insert(m_positions.begin(), m_positions.last());
                m_positions.append(m_positions[1]); // The point previously at index 0 is now 1.
            }
        }

        void PathFitter::fit()
        {
            if (m_positions.count() > 0)
                m_newSegments.append({m_positions[0], Vec2f(0), Vec2f(0)});

            if (m_positions.count() > 1)
            {
                fitCubic(0, m_positions.count() - 1,
                         // Left Tangent
                         crunch::normalize(m_positions[1] - m_positions[0]),
                         // Right Tangent
                         crunch::normalize(m_positions[m_positions.count() - 2] - m_positions[m_positions.count() - 1]));

            }

            stick::Size i = 0;
            stick::Size count = m_newSegments.count();
            if (m_path.isClosed())
            {
                i++;
                count--;
            }
            m_path.removeSegments();

            for (stick::Size i = 0; i < count; ++i)
            {
                m_path.addSegment(m_newSegments[i].position, m_newSegments[i].handleIn, m_newSegments[i].handleOut);
            }
        }

        void PathFitter::fitCubic(stick::Size _first, stick::Size _last, const Vec2f & _tan1, const Vec2f & _tan2)
        {
            // Use heuristic if region only has two points in it
            if (_last - _first == 1)
            {
                const Vec2f & pt1 = m_positions[_first];
                const Vec2f & pt2 = m_positions[_last];
                Float dist = crunch::distance(pt1, pt2) / 3.0;
                addCurve(pt1, pt1 + _tan1 * dist,
                         pt2 + _tan2 * dist, pt2);
                return;
            }

            stick::DynamicArray<Float> uPrime;
            chordLengthParameterize(_first, _last, uPrime);

            Float maxError = std::max(m_error, m_error * m_error);
            stick::Size split;

            // Try 4 iterations
            for (stick::Int32 i = 0; i <= 4; ++i)
            {
                Bezier curve = generateBezier(_first, _last, uPrime, _tan1, _tan2);

                //  Find max deviation of points to fitted curve
                auto max = findMaxError(_first, _last, curve, uPrime);

                if (max.value < m_error)
                {
                    addCurve(curve.positionOne(), curve.handleOne(), curve.handleTwo(), curve.positionTwo());
                    return;
                }
                split = max.index;

                // If error not too large, try reparameterization and iteration
                if (max.value >= maxError)
                    break;

                reparameterize(_first, _last, uPrime, curve);
                maxError = max.value;
            }

            // Fitting failed -- split at max error point and fit recursively
            Vec2f v1 = m_positions[split - 1] - m_positions[split];
            Vec2f v2 = m_positions[split] - m_positions[split + 1];
            Vec2f tanCenter = crunch::normalize((v1 + v2) * 0.5);
            fitCubic(_first, split, _tan1, tanCenter);
            fitCubic(split, _last, -tanCenter, _tan2);
        }

        void PathFitter::addCurve(const Vec2f & _pointOne, const Vec2f & _handleOne,
                                  const Vec2f & _handleTwo, const Vec2f & _pointTwo)
        {
            m_newSegments.last().handleOut = _handleOne - _pointOne;
            m_newSegments.append({_pointTwo, _handleTwo - _pointTwo, Vec2f(0)});
        }

        Bezier PathFitter::generateBezier(stick::Size _first, stick::Size _last,
                                          const stick::DynamicArray<Float> & _uPrime,
                                          const Vec2f & _tan1, const Vec2f & _tan2)
        {
            static const Float s_epsilon = detail::PaperConstants::epsilon();
            typedef crunch::Matrix2<Float> MatrixType;

            const Vec2f & pt1 = m_positions[_first];
            const Vec2f & pt2 = m_positions[_last];

            Float c[2][2] = {{0, 0}, {0, 0}};
            Float x[2] = {0, 0};

            for (stick::Size i = 0, l = _last - _first + 1; i < l; ++i)
            {
                Float u = _uPrime[i];
                Float t = 1.0 - u;
                Float b = 3.0 * u * t;
                Float b0 = t * t * t;
                Float b1 = b * t;
                Float b2 = b * u;
                Float b3 = u * u * u;
                Vec2f a1 = _tan1 * b1;
                Vec2f a2 = _tan2 * b2;
                Vec2f tmp = m_positions[_first + i];
                tmp -= pt1 * (b0 + b1);
                tmp -= pt2 * (b2 + b3);

                c[0][0] += crunch::dot(a1, a1);
                c[0][1] += crunch::dot(a1, a2);

                c[1][0] = c[0][1];
                c[1][1] += crunch::dot(a2, a2);

                x[0] += crunch::dot(a1, tmp);
                x[1] += crunch::dot(a2, tmp);
            }

            Float detC0C1 = c[0][0] * c[1][1] - c[1][0] * c[0][1];
            Float alpha1, alpha2;

            if (crunch::abs(detC0C1) > s_epsilon)
            {
                // Kramer's rule
                Float detC0X  = c[0][0] * x[1] - c[1][0] * x[0];
                Float detXC1  = x[0] * c[1][1] - x[1] * c[0][1];
                // Derive alpha values
                alpha1 = detXC1 / detC0C1;
                alpha2 = detC0X / detC0C1;
            }
            else
            {
                // Matrix is under-determined, try assuming alpha1 == alpha2
                Float c0 = c[0][0] + c[0][1];
                Float c1 = c[1][0] + c[1][1];

                if (crunch::abs(c0) > s_epsilon)
                {
                    alpha1 = alpha2 = x[0] / c0;
                }
                else if (crunch::abs(c1) > s_epsilon)
                {
                    alpha1 = alpha2 = x[1] / c1;
                }
                else
                {
                    // Handle below
                    alpha1 = alpha2 = 0;
                }
            }

            // If alpha negative, use the Wu/Barsky heuristic (see text)
            // (if alpha is 0, you get coincident control points that lead to
            // divide by zero in any subsequent NewtonRaphsonRootFind() call.
            Float segLength = crunch::distance(pt1, pt2);
            Float epsilon = s_epsilon * segLength;
            if (alpha1 < epsilon || alpha2 < epsilon)
            {
                // fall back on standard (probably inaccurate) formula,
                // and subdivide further if needed.
                alpha1 = alpha2 = segLength / 3.0;
            }

            // First and last control points of the Bezier curve are
            // positioned exactly at the first and last data points
            // Control points 1 and 2 are positioned an alpha distance out
            // on the tangent vectors, left and right, respectively
            return Bezier(pt1, pt1 + _tan1 * alpha1,
                              pt2 + _tan2 * alpha2, pt2);
        }

        Vec2f PathFitter::evaluate(stick::Int32 _degree, const Bezier & _curve, Float _t)
        {
            Vec2f tmp[4] = {_curve.positionOne(), _curve.handleOne(), _curve.handleTwo(), _curve.positionTwo()};

            for (stick::Int32 i = 1; i <= _degree; ++i)
            {
                for (stick::Int32 j = 0; j <= _degree - i; ++j)
                {
                    tmp[j] = tmp[j] * (1 - _t) + tmp[j + 1] * _t;
                }
            }

            return tmp[0];
        }

        void PathFitter::reparameterize(stick::Size _first, stick::Size _last,
                                        stick::DynamicArray<Float> & _u, const Bezier & _curve)
        {
            for (stick::Size i = _first; i <= _last; ++i)
            {
                _u[i - _first] = findRoot(_curve, m_positions[i], _u[i - _first]);
            }
        }

        Float PathFitter::findRoot(const Bezier & _curve, const Vec2f & _point, Float _u)
        {
            static Float s_tolerance = detail::PaperConstants::tolerance();

            Bezier curve1;
            Bezier curve2;

            // control vertices for Q'
            curve1.setPositionOne((_curve.handleOne() - _curve.positionOne()) * 3.0);
            curve1.setHandleOne((_curve.handleTwo() - _curve.handleOne()) * 3.0);
            curve1.setHandleTwo((_curve.positionTwo() - _curve.handleTwo()) * 3.0);

            // control vertices for Q''
            curve2.setPositionOne((curve1.handleOne() - curve1.positionOne()) * 2.0);
            curve2.setHandleOne((curve1.handleTwo() - curve1.handleOne()) * 2.0);

            // Compute Q(u), Q'(u) and Q''(u)
            Vec2f pt = evaluate(3, _curve, _u);
            Vec2f pt1 =  evaluate(2, curve1, _u);
            Vec2f pt2 = evaluate(1, curve2, _u);
            Vec2f diff = pt - _point;
            Float df = crunch::dot(pt1, pt1) + crunch::dot(diff, pt2);

            // Compute f(u) / f'(u)
            if (crunch::abs(df) <  s_tolerance)
                return _u;

            // u = u - f(u) / f'(u)
            return _u - crunch::dot(diff, pt1) / df;
        }

        void PathFitter::chordLengthParameterize(stick::Size _first, stick::Size _last,
                stick::DynamicArray<Float> & _outResult)
        {
            stick::Size size = _last - _first;
            _outResult.resize(size + 1);
            _outResult[0] = 0;
            for (stick::Size i = _first + 1; i <= _last; ++i)
            {
                _outResult[i - _first] = _outResult[i - _first - 1]
                                         + crunch::distance(m_positions[i], m_positions[i - 1]);
            }
            for (stick::Size i = 1; i <= size; i++)
                _outResult[i] /= _outResult[size];
        }

        PathFitter::MaxError PathFitter::findMaxError(stick::Size _first, stick::Size _last,
                const Bezier & _curve, const stick::DynamicArray<Float> & _u)
        {
            stick::Size index = crunch::floor((_last - _first + 1) / 2.0);
            Float maxDist = 0;
            for (stick::Size i = _first + 1; i < _last; ++i)
            {
                Vec2f p = evaluate(3, _curve, _u[i - _first]);
                Vec2f v = p - m_positions[i];
                Float dist = v.x * v.x + v.y * v.y; // squared
                if (dist >= maxDist)
                {
                    maxDist = dist;
                    index = i;
                }
            }
            return {maxDist, index};
        }
    }
}
