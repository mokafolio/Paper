#include <Paper/Private/StrokeTriangulator.hpp>

#include <Crunch/StringConversion.hpp>

namespace paper
{
    namespace detail
    {
        using namespace stick;

        StrokeTriangulator::StrokeTriangulator(const Mat3f & _strokeMat, const Mat3f & _invStrokeMat,
                                               StrokeJoin _join, StrokeCap _cap,
                                               Float _miterLimit, bool _bIsClosed, const DashArray & _dashArray, Float _dashOffset) :
            m_strokeMat(_strokeMat),
            m_invStrokeMat(_invStrokeMat),
            m_join(_join),
            m_cap(_cap),
            m_miterLimit(_miterLimit),
            m_bIsClosed(_bIsClosed),
            m_bTriangleStrip(!_dashArray.count())
        {
            m_dashArray.resize(_dashArray.count());
            for (Size i = 0; i < _dashArray.count(); ++i)
            {
                //bring the dash length to stroke space
                m_dashArray[i] = (_invStrokeMat * Vec2f(_dashArray[i])).x;
            }
            m_dashOffset = (_invStrokeMat * Vec2f(_dashOffset)).x;
        }

        Float StrokeTriangulator::shortestAngle(const Vec2f & _dirA, const Vec2f & _dirB)
        {
            Float theta = std::acos(crunch::dot(_dirA, _dirB));
            //make sure we have the shortest angle
            if (theta > crunch::Constants<Float>::halfPi())
                theta = crunch::Constants<Float>::pi() - theta;

            return theta;
        }

        void StrokeTriangulator::makeCapOrJoinRound(const Vec2f & _point, const Vec2f & _dir,
                PositionArray & _outVertices, Float _theta)
        {
            PositionArray vertices;
            detail::capOrJoinRound(vertices, _point, _dir, _theta);

            //generate the triangles from the vertices
            Vec2f lastPosition, v;
            for (Size i = 0; i < vertices.count(); ++i)
            {
                v =  vertices[i];
                if (i > 0 && !m_bTriangleStrip)
                    _outVertices.append(lastPosition);

                if (i > 0 && !m_bTriangleStrip)
                {
                    _outVertices.append(v);
                    _outVertices.append(_point);
                }
                else if (m_bTriangleStrip)
                {
                    _outVertices.append(_point);
                    _outVertices.append(v);
                }
                lastPosition = v;
            }
        }

        void StrokeTriangulator::makeCapRound(const Vec2f & _point, const Vec2f & _dir,
                                              PositionArray & _outVertices, bool _bStart)
        {
            makeCapOrJoinRound(_point, _bStart ? _dir * -1 : _dir, _outVertices,
                               crunch::Constants<Float>::pi());
        }

        void StrokeTriangulator::makeCapSquare(const Vec2f & _point, const Vec2f & _dir,
                                               PositionArray & _outVertices, bool _bStart)
        {
            Vec2f a, b, c, d;
            detail::capSquare(_point, _bStart ? -_dir : _dir, a, b, c, d);

            //this makes sure that the triangles are counter clockwise
            if (_bStart)
            {
                _outVertices.append(d);
                if (!m_bTriangleStrip)
                {
                    _outVertices.append(b);
                    _outVertices.append(a);
                    _outVertices.append(b);
                    _outVertices.append(d);
                }
                _outVertices.append(c);
            }
            else
            {
                _outVertices.append(c);
                if (!m_bTriangleStrip)
                {
                    _outVertices.append(b);
                    _outVertices.append(d);
                    _outVertices.append(b);
                    _outVertices.append(a);
                }
                _outVertices.append(d);
            }
        }

