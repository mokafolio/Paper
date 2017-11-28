#include <Paper/Paint.hpp>

namespace paper
{
    // Paint Paint::clone() const
    // {
    //     return cloneImpl();
    // }

    // PaintType Paint::paintType() const
    // {
    //     STICK_ASSERT(isValid());
    //     return get<comps::PaintType>();
    // }

    // Paint Paint::cloneImpl() const
    // {
    //     return brick::reinterpretEntity<Paint>(Entity::clone());
    // }

    // void Paint::remove()
    // {
    //     Entity::destroy();
    // }

    // NoPaint NoPaint::clone() const
    // {
    //     return brick::reinterpretEntity<NoPaint>(Paint::clone());
    // }

    // void ColorPaint::setColor(const ColorRGBA & _color)
    // {
    //     STICK_ASSERT(isValid());
    //     set<comps::PaintColor>(_color);
    // }

    // const ColorRGBA & ColorPaint::color() const
    // {
    //     STICK_ASSERT(isValid());
    //     return get<comps::PaintColor>();
    // }

    // ColorPaint ColorPaint::clone() const
    // {
    //     return brick::reinterpretEntity<ColorPaint>(Paint::clone());
    // }

    void LinearGradient::setOrigin(const Vec2f & _position)
    {
        m_origin = _position;
    }

    void LinearGradient::setDestination(const Vec2f & _position)
    {
        m_destination = _position;
    }

    void LinearGradient::addStop(const ColorRGBA & _color, Float _offset)
    {
        m_stops.append({_color, _offset});
    }

    const Vec2f & LinearGradient::origin() const
    {
        return m_origin;
    }

    const Vec2f & LinearGradient::destination() const
    {
        return m_destination;
    }

    const ColorStopArray & LinearGradient::stops() const
    {
        return m_stops;
    }
}
