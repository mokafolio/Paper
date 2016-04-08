#ifndef PAPER_PRIVATE_JOINANDCAP_HPP
#define PAPER_PRIVATE_JOINANDCAP_HPP

#include <Paper/BasicTypes.hpp>
#include <Crunch/CommonFunc.hpp>

namespace paper
{
    namespace detail
    {
        //a collection of math functions to compute joins and caps
        STICK_LOCAL void capSquare(const Vec2f & _position, const Vec2f & _direction,
                                   Vec2f & _outA, Vec2f & _outB, Vec2f & _outC, Vec2f & _outD);

        STICK_LOCAL void capOrJoinRound(stick::DynamicArray<Vec2f> & _outPositions,
                                        const Vec2f & _position,
                                        const Vec2f & _direction,
                                        Float _theta);

        //returns the shortest angle between two vectors
        STICK_LOCAL Float shortestAngle(const Vec2f & _dirA, const Vec2f & _dirB);

        //returns the miter position
        STICK_LOCAL Vec2f joinMiter(const Vec2f & _position,
                                    const Vec2f & _lastStart,
                                    const Vec2f & _lastDir,
                                    const Vec2f & _start,
                                    const Vec2f & _dir,
                                    Float & _outMiterLen);

        //computes the min max positions of a bevel
        STICK_LOCAL void capOrJoinBevelMinMax(const Vec2f & _position,
                                              const Vec2f & _direction,
                                              Vec2f & _a,
                                              Vec2f & _b);
    }
}

#endif //PAPER_PRIVATE_JOINANDCAP_HPP