        void StrokeTriangulator::makeJoinMiter(const Vec2f & _point, const Vec2f & _lastDir, const Vec2f & _dir,
                                               const Vec2f & _leftEdgePoint, const Vec2f & _rightEdgePoint,
                                               const Vec2f & _lastLeftEdgePoint, const Vec2f & _lastRightEdgePoint,
                                               PositionArray & _outVertices)
        {
            //to know which side of the stroke is outside
            Float cross = _lastDir.x * _dir.y - _dir.x * _lastDir.y;

            //compute the miter
            Float miterLen;
            Vec2f miterTip;

            if (cross >= 0.0)
                miterTip = detail::joinMiter(_point, _lastLeftEdgePoint, _lastDir, _leftEdgePoint, _dir, miterLen);
            else
                miterTip = detail::joinMiter(_point, _lastRightEdgePoint, _lastDir, _rightEdgePoint, _dir, miterLen);

            if (miterLen <= m_miterLimit)
            {
                if (m_bTriangleStrip)
                {
                    _outVertices.append(miterTip);
                    //_outVertices.append(_point);
                }
                else
                {
                    //to create the vertices in the right order
                    if (cross >= 0.0)
                    {
                        _outVertices.append(miterTip);
                        _outVertices.append(_lastLeftEdgePoint);
                        _outVertices.append(_point);

                        _outVertices.append(miterTip);
                        _outVertices.append(_point);
                        _outVertices.append(_leftEdgePoint);
                    }
                    else
                    {
                        _outVertices.append(miterTip);
                        _outVertices.append(_lastRightEdgePoint);
                        _outVertices.append(_point);

                        _outVertices.append(miterTip);
                        _outVertices.append(_point);
                        _outVertices.append(_rightEdgePoint);
                    }
                }
            }
            else
            {
                //fall back to bevel
                makeJoinBevel(_point, _lastDir, _dir,
                              _leftEdgePoint, _rightEdgePoint, _lastLeftEdgePoint, _lastRightEdgePoint,
                              _outVertices);
            }
        }

        void StrokeTriangulator::makeJoinBevel(const Vec2f & _point, const Vec2f & _lastDir, const Vec2f & _dir,
                                               const Vec2f & _leftEdgePoint, const Vec2f & _rightEdgePoint,
                                               const Vec2f & _lastLeftEdgePoint, const Vec2f & _lastRightEdgePoint,
                                               PositionArray & _outVertices)
        {
            //nothing to do if we are in triangle strip mode
            if (m_bTriangleStrip)
                return;

            //add the triangle that fills the join
            Float cross = _lastDir.x * _dir.y - _dir.x * _lastDir.y;

            if (cross >= 0.0)
            {
                _outVertices.append(_lastLeftEdgePoint);
                _outVertices.append(_point);
                _outVertices.append(_leftEdgePoint);
            }
            else
            {
                _outVertices.append(_lastRightEdgePoint);
                _outVertices.append(_point);
                _outVertices.append(_rightEdgePoint);
            }
        }

        void StrokeTriangulator::makeJoinRound(const Vec2f & _point, const Vec2f & _lastDir, const Vec2f & _dir,
                                               PositionArray & _outVertices)
        {
            //due to floating point precision we need to clamp to avoid NaN
            Float theta = std::acos(crunch::clamp(crunch::dot(_lastDir, _dir), -1.0f, 1.0f));
            Float crx = crunch::cross(_lastDir, _dir);
            makeCapOrJoinRound(_point, crx < 0 ? _lastDir : _dir * -1, _outVertices, theta);
        }

        void StrokeTriangulator::makeJoin(const Vec2f & _point, const Vec2f & _lastDir, const Vec2f & _dir,
                                          const Vec2f & _leftEdgePoint, const Vec2f & _rightEdgePoint,
                                          const Vec2f & _lastLeftEdgePoint, const Vec2f & _lastRightEdgePoint,
                                          PositionArray & _outVertices)
        {
            switch (m_join)
            {
            case StrokeJoin::Round:
                makeJoinRound(_point, _lastDir, _dir, _outVertices);
                break;
            case StrokeJoin::Miter:
                makeJoinMiter(_point, _lastDir, _dir,
                              _leftEdgePoint, _rightEdgePoint, _lastLeftEdgePoint, _lastRightEdgePoint,
                              _outVertices);
                break;
            default:
            case StrokeJoin::Bevel:
                makeJoinBevel(_point, _lastDir, _dir,
                              _leftEdgePoint, _rightEdgePoint, _lastLeftEdgePoint, _lastRightEdgePoint,
                              _outVertices);
                break;
            }
        }

