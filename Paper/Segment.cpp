#include <Paper/Segment.hpp>
#include <Paper/Curve.hpp>
#include <Paper/Constants.hpp>

namespace paper
{
    Segment::Segment(const Path & _path,
                     const Vec2f & _pos,
                     const Vec2f & _handleIn,
                     const Vec2f & _handleOut,
                     stick::Size _idx) :
        m_path(_path),
        m_position(_pos),
        m_handleIn(_handleIn),
        m_handleOut(_handleOut),
        m_index(_idx)
    {
    }

    void Segment::setPosition(const Vec2f & _pos)
    {
        m_position = _pos;
        m_path.segmentChanged(*this);
    }

    void Segment::setHandleIn(const Vec2f & _pos)
    {
        m_handleIn = _pos;
        m_path.segmentChanged(*this);
    }

    void Segment::setHandleOut(const Vec2f & _pos)
    {
        m_handleOut = _pos;
        m_path.segmentChanged(*this);
    }

    const Vec2f & Segment::position() const
    {
        return m_position;
    }

    const Vec2f & Segment::handleIn() const
    {
        return m_handleIn;
    }

    const Vec2f & Segment::handleOut() const
    {
        return m_handleOut;
    }

    Vec2f Segment::handleInAbsolute() const
    {
        return m_position + m_handleIn;
    }

    Vec2f Segment::handleOutAbsolute() const
    {
        return m_position + m_handleOut;
    }

    const Curve * Segment::curveIn() const
    {
        return const_cast<Segment *>(this)->curveIn();
    }

    Curve * Segment::curveIn()
    {
        if (m_index == 0)
        {
            if (!m_path.isClosed())
                return nullptr;
            else
                return m_path.curveArray().last().get();
        }
        else
        {
            return m_path.curveArray()[m_index - 1].get();
        }
        return nullptr;
    }

    const Curve * Segment::curveOut() const
    {
        return const_cast<Segment *>(this)->curveOut();
    }

    Curve * Segment::curveOut()
    {
        if (m_index == m_path.segmentArray().count() - 1)
        {
            if (!m_path.isClosed())
                return nullptr;
            else
                return m_path.curveArray().last().get();
        }
        else
        {
            return m_path.curveArray()[m_index].get();
        }
        return nullptr;
    }

    bool Segment::isLinear() const
    {
        if (crunch::isClose(m_handleIn, crunch::Vec2f(0.0), detail::PaperConstants::tolerance()) &&
                crunch::isClose(m_handleOut, crunch::Vec2f(0.0), detail::PaperConstants::tolerance()))
            return true;

        return false;
    }

    void Segment::remove()
    {
        m_path.removeSegment(m_index);
    }

    void Segment::transform(const Mat3f & _transform)
    {
        STICK_ASSERT(!std::isnan(m_handleIn.x));
        STICK_ASSERT(!std::isnan(m_handleIn.y));
        STICK_ASSERT(!std::isnan(m_handleOut.x));
        STICK_ASSERT(!std::isnan(m_handleOut.y));
        STICK_ASSERT(!std::isnan(m_position.x));
        STICK_ASSERT(!std::isnan(m_position.y));

        STICK_ASSERT(!std::isnan(_transform.element(0, 0)));
        STICK_ASSERT(!std::isnan(_transform.element(0, 1)));
        STICK_ASSERT(!std::isnan(_transform.element(0, 2)));

        STICK_ASSERT(!std::isnan(_transform.element(1, 0)));
        STICK_ASSERT(!std::isnan(_transform.element(1, 1)));
        STICK_ASSERT(!std::isnan(_transform.element(1, 2)));

        STICK_ASSERT(!std::isnan(_transform.element(2, 0)));
        STICK_ASSERT(!std::isnan(_transform.element(2, 1)));
        STICK_ASSERT(!std::isnan(_transform.element(2, 2)));

        m_handleIn = _transform * (m_position + m_handleIn);
        m_handleOut = _transform * (m_position + m_handleOut);

        STICK_ASSERT(!std::isnan(m_handleIn.x));
        STICK_ASSERT(!std::isnan(m_handleIn.y));
        STICK_ASSERT(!std::isnan(m_handleOut.x));
        STICK_ASSERT(!std::isnan(m_handleOut.y));
        STICK_ASSERT(!std::isnan(m_position.x));
        STICK_ASSERT(!std::isnan(m_position.y));

        m_position = _transform * m_position;
        m_handleIn -= m_position;
        m_handleOut -= m_position;

        STICK_ASSERT(!std::isnan(m_handleIn.x));
        STICK_ASSERT(!std::isnan(m_handleIn.y));
        STICK_ASSERT(!std::isnan(m_handleOut.x));
        STICK_ASSERT(!std::isnan(m_handleOut.y));
        STICK_ASSERT(!std::isnan(m_position.x));
        STICK_ASSERT(!std::isnan(m_position.y));

        Curve * cin = curveIn();
        Curve * cout = curveOut();
        if (cin)
            cin->markDirty();
        if (cout)
            cout->markDirty();
    }
}
