#include <Paper/Private/PathFitter.hpp>
#include <Paper/Segment.hpp>
#include <Paper/Constants.hpp>
#include <Paper/Components.hpp>
#include <Paper/Document.hpp>

namespace paper
{
    namespace detail
    {
        using namespace stick;
        using namespace crunch;

        PathFitter::PathFitter(const Path & _p, Float64 _error, bool _bIgnoreClosed) :
            m_path(_p),
            m_error(_error),
            m_bIgnoreClosed(_bIgnoreClosed)
        {
            auto & segs = m_path.segmentArray();
            m_positions.reserve(segs.count());

            Vec2f prev, point;
            prev = point = Vec2f(0);
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

            // printf("PREV C %lu\n", m_positions.count());
            if (m_path.isClosed() && !m_bIgnoreClosed)
            {
                // printf("F %f %f L %f %f\n", m_positions[0].x, m_positions[0].y,
                //        m_positions.last().x, m_positions.last().y);

                //this is kinda unefficient :( we should propably just insert that point first
                //rather than shifting the whole array
                Vec2f last = m_positions.last();
                m_positions.insert(m_positions.begin(), last);
                m_positions.append(m_positions[1]); // The point previously at index 0 is now 1.
            }

            // for (auto & pos : m_positions)
            // {
            //     printf("POS %f %f\n", pos.x, pos.y);
            // }

            // printf("POS COUNT %lu\n", m_positions.count());
        }

        void PathFitter::fit()
        {
            if (m_positions.count() > 0)
            {
                m_newSegments.append({m_positions[0], Vec2f(0), Vec2f(0)});

                stick::Size i = 0;
                stick::Size count = m_newSegments.count();
                bool bReclose = false;

                if (m_positions.count() > 1)
                {
                    fitCubic(0, m_positions.count() - 1,
                             // Left Tangent
                             m_positions[1] - m_positions[0],
                             // Right Tangent
                             m_positions[m_positions.count() - 2] - m_positions[m_positions.count() - 1]);

                    if (m_path.isClosed())
                    {
                        // i++;
                        // if (count > 0)
                        //     count--;
                        bReclose = true;
                        m_path.set<comps::ClosedFlag>(false);

                        // printf("F %f %f L %f %f\n", (*m_newSegments.begin()).position.x, (*m_newSegments.begin()).position.y,
                        //        m_newSegments.last().position.x, m_newSegments.last().position.y);
                        if (!m_bIgnoreClosed)
                        {
                            m_newSegments.remove(m_newSegments.begin());
                            m_newSegments.removeLast();
                        }
                    }
                }

                // m_path.removeSegments();
                auto & segs = m_path.segmentArray();
                segs.clear();
                Document doc = m_path.document();
                for (i = 0; i < m_newSegments.count(); ++i)
                {
                    segs.append(stick::makeUnique<Segment>(doc.allocator(), m_path, m_newSegments[i].position, m_newSegments[i].handleIn, m_newSegments[i].handleOut, segs.count()));
                }

                m_path.rebuildCurves();

                if (bReclose)
                    m_path.closePath();
            }

            // printf("SOOOOO SIMPLEEE\n");
        }