        //this function fills _outVertices with all the vertices necessary to render the stroke as a triangle strip
        void StrokeTriangulator::triangulateStroke(const PositionArray & _positions, const JoinArray & _joins, PositionArray & _outVertices, bool _bIsClockwise)
        {
            Vec2f pos, nextPos, dir, nextDir, perp, nextPerp, rightEdge, leftEdge, lastRightEdge, lastLeftEdge;

            //transform the positions to stroke space
            PositionArray positions(_positions);
            for (auto & pos : positions)
                pos = m_invStrokeMat * pos;

            //if the stroke is dashed, we treat every dash as a separate stroke and recursively call triangulateStroke
            //TODO: Optimize this by generating the vertices for the stroke on the fly instead of using the tmpPositions
            //and tmpJoins logic below.

            if (m_dashArray.count())
            {
                DashArray proxy;
                PositionArray tmpPositions;
                tmpPositions.reserve(positions.count());

                JoinArray tmpJoins;
                tmpJoins.reserve(positions.count());

                Size currentDashIndex = 0;
                Size currenPositionIndex = 0;
                Size nextPositionIndex;
                Float currentDashLength = 0.0;
                Float currentSegmentLength;
                Float leftOver;
                bool bSkip = false;

                //find the start dash etc for the dash offset
                if (m_dashOffset > 0)
                {
                    Float patterLen = 0;
                    for (Float val : m_dashArray)
                        patterLen += val;

                    Float offsetIntoPattern = m_dashOffset;
                    if (offsetIntoPattern >= patterLen)
                        offsetIntoPattern = fmod(offsetIntoPattern, patterLen);

                    Size idx = 0;
                    Float off = 0;
                    while (off < offsetIntoPattern)
                    {
                        off += m_dashArray[idx];
                        idx++;
                    }
                    currentDashIndex = idx - 1;
                    currentDashLength = m_dashArray[currentDashIndex] - (off - offsetIntoPattern);
                    bSkip = currentDashIndex % 2;
                }

                Float dashTargetLength = m_dashArray[currentDashIndex];
                Vec2f lastDashEnd = positions[0];
                if(!bSkip)
                {
                    tmpPositions.append(lastDashEnd);
                    tmpJoins.append(false); //TODO: Double check if this is on the start join?
                }
                bool bFirstSkip = bSkip;
                nextPositionIndex = currenPositionIndex + 1;
                do
                {
                    //consume all the segments that are within the current dash
                    while (nextPositionIndex < positions.count())
                    {
                        currentSegmentLength = crunch::distance(lastDashEnd, positions[nextPositionIndex]);
                        if (currentSegmentLength + currentDashLength <= dashTargetLength)
                        {
                            if (!bSkip)
                            {
                                tmpPositions.append(positions[nextPositionIndex]);
                                tmpJoins.append(_joins[nextPositionIndex]);
                            }
                            lastDashEnd = positions[nextPositionIndex];
                            currentDashLength += currentSegmentLength;
                            currenPositionIndex = nextPositionIndex;
                            nextPositionIndex++;
                        }
                        else
                            break;
                    }

                    //take care of the dash end in case its not a perfect fit
                    if (currentDashLength < dashTargetLength && nextPositionIndex < positions.count())
                    {
                        leftOver = dashTargetLength - currentDashLength;
                        dir = crunch::normalize(positions[nextPositionIndex] - lastDashEnd);
                        lastDashEnd = lastDashEnd + dir * leftOver;

                        if (!bSkip)
                        {
                            tmpPositions.append(lastDashEnd);
                            tmpJoins.append(false); //this is in between segments so it cant be a join
                        }
                    }

                    //triangulate the current dash
                    if (!bSkip)
                    {
                        triangulateStrokeImpl(tmpPositions, tmpJoins, _outVertices, true, _bIsClockwise);

                        //manually generate the last closing join if we have to :(
                        //this is stupid right now
                        if (nextPositionIndex == positions.count() && tmpPositions.last() == positions.last() && m_bIsClosed && m_join != StrokeJoin::Bevel && !bFirstSkip)
                        {
                            Vec2f dir = crunch::normalize(positions[0] - positions[positions.count() - 2]);
                            Vec2f perp = Vec2f(-dir.y, dir.x);

                            Vec2f nextDir = crunch::normalize(positions[1] - positions[0]);
                            Vec2f nextPerp = Vec2f(-nextDir.y, nextDir.x);

                            Vec2f leftEdge = positions[0] - nextPerp;
                            Vec2f rightEdge = positions[0] + nextPerp;

                            Vec2f lastLeftEdge = positions[0] - perp;
                            Vec2f lastRightEdge = positions[0] + perp;

                            makeJoin(positions[0], dir, nextDir,
                                     leftEdge, rightEdge, lastLeftEdge, lastRightEdge,
                                     _outVertices);
                        }
                    }

                    //get the next dash and reset all the helpers
                    bSkip = !bSkip;
                    currentDashLength = 0;
                    currentDashIndex = (currentDashIndex + 1) % m_dashArray.count();
                    dashTargetLength = m_dashArray[currentDashIndex];
                    tmpPositions.clear();
                    tmpJoins.clear();
                    if (!bSkip)
                    {
                        tmpPositions.append(lastDashEnd);
                        tmpJoins.append(true); //TODO double check if this is on a join??
                    }

                    // we reached the end
                    if (currenPositionIndex == positions.count() - 1)
                        break;
                }
                while (true);
            }
            else
            {
                // triangulate the whole stroke in one go.
                triangulateStrokeImpl(positions, _joins, _outVertices, false, _bIsClockwise);
            }


            // transform the vertices back to path space
            for (auto & vert : _outVertices)
                vert = m_strokeMat * vert;
        }

