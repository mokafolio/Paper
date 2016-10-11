#include <Paper/Paint.hpp>

namespace paper
{
    Paint Paint::clone() const
    {
        return cloneImpl();
    }

    PaintType Paint::paintType() const
    {
        STICK_ASSERT(isValid());
        return get<comps::PaintType>();
    }

    Paint Paint::cloneImpl() const
    {
        return brick::reinterpretEntity<Paint>(Entity::clone());
    }

    void Paint::remove()
    {
        Entity::destroy();
    }

    NoPaint NoPaint::clone() const
    {
        return brick::reinterpretEntity<NoPaint>(Paint::clone());
    }

    void ColorPaint::setColor(const ColorRGBA & _color)
    {
        STICK_ASSERT(isValid());
        set<comps::PaintColor>(_color);
    }

    const ColorRGBA & ColorPaint::color() const
    {
        STICK_ASSERT(isValid());
        return get<comps::PaintColor>();
    }

    ColorPaint ColorPaint::clone() const
    {
        return brick::reinterpretEntity<ColorPaint>(Paint::clone());
    }
}
