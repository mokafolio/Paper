#ifndef PAPER_PRIVATE_STROKETRIANGULATOR_HPP
#define PAPER_PRIVATE_STROKETRIANGULATOR_HPP

#include <Paper/Constants.hpp>
#include <Paper/Private/JoinAndCap.hpp>

namespace paper
{
  class Path;

  namespace detail
  {
    class STICK_LOCAL StrokeTriangulator
    {
    public:

      typedef stick::DynamicArray<Vec2f> PositionArray;
      typedef stick::DynamicArray<bool> JoinArray;


      StrokeTriangulator(const Mat3f & _strokeMat, const Mat3f & _invStrokeMat,
                         StrokeJoin _join, StrokeCap _cap,
                         Float _miterLimit, bool _bIsClosed, const DashArray & _dashArray, Float _dashOffset);


      static Float shortestAngle(const Vec2f & _dirA, const Vec2f & _dirB);

      void makeCapOrJoinRound(const Vec2f & _point, const Vec2f & _dir,
                              PositionArray & _outVertices, Float _theta);

      void makeCapRound(const Vec2f & _point, const Vec2f & _dir,
                        PositionArray & _outVertices, bool _bStart);

      void makeCapSquare(const Vec2f & _point, const Vec2f & _dir,
                         PositionArray & _outVertices, bool _bStart);

      void makeJoinMiter(const Vec2f & _point, const Vec2f & _lastDir, const Vec2f & _dir,
                         const Vec2f & _leftEdgePoint, const Vec2f & _rightEdgePoint,
                         const Vec2f & _lastLeftEdgePoint, const Vec2f & _lastRightEdgePoint,
                         PositionArray & _outVertices);

      void makeJoinBevel(const Vec2f & _point, const Vec2f & _lastDir, const Vec2f & _dir,
                         const Vec2f & _leftEdgePoint, const Vec2f & _rightEdgePoint,
                         const Vec2f & _lastLeftEdgePoint, const Vec2f & _lastRightEdgePoint,
                         PositionArray & _outVertices);

      void makeJoinRound(const Vec2f & _point, const Vec2f & _lastDir, const Vec2f & _dir,
                         PositionArray & _outVertices);

      void makeJoin(const Vec2f & _point, const Vec2f & _lastDir, const Vec2f & _dir,
                    const Vec2f & _leftEdgePoint, const Vec2f & _rightEdgePoint,
                    const Vec2f & _lastLeftEdgePoint, const Vec2f & _lastRightEdgePoint,
                    PositionArray & _outVertices);

      //this function fills _outVertices with all the vertices necessary to render the stroke as a triangle strip
      void triangulateStroke(const PositionArray & _positions, const JoinArray & _joins, PositionArray & _outVertices, bool _bIsClockwise);

    private:

      void triangulateStrokeImpl(const PositionArray & _positions, const JoinArray & _joins, PositionArray & _outVertices, bool _bDashing, bool _bIsClockwise);

      Mat3f m_strokeMat;
      Mat3f m_invStrokeMat;
      StrokeJoin m_join;
      StrokeCap m_cap;
      Float m_miterLimit;
      bool m_bIsClosed;
      DashArray m_dashArray;
      Float m_dashOffset;
      bool m_bTriangleStrip;
    };
  }
}

#endif //PAPER_PRIVATE_STROKETRIANGULATOR_HPP