        void StrokeTriangulator::triangulateStrokeImpl(const PositionArray & _positions, const JoinArray & _joins, PositionArray & _outVertices, bool _bDashing, bool _bIsClockwise)
        {
            Vec2f pos, nextPos, dir, nextDir, perp, nextPerp, rightEdge, leftEdge, lastRightEdge, lastLeftEdge;

            typename PositionArray::ConstIter it =  _positions.begin();
            typename PositionArray::ConstIter nit, nnit;
            nit = it + 1;
            if (nit == _positions.end())
                return;

            //calculate direction of the first segment
            nextDir = crunch::normalize((*nit) - (*it));
            nextPerp = Vec2f(-nextDir.y, nextDir.x);

            //starting cap if needed
            if (!m_bIsClosed || _bDashing)
            {
                switch (m_cap)
                {
                case StrokeCap::Square:
                    makeCapSquare(_positions[0], nextDir, _outVertices, true);
                    break;
                case StrokeCap::Round:
                    makeCapRound(_positions[0], nextDir, _outVertices, true);
                    break;
                default:
                case StrokeCap::Butt:
                    break;
                }
            }

            //iterate over all the positions and generate the stroke geometry
            for (; it != _positions.end(); ++it)
            {
                nit = it + 1;
                if (nit != _positions.end())
                {
                    pos = *it;
                    nextPos = *nit;
                    dir = nextDir;
                    perp = nextPerp;
                    lastRightEdge = nextPos + perp;
                    lastLeftEdge = nextPos - perp;

                    //add the triangles making up the quad of the stroke segments
                    _outVertices.append((pos - perp));
                    _outVertices.append((pos + perp));
                    _outVertices.append(lastLeftEdge);
                    if (!m_bTriangleStrip)
                    {
                        _outVertices.append(lastLeftEdge);
                        _outVertices.append((pos + perp));
                    }
                    _outVertices.append(lastRightEdge);

                    nnit = nit + 1;

                    //check if there is a join to make
                    if (nnit != _positions.end())
                    {
                        nextDir = crunch::normalize((*nnit) - nextPos);
                        nextPerp = Vec2f(-nextDir.y, nextDir.x);

                        //check if this is a join vertex, only do the work if thats the case!
                        if (_joins[std::distance(_positions.begin(), nit)])
                        {
                            leftEdge = nextPos - nextPerp;
                            rightEdge = nextPos + nextPerp;

                            //take care of the stroke join
                            makeJoin(nextPos, dir, nextDir,
                                     leftEdge, rightEdge, lastLeftEdge, lastRightEdge,
                                     _outVertices);
                        }
                    }
                    else
                    {
                        //take care of end stroke cap or closing join
                        if (m_bIsClosed && _positions.count() >= 2 && !_bDashing)
                        {
                            nextDir = crunch::normalize(_positions[1] - _positions[0]);
                            nextPerp = Vec2f(-nextDir.y, nextDir.x);

                            leftEdge = nextPos - nextPerp;
                            rightEdge = nextPos + nextPerp;

                            makeJoin(nextPos, dir, nextDir,
                                     leftEdge, rightEdge, lastLeftEdge, lastRightEdge,
                                     _outVertices);

                            //if we are in triangle strip mode, we need to manually close the stroke end to the stroke beginning
                            if (m_bTriangleStrip && (m_join == StrokeJoin::Bevel || m_join == StrokeJoin::Miter))
                            {
                                //copy the first vertex to close the triangle strip shape
                                if(_bIsClockwise)
                                    _outVertices.append(_outVertices[0]);
                                else
                                    _outVertices.append(_outVertices[1]);
                            }
                        }
                        else
                        {
                            //cap the end if the path is open
                            switch (m_cap)
                            {
                            case StrokeCap::Square:
                                makeCapSquare(_positions.last(), nextDir, _outVertices, false);
                                break;
                            case StrokeCap::Round:
                                makeCapRound(_positions.last(), nextDir, _outVertices, false);
                                break;
                            default:
                            case StrokeCap::Butt:
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}
