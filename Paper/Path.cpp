#include <Paper/Document.hpp>
#include <Paper/CurveLocation.hpp>
#include <Paper/Private/BooleanOperations.hpp>
#include <Paper/Private/JoinAndCap.hpp>
#include <Paper/Private/PathFlattener.hpp>
#include <Paper/Private/PathFitter.hpp>
#include <Crunch/Line.hpp>
#include <Crunch/StringConversion.hpp>
#include <Crunch/MatrixFunc.hpp>

namespace paper
{
    using namespace stick;

    Path::Path()
    {

    }

    void Path::addPoint(const Vec2f & _to)
    {
        createSegment(_to, Vec2f(0.0), Vec2f(0.0));
    }

    void Path::cubicCurveTo(const Vec2f & _handleOne, const Vec2f & _handleTwo, const Vec2f & _to)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();

        //make relative
        current.setHandleOut(_handleOne - current.position());
        createSegment(_to, _handleTwo - _to, Vec2f(0.0));
    }

    void Path::quadraticCurveTo(const Vec2f & _handle, const Vec2f & _to)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();

        // Comment from paper.js Path.js:
        // This is exact:
        // If we have the three quad points: A E D,
        // and the cubic is A B C D,
        // B = E + 1/3 (A - E)
        // C = E + 1/3 (D - E)

        cubicCurveTo(_handle + (current.position() - _handle) / 3.0, _handle + (_to - _handle) / 3.0 , _to);
    }

    void Path::curveTo(const Vec2f & _through, const Vec2f & _to, Float32 _parameter)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();

        Float t1 = 1 - _parameter;
        Float tt = _parameter * _parameter;
        Vec2f handle = (_through - current.position() * tt - _to * tt) / (2 * _parameter * t1);

        quadraticCurveTo(handle, _to);
    }

    Error Path::arcTo(const Vec2f & _through, const Vec2f & _to)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();

        const Vec2f & from = current.position();

        Vec2f lineOneStart = (from + _through) * 0.5;
        Vec2f lineOneEnd = lineOneStart + crunch::rotate(_through - from, crunch::Constants<Float>::halfPi());
        crunch::Line<Vec2f> lineOne(lineOneStart, lineOneEnd);

        Vec2f lineTwoStart = (_through + _to) * 0.5;
        Vec2f lineTwoEnd = lineTwoStart + crunch::rotate(_to - _through, crunch::Constants<Float>::halfPi());
        crunch::Line<Vec2f> lineTwo(lineTwoStart, lineTwoEnd);

        crunch::IntersectionResult<Vec2f> result = crunch::intersect(lineOne, lineTwo);
        crunch::Line<Vec2f> line(from, _to);
        Int32 throughSide = line.side(_through);

        if (!result)
        {
            if (throughSide == 0)
            {
                // From paper.js
                // If the two lines are colinear, there cannot be an arc as the
                // circle is infinitely big and has no center point. If side is
                // 0, the connecting arc line of this huge circle is a line
                // between the two points, so we can use #lineTo instead.
                // Otherwise we bail out:
                addPoint(_to);
                return Error();
            }
            return Error(ec::InvalidArgument, String::concat("Cannot put an arc through the given points: " , crunch::toString(_through), " and ", crunch::toString(_to)), STICK_FILE, STICK_LINE);
        }
        Vec2f vec =  from - result.intersections()[0];
        Float extent = crunch::toDegrees(crunch::directedAngle(vec, _to - result.intersections()[0]));
        Int32 centerSide = line.side(result.intersections()[0]);
        if (centerSide == 0)
        {
            // If the center is lying on the line, we might have gotten the
            // wrong sign for extent above. Use the sign of the side of the
            // through point.
            extent = throughSide * crunch::abs(extent);
        }
        else if (throughSide == centerSide)
        {
            // If the center is on the same side of the line (from, to) as
            // the through point, we're extending bellow 180 degrees and
            // need to adapt extent.
            extent -= 360.0 * (extent < 0 ? -1 : 1);
        }

        // Float ext = crunch::abs(extent);
        // Int32 count = ext >= 360.0 ? 4 : crunch::ceil(ext / 90.0);
        // Float inc = extent / (Float)count;
        // Float half = inc * crunch::Constants<Float>::pi() / 360.0;
        // Float z = 4.0 / 3.0 * std::sin(half) / (1.0 + std::cos(half));

        // for (Int32 i = 0; i <= count; ++i)
        // {
        //     // Explicitely use to point for last segment, since depending
        //     // on values the calculation adds imprecision:
        //     Vec2f pt = i < count ? result.intersections()[0] + vec : _to;
        //     Vec2f out(-vec.y * z, vec.x * z);
        //     if (i == 0)
        //     {
        //         // Modify startSegment
        //         current.setHandleOut(out);
        //     }
        //     else
        //     {
        //         // Add new Segment
        //         if (i < count)
        //             createSegment(pt, Vec2f(vec.y * z, -vec.x * z), out);
        //         else
        //             createSegment(pt, Vec2f(vec.y * z, -vec.x * z), Vec2f(0.0));
        //     }
        //     vec = crunch::rotate(vec, crunch::toRadians(inc));
        // }
        //return Error();
        return arcHelper(extent, current, vec, _to, result.intersections()[0], nullptr);
    }

    Error Path::arcTo(const Vec2f & _to, bool _bClockwise)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();

        const Vec2f & from = current.position();

        Vec2f mid = (from + _to) * 0.5;
        Vec2f dir = (mid - from);
        dir = !_bClockwise ? Vec2f(-dir.y, dir.x) : Vec2f(dir.y, -dir.x);
        return arcTo(mid + dir, _to);
    }

    Error Path::arcTo(const Vec2f & _to, const Vec2f & _radii, Float _rotation, bool _bClockwise, bool _bLarge)
    {
        STICK_ASSERT(segmentArray().count());
        if (crunch::isClose(_radii.x, (Float)0.0) || crunch::isClose(_radii.y, (Float)0.0))
        {
            addPoint(_to);
            return Error();
        }

        Vec2f from = segmentArray().last()->position();
        Vec2f middle = (from + _to) * 0.5;
        Vec2f pt = crunch::rotate(from - middle, -_rotation);
        Float rx = crunch::abs(_radii.x);
        Float ry = crunch::abs(_radii.y);
        printf("RX, RY %f %f\n", rx, ry);
        Float rxSq = rx * rx;
        Float rySq = ry * ry;
        Float xSq = pt.x * pt.x;
        Float ySq = pt.y * pt.y;
        Float factor = std::sqrt(xSq / rxSq + ySq / rySq);
        if (factor > 1)
        {
            printf("APPLY FACTOR\n");
            rx *= factor;
            ry *= factor;
            rxSq = rx * rx;
            rySq = ry * ry;
        }
        printf("RX2, RY2 %f %f\n", rx, ry);

        factor = (rxSq * rySq - rxSq * ySq - rySq * xSq) / (rxSq * ySq + rySq * xSq);
        if (crunch::abs(factor) < detail::PaperConstants::trigonometricEpsilon())
        {
            printf("SET FACTOR TO 0\n");
            factor = 0;
        }

        if (factor < 0)
        {
            return Error(ec::InvalidArgument, "Cannot create an arc with the given arguments", STICK_FILE, STICK_LINE);
        }

        Vec2f center(rx * pt.y / ry, -ry * pt.x / rx);
        center = crunch::rotate(center * ((_bLarge == _bClockwise ? -1 : 1) * std::sqrt(factor)), _rotation) + middle;
        // Now create a matrix that maps the unit circle to the ellipse,
        // for easier construction below.
        Mat3f matrix = Mat3f::translation2D(center);
        matrix.rotate2D(_rotation);
        matrix.scale(_radii);
        Mat3f inv = crunch::inverse(matrix);
        Vec2f vect = inv * from;
        Float extent = crunch::directedAngle(vect, inv * _to);

        if (!_bClockwise && extent > 0)
            extent -= crunch::Constants<Float>::twoPi();
        else if (_bClockwise && extent < 0)
            extent += crunch::Constants<Float>::twoPi();

        return arcHelper(crunch::toDegrees(extent), *segmentArray().last(), vect, _to, center, &matrix);
    }

    Error Path::arcHelper(Float _extentDeg, Segment & _segment, const Vec2f & _direction, const Vec2f & _to, const Vec2f & _center, const Mat3f * _transform)
    {
        Float ext = crunch::abs(_extentDeg);
        Int32 count = ext >= 360.0 ? 4 : crunch::ceil(ext / 90.0);
        Float inc = _extentDeg / (Float)count;
        Float half = inc * crunch::Constants<Float>::pi() / 360.0;
        Float z = 4.0 / 3.0 * std::sin(half) / (1.0 + std::cos(half));
        Vec2f dir = _direction;

        for (Int32 i = 0; i <= count; ++i)
        {
            // Explicitely use to point for last segment, since depending
            // on values the calculation adds imprecision:
            Vec2f pt = _to;
            Vec2f out(-dir.y * z, dir.x * z);
            if (i < count)
            {
                if (_transform)
                {
                    pt = *_transform * dir;
                    out = (*_transform * (dir + out)) - pt;
                }
                else
                {
                    pt = _center + dir;
                }
            }
            if (i == 0)
            {
                // Modify startSegment
                _segment.setHandleOut(out);
            }
            else
            {
                // Add new Segment
                Vec2f in(dir.y * z, -dir.x * z);
                if (!_transform)
                {
                    createSegment(pt, in, i < count ? out : Vec2f(0.0));
                }
                else
                {
                    createSegment(pt, (*_transform * (dir + in)) - pt, i < count ? out : Vec2f(0.0));
                }
            }
            dir = crunch::rotate(dir, crunch::toRadians(inc));
        }
        return Error();
    }

    void Path::cubicCurveBy(const Vec2f & _handleOne, const Vec2f & _handleTwo, const Vec2f & _by)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();
        cubicCurveTo(current.position() + _handleOne, current.position() + _handleTwo, current.position() + _by);
    }

    void Path::quadraticCurveBy(const Vec2f & _handle, const Vec2f & _by)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();
        quadraticCurveTo(current.position() + _handle, current.position() + _by);
    }

    void Path::curveBy(const Vec2f & _through, const Vec2f & _by, Float32 _parameter)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();
        curveTo(current.position() + _through, current.position() + _by, _parameter);
    }

    Error Path::arcBy(const Vec2f & _through, const Vec2f & _by)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();
        return arcTo(current.position() + _through, current.position() + _by);
    }

    Error Path::arcBy(const Vec2f & _to, bool _bClockwise)
    {
        STICK_ASSERT(segmentArray().count());
        Segment & current = *segmentArray().last();
        return arcTo(current.position() + _to, _bClockwise);
    }

    void Path::closePath()
    {
        if (isClosed())
            return;

        if (segmentArray().count() > 1)
        {
            Segment & first = *segmentArray()[0];
            Segment & last = *segmentArray().last();

            if (crunch::isClose(first.position(), last.position(), detail::PaperConstants::tolerance()))
            {
                first.setHandleIn(last.handleIn());
                curveArray().last()->m_segmentB = first.m_index;
                segmentArray().removeLast();
            }
            else
            {
                curveArray().append(document().allocator().create<Curve>(*this, last.m_index, first.m_index));
            }

            set<comps::ClosedFlag>(true);
            markGeometryDirty(true);
            markBoundsDirty(true);
        }
    }

    void Path::smooth(Smoothing _type)
    {
        smooth(0, segmentArray().count() - 1, _type);
    }

    static Size smoothIndex(Int64 _idx, Int64 _length, bool _bClosed)
    {
        // Handle negative values based on whether a path is open or not:
        // Ranges on closed paths are allowed to wrapped around the
        // beginning/end (e.g. start near the end, end near the beginning),
        // while ranges on open paths stay within the path's open range.
        return min(_idx < 0 && _bClosed
                   ? _idx % _length
                   : _idx < 0 ? _idx + _length : _idx, _length - 1);
    }

    void Path::smooth(Int64 _from, Int64 _to, Smoothing _type)
    {
        // Continuous smoothing approach based on work by Lubos Brieda,
        // Particle In Cell Consulting LLC, but further simplified by
        // addressing handle symmetry across segments, and the possibility
        // to process x and y coordinates simultaneously. Also added
        // handling of closed paths.
        // https://www.particleincell.com/2012/bezier-splines/
        //
        // We use different parameters for the two supported smooth methods
        // that use this algorithm: continuous and asymmetric. asymmetric
        // was the only approach available in v0.9.25 & below.

        _from = smoothIndex(_from, segmentArray().count(), isClosed());
        _to = smoothIndex(_to, segmentArray().count(), isClosed());
        if (_from > _to)
        {
            if (isClosed())
            {
                _from -= segmentArray().count();
            }
            else
            {
                Size tmp = _from;
                _from = _to;
                _to = tmp;
            }
        }

        if (_type == Smoothing::Asymmetric || _type == Smoothing::Continuous)
        {
            auto & segs = get<comps::Segments>();
            bool bAsymmetric = _type == Smoothing::Asymmetric;
            Int64 amount = _to - _from + 1;
            Int64 n = amount - 1;
            bool bLoop = isClosed() && _from == 0 && _to == segs.count() - 1;

            // Overlap by up to 4 points on closed paths since a current
            // segment is affected by its 4 neighbors on both sides (?).
            Int64 padding = bLoop ? min(amount, (Int64)4) : 1;
            Int64 paddingLeft = padding;
            Int64 paddingRight = padding;

            if (!isClosed())
            {
                // If the path is open and a range is defined, try using a
                // padding of 1 on either side.
                paddingLeft = min((Int64)1, _from);
                paddingRight = min((Int64)1, (Int64)segs.count() - _to - 1);
            }

            n += paddingLeft + paddingRight;
            if (n <= 1)
                return;

            DynamicArray<Vec2f> knots(n + 1);

            for (Int64 i = 0, j = _from - paddingLeft; i <= n; i++, j++)
            {
                knots[i] = segs[(j < 0 ? j + segs.count() : j) % segs.count()]->position();
            }

            // In the algorithm we treat these 3 cases:
            // - left most segment (L)
            // - internal segments (I)
            // - right most segment (R)
            //
            // In both the continuous and asymmetric method, c takes these
            // values and can hence be removed from the loop starting in n - 2:
            // c = 1 (L), 1 (I), 0 (R)
            //
            // continuous:
            // a = 0 (L), 1 (I), 2 (R)
            // b = 2 (L), 4 (I), 7 (R)
            // u = 1 (L), 4 (I), 8 (R)
            // v = 2 (L), 2 (I), 1 (R)
            //
            // asymmetric:
            // a = 0 (L), 1 (I), 1 (R)
            // b = 2 (L), 4 (I), 2 (R)
            // u = 1 (L), 4 (I), 3 (R)
            // v = 2 (L), 2 (I), 0 (R)

            // (L): u = 1, v = 2
            Float x = knots[0].x + 2 * knots[1].x;
            Float y = knots[0].y + 2 * knots[1].y;
            Float f = 2;
            Float n1 = n - 1;
            DynamicArray<Float> rx(n + 1);
            rx[0] = x;
            DynamicArray<Float> ry(n + 1);
            ry[0] = y;
            DynamicArray<Float> rf(n + 1);
            rf[0] = f;

            DynamicArray<Float> px(n + 1);
            DynamicArray<Float> py(n + 1);

            // Solve with the Thomas algorithm
            for (Size i = 1; i < n; i++)
            {
                bool bInternal = i < n1;
                // internal--(I)  asymmetric--(R) (R)--continuous
                Float a = bInternal ? 1 : bAsymmetric ? 1 : 2;
                Float b = bInternal ? 4 : bAsymmetric ? 2 : 7;
                Float u = bInternal ? 4 : bAsymmetric ? 3 : 8;
                Float v = bInternal ? 2 : bAsymmetric ? 0 : 1;
                Float m = a / f;
                f = rf[i] = b - m;
                x = rx[i] = u * knots[i].x + v * knots[i + 1].x - m * x;
                y = ry[i] = u * knots[i].y + v * knots[i + 1].y - m * y;
            }

            px[n1] = rx[n1] / rf[n1];
            py[n1] = ry[n1] / rf[n1];
            for (Int64 i = n - 2; i >= 0; i--)
            {
                px[i] = (rx[i] - px[i + 1]) / rf[i];
                py[i] = (ry[i] - py[i + 1]) / rf[i];
            }
            px[n] = (3 * knots[n].x - px[n1]) / 2;
            py[n] = (3 * knots[n].y - py[n1]) / 2;

            // Now update the segments
            for (Int64 i = paddingLeft, max = n - paddingRight, j = _from; i <= max; i++, j++)
            {
                Int64 index = j < 0 ? j + segs.count() : j;
                Segment & segment = *segs[index];
                Float hx = px[i] - segment.position().x;
                Float hy = py[i] - segment.position().y;
                if (bLoop || i < max)
                {
                    segment.setHandleOut(Vec2f(hx, hy));
                }
                if (bLoop || i > paddingLeft)
                {
                    segment.setHandleIn(Vec2f(-hx, -hy));
                }
            }

            rebuildCurves();
            markGeometryDirty(true);
            markBoundsDirty(true);
        }
    }

    void Path::simplify(Float _tolerance)
    {
        detail::PathFitter fitter(*this, _tolerance);
        fitter.fit();
    }

    void Path::addSegment(const Vec2f & _point, const Vec2f & _handleIn, const Vec2f & _handleOut)
    {
        createSegment(_point, _handleIn, _handleOut);
    }

    Segment & Path::insertSegment(Size _index, const Vec2f & _pos,
                                  const Vec2f & _handleIn,
                                  const Vec2f & _handleOut)
    {
        auto & segs = segmentArray();
        auto seg = document().allocator().create<Segment>(*this, _pos, _handleIn, _handleOut, _index);
        //append case
        if (_index >= segs.count())
        {
            segs.append(std::move(seg));
            if (segs.count() > 1)
            {
                auto & curves = curveArray();
                curves.append(document().allocator().create<Curve>(*this, segs.count() - 2, segs.count() - 1));
                if (isClosed())
                    curves.append(document().allocator().create<Curve>(*this, segs.count() - 1, 0));
            }
        }
        //insert case
        else
        {
            auto sit = segs.insert(segs.begin() + _index, std::move(seg));

            //insert the new curve created by due to the segment insertion
            auto & curves = curveArray();
            auto cit = curves.begin() + _index;
            cit = curves.insert(cit, document().allocator().create<Curve>(*this, seg->m_index , seg->m_index + 1));

            //iterate over all the segments after the new one and update their indices
            ++sit;
            for (; sit != segs.end(); ++sit)
            {
                (*sit)->m_index++;
            }
            ++cit;
            for (; cit != curves.end(); ++cit)
            {
                (*cit)->m_segmentA++;
                if ((*cit)->m_segmentB != 0)
                    (*cit)->m_segmentB++;
            }
        }

        markBoundsDirty(true);
        markGeometryDirty(true);
        return *seg;
    }

    void Path::removeSegment(Size _index)
    {
        auto & segs = segmentArray();
        STICK_ASSERT(_index < segs.count());
        segs.remove(segs.begin() + _index);
        updateSegmentIndices(_index, segmentArray().count());
        rebuildCurves();
        markGeometryDirty(true);
    }

    void Path::removeSegments(Size _from)
    {
        auto & segs = segmentArray();
        STICK_ASSERT(_from < segs.count());
        segs.remove(segs.begin() + _from, segs.end());
        rebuildCurves();
        markGeometryDirty(true);
    }

    void Path::removeSegments(Size _from, Size _to)
    {
        auto & segs = segmentArray();
        STICK_ASSERT(_from < segs.count());
        STICK_ASSERT(_to < segs.count());
        segs.remove(segs.begin() + _from, segs.begin() + _to);
        updateSegmentIndices(_from, _to);
        rebuildCurves();
        markGeometryDirty(true);
    }

    void Path::removeSegments()
    {
        segmentArray().clear();
        curveArray().clear();
        markGeometryDirty(true);
    }

    Segment & Path::createSegment(const Vec2f & _pos, const Vec2f & _handleIn, const Vec2f & _handleOut)
    {
        return insertSegment(segmentArray().count(), _pos, _handleIn, _handleOut);
    }

    void Path::reverse()
    {
        //TODO: Can be optimized
        auto & segs = get<comps::Segments>();
        std::reverse(segs.begin(), segs.end());
        Size i = 0;
        Vec2f tmp;
        for (; i < segs.count(); ++i)
        {
            segs[i]->m_index = i;
            tmp = segs[i]->m_handleOut;
            segs[i]->m_handleOut = segs[i]->m_handleIn;
            segs[i]->m_handleIn = tmp;
        }

        for (auto & c : get<comps::Children>())
        {
            Path p = brick::reinterpretEntity<Path>(c);
            p.reverse();
        }

        rebuildCurves();
        markGeometryDirty(true);
    }

    void Path::setClockwise(bool _b)
    {
        if (isClockwise() != _b)
            reverse();
    }

    void Path::flatten(Float _angleTolerance, Float _minDistance, Size _maxRecursion)
    {
        detail::PathFlattener::PositionArray newSegmentPositions;
        newSegmentPositions.reserve(1024);
        detail::PathFlattener::flatten(*this, newSegmentPositions, nullptr, _angleTolerance, _minDistance, _maxRecursion);
        auto & segs = segmentArray();
        segs.clear();

        for (const auto & pos : newSegmentPositions)
        {
            segs.append(document().allocator().create<Segment>(*this, pos, Vec2f(0.0f), Vec2f(0.0f), segs.count()));
        }

        //make sure to possibly remove duplicate closing segments again
        bool bClosed = isClosed();
        if (bClosed)
        {
            set<comps::ClosedFlag>(false);
            closePath();
        }

        rebuildCurves();
        markBoundsDirty(true);
        markGeometryDirty(true);
    }

    void Path::flattenRegular(Float _maxDistance)
    {
        DynamicArray<Vec2f> tmp;
        Float currentOffset = 0;
        Float len = length();
        Size samples = 0;
        auto stepAndSampleCount = regularOffsetAndSampleCount(_maxDistance);
        Float step = stepAndSampleCount.offset;
        for (Int32 i = 0; i < stepAndSampleCount.sampleCount; ++i)
        {
            tmp.append(positionAt(std::min(currentOffset, len)));
            Float delta = len - currentOffset;
            if (delta < step)
            {
                step = delta * 0.5;
            }
            currentOffset += step;
        }

        /*//due to floating point in accuracies, the last sample might overshoot the length of the path,
        //if that is the case, we just sample the end of the path
        if (samples < stepAndSampleCount.sampleCount)
        {
            printf("ADD LAST %f, %lu\n", len, curveArray().count());
            tmp.append(positionAt(len));
        }*/

        auto & segs = get<comps::Segments>();
        segs.clear();
        for (const Vec2f & p : tmp)
        {
            segs.append(document().allocator().create<Segment>(*this, p, Vec2f(0), Vec2f(0), segs.count()));
        }

        //printf("NUM NEW SEGS %lu\n", segs.count());

        //make sure to possibly remove duplicate closing segments again
        bool bClosed = isClosed();
        if (bClosed)
        {
            set<comps::ClosedFlag>(false);
            closePath();
        }

        rebuildCurves();
        markBoundsDirty(true);
        markGeometryDirty(true);
    }

    Path::OffsetAndSampleCount Path::regularOffsetAndSampleCount(Float _maxDistance)
    {
        Float len = length();
        Size sampleCount = std::ceil(len / _maxDistance);
        return {std::min(len, len / (Float)sampleCount), sampleCount};
    }

    Float Path::regularOffset(Float _maxDistance)
    {
        return regularOffsetAndSampleCount(_maxDistance).offset;
    }

    Path::SegmentViewConst Path::segments() const
    {
        STICK_ASSERT(hasComponent<comps::Segments>());
        return SegmentViewConst(segmentArray().begin(), segmentArray().end());
    }

    Path::SegmentView Path::segments()
    {
        STICK_ASSERT(hasComponent<comps::Segments>());
        //return get<comps::Segments>();
        return SegmentView(segmentArray().begin(), segmentArray().end());
    }

    Path::CurveViewConst Path::curves() const
    {
        STICK_ASSERT(hasComponent<comps::Curves>());
        return CurveViewConst(curveArray().begin(), curveArray().end());
    }

    Path::CurveView Path::curves()
    {
        STICK_ASSERT(hasComponent<comps::Curves>());
        return CurveView(curveArray().begin(), curveArray().end());
    }

    SegmentArray & Path::segmentArray()
    {
        STICK_ASSERT(hasComponent<comps::Segments>());
        return get<comps::Segments>();
    }

    const SegmentArray & Path::segmentArray() const
    {
        STICK_ASSERT(hasComponent<comps::Segments>());
        return get<comps::Segments>();
    }

    CurveArray & Path::curveArray()
    {
        STICK_ASSERT(hasComponent<comps::Curves>());
        return get<comps::Curves>();
    }

    const CurveArray & Path::curveArray() const
    {
        STICK_ASSERT(hasComponent<comps::Curves>());
        return get<comps::Curves>();
    }

    Vec2f Path::positionAt(Float _offset) const
    {
        auto loc = curveLocationAt(_offset);
        return loc.position();
    }

    Vec2f Path::normalAt(Float _offset) const
    {
        auto loc = curveLocationAt(_offset);
        return loc.normal();
    }

    Vec2f Path::tangentAt(Float _offset) const
    {
        auto loc = curveLocationAt(_offset);
        return loc.tangent();
    }

    Float Path::curvatureAt(Float _offset) const
    {
        auto loc = curveLocationAt(_offset);
        return loc.curvature();
    }

    Float Path::angleAt(Float _offset) const
    {
        auto loc = curveLocationAt(_offset);
        return loc.angle();
    }

    Path Path::splitAt(Float _offset)
    {

    }

    Path Path::splitAt(const CurveLocation & _loc)
    {

    }

    CurveLocation Path::closestCurveLocation(const Vec2f & _point, Float & _outDistance) const
    {
        Float minDist = std::numeric_limits<Float>::infinity();
        Float currentDist;
        Float closestParameter;
        Float currentParameter;
        auto closestCurve = curveArray().end();

        auto it = curveArray().begin();

        for (; it != curveArray().end(); ++it)
        {
            currentParameter = (*it)->closestParameter(_point, currentDist);
            if (currentDist < minDist)
            {
                minDist = currentDist;
                closestParameter = currentParameter;
                closestCurve = it;
            }
        }

        if (closestCurve != curveArray().end())
        {
            _outDistance = minDist;
            return (*closestCurve)->curveLocationAtParameter(closestParameter);
        }

        return CurveLocation();
    }

    CurveLocation Path::closestCurveLocation(const Vec2f & _point) const
    {
        Float tmp;
        return closestCurveLocation(_point, tmp);
    }

    CurveLocation Path::curveLocationAt(Float _offset) const
    {
        Float len = 0;
        Float start;

        auto it = curveArray().begin();

        for (; it != curveArray().end(); ++it)
        {
            start = len;
            len += (*it)->length();

            //we found the curve
            if (len >= _offset)
            {
                return (*it)->curveLocationAt(_offset - start);
            }
        }

        // comment from paper.js source in Path.js:
        // It may be that through impreciseness of getLength (length) , that the end
        // of the curves was missed:
        if (_offset <= length())
        {
            return curveArray().last()->curveLocationAtParameter(1);
        }

        return CurveLocation();
    }

    Float Path::length() const
    {
        auto & lenData = const_cast<Path *>(this)->get<comps::PathLength>();
        if (lenData.bDirty)
        {
            lenData.length = 0;
            for (const auto & c : curveArray())
            {
                lenData.length += c->length();
            }
            lenData.bDirty = false;
        }
        return lenData.length;
    }

    Float Path::area() const
    {
        Float ret = 0;
        for (const auto & c : curveArray())
        {
            ret += c->area();
        }
        return ret;
    }

    bool Path::isClosed() const
    {
        return get<comps::ClosedFlag>();
    }

    bool Path::isPolygon() const
    {
        for (const auto & seg : segmentArray())
        {
            if (!seg->isLinear())
                return false;
        }
        return true;
    }

    bool Path::isClockwise() const
    {
        // Comment from paper.js source:
        // Method derived from:
        // http://stackoverflow.com/questions/1165647
        // We treat the curve points and handles as the outline of a polygon of
        // which we determine the orientation using the method of calculating
        // the sum over the edges. This will work even with non-convex polygons,
        // telling you whether it's mostly clockwise
        // TODO: Check if this works correctly for all open paths.

        Float sum = 0;

        Size i = 0;
        Size i2;
        auto & segs = segmentArray();
        Size s = segs.count();
        for (; i < s; ++i)
        {
            Vec2f posA = segs[i]->position();
            Vec2f handleA = posA + segs[i]->handleOut();
            i2 = (i + 1) % s;
            Vec2f posB = segs[i2]->position();
            Vec2f handleB = posB + segs[i2]->handleIn();

            sum += (posA.x - handleA.x) * (handleA.y + posA.y);
            sum += (handleA.x - handleB.x) * (handleB.y + handleA.y);
            sum += (handleB.x - posB.x) * (posB.y + handleB.y);
        }

        return sum > 0;
    }

    void Path::rebuildCurves()
    {
        auto & curves = curveArray();
        curves.clear();
        auto & segs = segmentArray();
        for (Size i = 0; i < segs.count() - 1; ++i)
        {
            curves.append(document().allocator().create<Curve>(*this, i, i + 1));
        }

        if (isClosed() && segs.count() > 1)
            curves.append(document().allocator().create<Curve>(*this, segs.count() - 1, 0));
    }

    void Path::updateSegmentIndices(Size _from, Size _to)
    {
        for (Size i = _from; i < _to && i < segmentArray().count(); ++i)
        {
            segmentArray()[i]->m_index = i;
        }
    }

    void Path::segmentChanged(const Segment & _seg)
    {
        //printf("SEGMENT CHANGED A %lu\n", curveArray().count());
        if (curveArray().count() == 0)
            return;

        if (_seg.m_index == 0)
        {
            printf("SEGMENT CHANGED B\n");
            curveArray().first()->markDirty();
            if (isClosed())
                curveArray().last()->markDirty();
        }
        else if (_seg.m_index == segmentArray().count() - 1)
        {
            printf("SEGMENT CHANGED C\n");
            curveArray().last()->markDirty();
            if (isClosed())
                curveArray().first()->markDirty();
        }
        else
        {
            printf("SEGMENT CHANGED D\n");
            curveArray()[_seg.m_index - 1]->markDirty();
            curveArray()[_seg.m_index]->markDirty();
        }
        markBoundsDirty(true);
        markGeometryDirty(true);
        //printf("SEGMENT CHANGED E\n");
    }

    namespace detail
    {
        //a helper to store segment data in stroke space
        struct SegmentData
        {
            Vec2f position;
            Vec2f handleIn;
            Vec2f handleOut;
        };

        //helpers for updateStrokeBounds
        static void mergeStrokeCap(Rect & _rect, StrokeCap _cap, const SegmentData & _a, const SegmentData & _b, bool _bStart,
                                   const Vec2f & _strokePadding, const Mat3f & _strokeMat, const Mat3f * _transform)
        {
            Bezier curve(_a.position, _a.handleOut, _b.handleIn, _b.position);
            Vec2f dir = _bStart ? -curve.tangentAt(0) : curve.tangentAt(1);
            Vec2f pos = _bStart ? _a.position : _b.position;
            switch (_cap)
            {
                case StrokeCap::Square:
                    {
                        Vec2f a, b, c, d;
                        detail::capSquare(pos, dir, a, b, c, d);
                        c = _strokeMat * c;
                        d = _strokeMat * d;
                        _rect = crunch::merge(_rect, _transform ? *_transform * c : c);
                        _rect = crunch::merge(_rect, _transform ? *_transform * d : d);
                        break;
                    }
                case StrokeCap::Round:
                    {
                        Vec2f p = _strokeMat * pos;
                        if (_transform)
                        {
                            p = *_transform * p;
                        }
                        Rect r(p - _strokePadding, p + _strokePadding);
                        _rect = crunch::merge(_rect, r);
                        break;
                    }
                case StrokeCap::Butt:
                    {
                        Vec2f min, max;
                        detail::capOrJoinBevelMinMax(pos, dir, min, max);
                        min = _strokeMat * min;
                        max = _strokeMat * max;
                        _rect = crunch::merge(_rect, _transform ? *_transform * min : min);
                        _rect = crunch::merge(_rect, _transform ? *_transform * max : max);
                        break;
                    }
                default:
                    break;
            }
        }

        static void mergeStrokeJoin(Rect & _rect, StrokeJoin _join, Float _miterLimit,
                                    const SegmentData & _prev, const SegmentData & _current, const SegmentData & _next, const Vec2f & _strokePadding,
                                    const Mat3f & _strokeMat, const Mat3f * _transform)
        {
            switch (_join)
            {
                case StrokeJoin::Round:
                    {
                        Vec2f p = _strokeMat * _current.position;
                        if (_transform)
                        {
                            p = *_transform * p;
                        }
                        Rect r(p - _strokePadding, p + _strokePadding);
                        _rect = crunch::merge(_rect, r);
                        break;
                    }
                case StrokeJoin::Miter:
                    {
                        /*const Curve & curveIn = *_segment.curveIn();
                        const Curve & curveOut = *_segment.curveOut();*/

                        Bezier curveIn(_prev.position, _prev.handleOut, _current.handleIn, _current.position);
                        Bezier curveOut(_current.position, _current.handleOut, _next.handleIn, _next.position);

                        //to know which side of the stroke is outside
                        Vec2f lastDir = curveIn.tangentAt(1);
                        Vec2f nextDir = curveOut.tangentAt(0);
                        Vec2f lastPerp(lastDir.y, -lastDir.x);
                        Vec2f perp(nextDir.y, -nextDir.x);
                        Float cross = lastDir.x * nextDir.y - nextDir.x * lastDir.y;

                        Vec2f miter;
                        Float miterLen;
                        Vec2f pos = _current.position;

                        if (cross >= 0.0)
                        {
                            miter = detail::joinMiter(pos, pos + lastPerp, lastDir,
                                                      pos + perp, nextDir, miterLen);
                        }
                        else
                        {
                            miter = detail::joinMiter(pos, pos - lastPerp, lastDir,
                                                      pos - perp, nextDir, miterLen);
                        }

                        if (miterLen <= _miterLimit)
                        {
                            miter = _strokeMat * miter;
                            _rect = crunch::merge(_rect, _transform ? *_transform * miter : miter);
                            break;
                        }
                        //else fall back to Bevel
                    }
                case StrokeJoin::Bevel:
                    {
                        Bezier curveIn(_prev.position, _prev.handleOut, _current.handleIn, _current.position);
                        Bezier curveOut(_current.position, _current.handleOut, _next.handleIn, _next.position);

                        Vec2f min, max;
                        Vec2f dirA = curveIn.tangentAt(1);
                        Vec2f dirB = curveOut.tangentAt(0);
                        detail::capOrJoinBevelMinMax(_current.position, dirA, min, max);
                        min = _strokeMat * min;
                        max = _strokeMat * max;

                        _rect = crunch::merge(_rect, _transform ? *_transform * min : min);
                        _rect = crunch::merge(_rect, _transform ? *_transform * max : max);
                        detail::capOrJoinBevelMinMax(_current.position, dirB, min, max);
                        min = _strokeMat * min;
                        max = _strokeMat * max;
                        _rect = crunch::merge(_rect, _transform ? *_transform * min : min);
                        _rect = crunch::merge(_rect, _transform ? *_transform * max : max);
                        break;
                    }
                default:
                    break;
            }
        }
    }

    Path::BoundsResult Path::computeBoundsImpl(Float _padding, const Mat3f * _transform)
    {
        if (!segmentArray().count())
            return {true, Rect(0, 0, 0, 0)};

        if (segmentArray().count() == 1)
        {
            Vec2f p = _transform ? *_transform * segmentArray()[0]->position() : segmentArray()[0]->position();
            return {false, Rect(p, p)};
        }

        Rect ret;
        if (!_transform)
        {
            //if not transformation is applied in the document hierarchy, we can simply use the
            //existing beziers as local = global space.
            auto & curves = curveArray();
            for (auto it = curves.begin(); it != curves.end(); ++it)
            {
                if (it == curves.begin())
                    ret = _padding > 0.0 ? (*it)->bounds(_padding) : (*it)->bounds();
                else
                    ret = crunch::merge(ret, _padding > 0.0 ? (*it)->bounds(_padding) : (*it)->bounds());
            }
        }
        else
        {
            /*auto & curves = curveArray();
            for (auto it = curves.begin(); it != curves.end(); ++it)
            {
                if (it == curves.begin())
                    ret = _padding > 0.0 ? (*it).bounds(_padding) : (*it).bounds();
                else
                    ret = crunch::merge(ret, _padding > 0.0 ? (*it).bounds(_padding) : (*it).bounds());
            }

            Vec2f xa = absTrans[0] * ret.min().x;
            Vec2f xb = absTrans[0] * ret.max().x;

            Vec2f ya = absTrans[1] * ret.min().y;
            Vec2f yb = absTrans[1] * ret.max().y;

            Vec2f nmin = crunch::min(xa, xb) + crunch::min(ya, yb) + absTrans[2];
            Vec2f nmax = crunch::max(xa, xb) + crunch::max(ya, yb) + absTrans[2];

            return {Rect(nmin, nmax), false};*/

            //if not, we have to bring the beziers to document space in order
            //to calculate the bounds.

            //we iterate over segments instead so we only do the matrix multiplication
            //once for each segment.
            auto & segs = segmentArray();
            auto it = segs.begin();
            Vec2f lastPosition = *_transform * (*it)->position();
            Vec2f firstPosition = lastPosition;
            Vec2f lastHandle = *_transform * (*it)->handleOutAbsolute();
            ++it;
            Vec2f currentPosition, handleIn;
            for (; it != segs.end(); ++it)
            {
                handleIn = *_transform * (*it)->handleInAbsolute();
                currentPosition = *_transform * (*it)->position();

                Bezier bez(lastPosition, lastHandle, handleIn, currentPosition);
                if (it == segs.begin() + 1)
                    ret = bez.bounds(_padding);
                else
                    ret = crunch::merge(ret, bez.bounds(_padding));

                lastHandle = *_transform * (*it)->handleOutAbsolute();
                lastPosition = currentPosition;
            }
            if (isClosed())
            {
                Bezier bez(lastPosition, lastHandle, *_transform * segs.first()->handleInAbsolute(), firstPosition);
                ret = crunch::merge(ret, bez.bounds(_padding));
            }
        }

        return {false, ret};
    }

    Path::BoundsResult Path::computeBounds(const Mat3f * _transform)
    {
        return computeBoundsImpl(0.0, _transform);
    }

    Path::BoundsResult Path::computeStrokeBounds(const Mat3f * _transform)
    {
        if (!hasStroke())
            return computeBounds(_transform);

        StrokeJoin join = strokeJoin();
        StrokeCap cap = strokeCap();
        Float sw = strokeWidth();
        Float strokeRad = sw * 0.5;
        Float ml = miterLimit();
        bool bIsScalingStroke = isScalingStroke();
        Mat3f transform = _transform ? *_transform : Mat3f::identity();
        Mat3f smat = strokeTransform(_transform, sw, bIsScalingStroke);
        Vec2f sp = strokePadding(strokeRad, bIsScalingStroke ? transform : Mat3f::identity());

        //use proper 2D padding for non uniformly transformed strokes?
        auto result = computeBoundsImpl(max(sp.x, sp.y), _transform);
        bool bIsTransformed = _transform != nullptr;

        //if there is no bounds
        if (result.bEmpty)
            return result;

        auto & segments = segmentArray();
        Size segmentCount = isClosed() ? segments.count() : segments.count() - 1;

        Mat3f ismat = crunch::inverse(smat);
        DynamicArray<detail::SegmentData> strokeSegs(segments.count());
        for (Size i = 0; i < strokeSegs.count(); ++i)
        {
            strokeSegs[i] = {ismat * segments[i]->position(), ismat * (segments[i]->position() + segments[i]->handleIn()), ismat * (segments[i]->position() + segments[i]->handleOut())};
        }

        for (Size i = 1; i < segments.count(); ++i)
        {
            detail::mergeStrokeJoin(result.rect, join, ml, strokeSegs[i - 1], strokeSegs[i], strokeSegs[i + 1], sp, smat, _transform);
        }

        if (isClosed())
        {
            //closing join
            detail::mergeStrokeJoin(result.rect, join, ml, strokeSegs[strokeSegs.count() - 1],
                                    strokeSegs[0], strokeSegs[1], sp, smat, _transform);
        }
        else
        {
            //caps
            detail::mergeStrokeCap(result.rect, cap, strokeSegs[0], strokeSegs[1], true, sp, smat, _transform);
            detail::mergeStrokeCap(result.rect, cap, strokeSegs[strokeSegs.count() - 2],
                                   strokeSegs[strokeSegs.count() - 1], false, sp, smat, _transform);
        }

        //return the merged box;
        return result;
    }

    Path::BoundsResult Path::computeHandleBounds(const Mat3f * _transform)
    {
        auto ret = computeStrokeBounds(_transform);

        if (ret.bEmpty)
            return ret;

        if (_transform)
        {
            auto & segs = segmentArray();
            for (auto & seg : segs)
            {
                ret.rect = crunch::merge(ret.rect, *_transform * seg->handleInAbsolute());
                ret.rect = crunch::merge(ret.rect, *_transform * seg->handleOutAbsolute());
            }
        }
        else
        {
            auto & segs = segmentArray();
            for (auto & seg : segs)
            {
                ret.rect = crunch::merge(ret.rect, seg->handleInAbsolute());
                ret.rect = crunch::merge(ret.rect, seg->handleOutAbsolute());
            }
        }

        return ret;
    }

    bool Path::contains(const Vec2f & _point) const
    {
        if (!handleBounds().contains(_point))
            return false;

        if (windingRule() == WindingRule::EvenOdd)
            return detail::winding(_point, detail::monoCurves(*const_cast<Path *>(this)), false) & 1;
        else
            return detail::winding(_point, detail::monoCurves(*const_cast<Path *>(this)), false) > 0;
    }

    void Path::applyTransform(const Mat3f & _transform)
    {
        for (auto & s : segmentArray())
        {
            s->transform(_transform);
        }
    }

    Path Path::clone() const
    {
        return brick::reinterpretEntity<Path>(Item::clone());
    }

    Curve & Path::curve(Size _index)
    {
        return *curveArray()[_index];
    }

    const Curve & Path::curve(Size _index) const
    {
        return *curveArray()[_index];
    }

    Segment & Path::segment(Size _index)
    {
        return *segmentArray()[_index];
    }

    const Segment & Path::segment(Size _index) const
    {
        return *segmentArray()[_index];
    }

    Size Path::curveCount() const
    {
        return curveArray().count();
    }

    Size Path::segmentCount() const
    {
        return segmentArray().count();
    }
}
