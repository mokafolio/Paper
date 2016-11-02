#include <Paper/Path.hpp>
#include <Paper/Curve.hpp>
#include <Paper/Components.hpp>
#include <Paper/Private/PathFlattener.hpp>

#include <Crunch/StringConversion.hpp>

namespace paper
{
    namespace detail
    {
        bool PathFlattener::isFlatEnough(const Bezier & _curve, Float _tolerance)
        {
            if (_curve.isLinear())
                return true;

            // Comment from paper.js source in Curve.js:
            // Thanks to Kaspar Fischer and Roger Willcocks for the following:
            // http://hcklbrrfnn.files.wordpress.com/2012/08/bez.pdf
            Vec2f u = _curve.handleOne() * 3.0 - _curve.positionOne() * 2.0 - _curve.positionTwo();
            Vec2f v = _curve.handleTwo() * 3.0 - _curve.positionTwo() * 2.0 - _curve.positionOne();
            auto val = std::max(u.x * u.x, v.x * v.x) + std::max(u.y * u.y, v.y * v.y);
            //STICK_ASSERT(!std::isnan(val));
            return val < 10.0 * _tolerance * _tolerance;
        }

        void PathFlattener::flatten(const Path & _path, PositionArray & _outPositions, JoinArray * _outJoins,
                                    Float _angleTolerance, Float _minDistance, stick::Size _maxRecursionDepth)
        {
            auto & curves = _path.get<comps::Curves>();
            auto it = curves.begin();
            for (; it != curves.end(); ++it)
            {
                flattenCurve((*it)->bezier(), (*it)->bezier(), _outPositions, _outJoins, _angleTolerance, _minDistance, 0, _maxRecursionDepth, _path.isClosed(), it == curves.end() - 1);
            }
        }

        void PathFlattener::flattenCurve(const Bezier & _curve, const Bezier & _initialCurve, PositionArray & _outPositions, JoinArray * _outJoins,
                                         Float _angleTolerance, Float _minDistance, stick::Size _recursionDepth, stick::Size _maxRecursionDepth, bool _bIsClosed, bool _bLastCurve)
        {
            if (_recursionDepth < _maxRecursionDepth && !isFlatEnough(_curve, _angleTolerance))
            {
                Bezier::Pair curves = _curve.subdivide(0.5);
                flattenCurve(curves.first, _initialCurve, _outPositions, _outJoins, _angleTolerance, _minDistance, _recursionDepth + 1, _maxRecursionDepth, _bIsClosed, _bLastCurve);
                flattenCurve(curves.second, _initialCurve, _outPositions, _outJoins, _angleTolerance, _minDistance, _recursionDepth + 1, _maxRecursionDepth, _bIsClosed, _bLastCurve);
            }
            else
            {
                Float minDistSquared = _minDistance * _minDistance;
                if (_outPositions.count())
                {
                    if (crunch::distanceSquared(_curve.positionTwo(), _outPositions.last()) >= minDistSquared)
                    {
                        _outPositions.append(_curve.positionTwo());
                        if (_outJoins)
                        {
                            if (_bIsClosed)
                                _outJoins->append(_curve.positionTwo() == _initialCurve.positionTwo());
                            else
                                _outJoins->append(_curve.positionTwo() == _initialCurve.positionTwo() && !_bLastCurve);
                        }
                    }
                }
                else
                {
                    //for the first curve we also add its first segment
                    _outPositions.append(_curve.positionOne());
                    if (_outJoins)
                        _outJoins->append(false);
                    _outPositions.append(_curve.positionTwo());
                    if (_outJoins)
                    {
                        _outJoins->append(_curve.positionTwo() == _initialCurve.positionTwo() && !_bLastCurve);
                    }
                }
            }
        }
    }
}
