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

    void BaseGradient::setOrigin(const Vec2f & _position)
    {
        set<comps::Origin>(_position);
        markPositionsDirty();
    }

    void BaseGradient::setDestination(const Vec2f & _position)
    {
        set<comps::Destination>(_position);
        markPositionsDirty();
    }

    void BaseGradient::setOriginAndDestination(const Vec2f & _orig, const Vec2f & _dest)
    {
        set<comps::Origin>(_orig);
        set<comps::Destination>(_dest);
        markPositionsDirty();
    }

    void BaseGradient::addStop(const ColorRGBA & _color, Float _offset)
    {
        get<comps::ColorStops>().append({_color, _offset});
        markStopsDirty();
    }

    const Vec2f & BaseGradient::origin() const
    {
        return get<comps::Origin>();
    }

    const Vec2f & BaseGradient::destination() const
    {
        return get<comps::Destination>();
    }

    const ColorStopArray & BaseGradient::stops() const
    {
        return get<comps::ColorStops>();
    }

    void BaseGradient::markStopsDirty()
    {
        get<comps::GradientDirtyFlags>().bStopsDirty = true;
    }

    void BaseGradient::markPositionsDirty()
    {
        get<comps::GradientDirtyFlags>().bPositionsDirty = true;
    }
}