        void PathFitter::fitCubic(stick::Size _first, stick::Size _last, const Vec2f & _tan1, const Vec2f & _tan2)
        {
            // printf("FIRST %lu LAST %lu\n", _first, _last);
            // Use heuristic if region only has two points in it
            if (_last - _first == 1)
            {
                // printf("heuristic\n");
                const Vec2f & pt1 = m_positions[_first];
                const Vec2f & pt2 = m_positions[_last];
                Float64 dist = crunch::distance(pt1, pt2) / 3.0;
                addCurve(pt1, pt1 + normalize(_tan1) * dist,
                         pt2 + normalize(_tan2) * dist, pt2);
                return;
            }

            stick::DynamicArray<Float64> uPrime;
            chordLengthParameterize(_first, _last, uPrime);

            // printf("NORMAL\n");
            Float64 maxError = std::max(m_error, m_error * m_error);
            stick::Size split;
            bool bParametersInOrder = true;

            // Try 4 iterations
            for (stick::Int32 i = 0; i <= 4; ++i)
            {
                Bezier curve = generateBezier(_first, _last, uPrime, _tan1, _tan2);
                // printf("GEN BEZ %f %f, %f %f, %f %f, %f %f\n", curve.positionOne().x, curve.positionOne().y,
                //        curve.handleOne().x, curve.handleOne().y,
                //        curve.handleTwo().x, curve.handleTwo().y,
                //        curve.positionTwo().x, curve.positionTwo().y);

                //  Find max deviation of points to fitted curve
                auto max = findMaxError(_first, _last, curve, uPrime);

                if (max.value < m_error && bParametersInOrder)
                {
                    // printf("ADDING CURVE\n");
                    addCurve(curve.positionOne(), curve.handleOne(), curve.handleTwo(), curve.positionTwo());
                    return;
                }
                split = max.index;

                // If error not too large, try reparameterization and iteration
                if (max.value >= maxError)
                {
                    // printf("MAX ERROR %f %f %f\n", m_error, maxError, max.value);
                    break;
                }

                bParametersInOrder = reparameterize(_first, _last, uPrime, curve);
                maxError = max.value;
            }

            // Fitting failed -- split at max error point and fit recursively
            // Vec2f v1 = m_positions[split - 1] - m_positions[split];
            // Vec2f v2 = m_positions[split] - m_positions[split + 1];
            // Vec2f tanCenter = crunch::normalize((v1 + v2) * 0.5);
            Vec2f tanCenter = m_positions[split - 1] - m_positions[split + 1];
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
                                          const stick::DynamicArray<Float64> & _uPrime,
                                          const Vec2f & _tan1, const Vec2f & _tan2)
        {
            static const Float64 s_epsilon = detail::PaperConstants::epsilon();

            const Vec2f & pt1 = m_positions[_first];
            const Vec2f & pt2 = m_positions[_last];

            Float64 c[2][2] = {{0, 0}, {0, 0}};
            Float64 x[2] = {0, 0};

            for (stick::Size i = 0, l = _last - _first + 1; i < l; ++i)
            {
                Float64 u = _uPrime[i];
                Float64 t = 1.0 - u;
                Float64 b = 3.0 * u * t;
                Float64 b0 = t * t * t;
                Float64 b1 = b * t;
                Float64 b2 = b * u;
                Float64 b3 = u * u * u;
                Vec2f a1 = normalize(_tan1) * b1;
                Vec2f a2 = normalize(_tan2) * b2;
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

            Float64 detC0C1 = c[0][0] * c[1][1] - c[1][0] * c[0][1];
            Float64 alpha1, alpha2;

            if (crunch::abs(detC0C1) > s_epsilon)
            {
                // Kramer's rule
                Float64 detC0X  = c[0][0] * x[1] - c[1][0] * x[0];
                Float64 detXC1  = x[0] * c[1][1] - x[1] * c[0][1];
                // Derive alpha values
                alpha1 = detXC1 / detC0C1;
                alpha2 = detC0X / detC0C1;
            }
            else
            {
                // Matrix is under-determined, try assuming alpha1 == alpha2
                Float64 c0 = c[0][0] + c[0][1];
                Float64 c1 = c[1][0] + c[1][1];

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
            Float64 segLength = crunch::distance(pt1, pt2);
            Float64 epsilon = s_epsilon * segLength;
            Vec2f handleOne(0);
            Vec2f handleTwo(0);
            if (alpha1 < epsilon || alpha2 < epsilon)
            {
                // fall back on standard (probably inaccurate) formula,
                // and subdivide further if needed.
                alpha1 = alpha2 = segLength / 3.0;
            }
            else
            {
                // Check if the found control points are in the right order when
                // projected onto the line through pt1 and pt2.
                Vec2f line = pt2 - pt1;

                handleOne = normalize(_tan1) * alpha1;
                handleTwo = normalize(_tan2) * alpha2;

                if (crunch::dot(handleOne, line) - crunch::dot(handleTwo, line) > segLength * segLength)
                {
                    // Fall back to the Wu/Barsky heuristic above.
                    alpha1 = alpha2 = segLength / 3.0;
                    handleOne = normalize(_tan1) * alpha1;
                    handleTwo = normalize(_tan2) * alpha2;
                }
            }

            // First and last control points of the Bezier curve are
            // positioned exactly at the first and last data points
            // Control points 1 and 2 are positioned an alpha distance out
            // on the tangent vectors, left and right, respectively
            // printf("PT1, %f %f, PT2 %f %f\n", pt1.x, pt1.y, handleOne.x, handleOne.y);
            return Bezier(pt1, pt1 + handleOne,
                          pt2 + handleTwo, pt2);
        }

        Vec2f PathFitter::evaluate(stick::Int32 _degree, const Bezier & _curve, Float64 _t)
        {
            Vec2f tmp[4] = {_curve.positionOne(), _curve.handleOne(), _curve.handleTwo(), _curve.positionTwo()};
            // printf("BEZ %f %f, %f %f, %f %f, %f %f\n", _curve.positionOne().x, _curve.positionOne().y,
            //        _curve.handleOne().x, _curve.handleOne().y,
            //        _curve.handleTwo().x, _curve.handleTwo().y,
            //        _curve.positionTwo().x, _curve.positionTwo().y);
            for (stick::Int32 i = 1; i <= _degree; ++i)
            {
                for (stick::Int32 j = 0; j <= _degree - i; ++j)
                {
                    tmp[j] = tmp[j] * (1 - _t) + tmp[j + 1] * _t;
                }
            }

            return tmp[0];
        }

        bool PathFitter::reparameterize(stick::Size _first, stick::Size _last,
                                        stick::DynamicArray<Float64> & _u, const Bezier & _curve)
        {
            // printf("REPARA\n");
            for (stick::Size i = _first; i <= _last; ++i)
            {
                _u[i - _first] = findRoot(_curve, m_positions[i], _u[i - _first]);
            }

            // Detect if the new parameterization has reordered the points.
            // In that case, we would fit the points of the path in the wrong order.
            for (stick::Size i = 1; i < _u.count(); ++i)
            {
                if (_u[i] <= _u[i - 1])
                    return false;
            }

            return true;
        }

        Float64 PathFitter::findRoot(const Bezier & _curve, const Vec2f & _point, Float64 _u)
        {
            static Float64 s_tolerance = detail::PaperConstants::tolerance();

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
            Float64 df = crunch::dot(pt1, pt1) + crunch::dot(diff, pt2);

            // u = u - f(u) / f'(u)
            return crunch::isClose(df, (Float64)0.0) ? _u : _u - crunch::dot(diff, pt1) / df;
        }

        void PathFitter::chordLengthParameterize(stick::Size _first, stick::Size _last,
                stick::DynamicArray<Float64> & _outResult)
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
                const Bezier & _curve, const stick::DynamicArray<Float64> & _u)
        {
            stick::Size index = crunch::floor((_last - _first + 1) / 2.0);
            Float64 maxDist = 0;
            for (stick::Size i = _first + 1; i < _last; ++i)
            {
                Vec2f p = evaluate(3, _curve, _u[i - _first]);
                // printf("P %f %f\n", p.x, p.y);
                Vec2f v = p - m_positions[i];
                Float64 dist = v.x * v.x + v.y * v.y; // squared
                // printf("D %f\n", dist);
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
