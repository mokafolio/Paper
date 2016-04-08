#include <Paper/Private/JoinAndCap.hpp>

namespace paper
{
    namespace detail
    {
        void capSquare(const Vec2f & _position, const Vec2f & _direction,
                       Vec2f & _outA, Vec2f & _outB, Vec2f & _outC, Vec2f & _outD)
        {
            Vec2f perp(-_direction.y, _direction.x);
            _outA = _position + perp;
            _outB = _position - perp;
            _outC = _outB + _direction;
            _outD = _outA + _direction;
        }

        void capOrJoinRound(stick::DynamicArray<Vec2f> & _outPositions, const Vec2f & _position, const Vec2f & _direction,
                            Float _theta)
        {
            Vec2f dir = _direction;
            dir = Vec2f(-dir.y, dir.x);

            stick::Size circleSubdivisionCount = 12;

            Float currentAngle = 0;
            Float radStep = _theta / (Float)(circleSubdivisionCount);
            _outPositions.resize(circleSubdivisionCount + 1);

            for (stick::Size i = 0; i <= circleSubdivisionCount; ++i)
            {
                currentAngle = radStep * i;

                Float cosa = cos(-currentAngle);
                Float sina = sin(-currentAngle);
                _outPositions[i] = _position + Vec2f(dir.x * cosa - dir.y * sina,
                                                     dir.x * sina + dir.y * cosa);
            }
        }

        //returns the shortest angle between two vectors
        Float shortestAngle(const Vec2f & _dirA, const Vec2f & _dirB)
        {
            Float theta = std::acos(crunch::dot(_dirA, _dirB));
            //make sure we have the shortest angle
            if (theta > crunch::Constants<Float>::halfPi())
                theta = crunch::Constants<Float>::pi() - theta;

            return theta;
        }

        //returns the miter position
        Vec2f joinMiter(const Vec2f & _position, const Vec2f & _lastStart, const Vec2f & _lastDir, const Vec2f & _start, const Vec2f & _dir, Float & _outMiterLen)
        {
            Float theta = shortestAngle(_lastDir, _dir);
            _outMiterLen = 1.0 / std::sin(theta / 2.0);

            //compute the intersection
            Float cross = _lastDir.x * _dir.y - _lastDir.y * _dir.x;

            //parallel case
            if (cross == 0)
            {
                return _position; //due to the miter limit this return should not be used as it should switch to bevel
            }

            Vec2f dir = _start - _lastStart;
            Float t = (dir.x * _dir.y - dir.y * _dir.x) / cross;

            return _lastStart + _lastDir * t;
        }

        //computes the min max positions of a bevel
        void capOrJoinBevelMinMax(const Vec2f & _position, const Vec2f & _direction, Vec2f & _a, Vec2f & _b)
        {
            Vec2f perp(-_direction.y, _direction.x);
            _a = _position + perp;
            _b = _position - perp;
        }
    }
}
