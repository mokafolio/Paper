#ifndef PAPER_PAINT_HPP
#define PAPER_PAINT_HPP

#include <Stick/Variant.hpp>
#include <Brick/TypedEntity.hpp>
#include <Brick/Component.hpp>
#include <Paper/BasicTypes.hpp>
#include <Paper/Constants.hpp>

namespace paper
{
    namespace comps
    {
        using PaintColor = brick::Component<ComponentName("PaintColor"), ColorRGBA>;
        using PaintType = brick::Component<ComponentName("PaintType"), PaintType>;
    }

    // @TODO: Reconsider if paints should be entities or simple self contained types:
    // The advantage of having them as entities is that it fits quite nicely into the
    // workflow/api of the rest of the codebase.
    //
    // The disadvantage is that the underlying Hub will allocate memory for all components
    // for all entities. That might be a lot of memory...we shall see.
    //
    // To avoid this we could either create self contained types and possibly say that
    // all paints are value types (the amount of data in them should be pretty small), or
    // use some different kind of shared ownership (i.e. some sort of shared_ptr).
    //
    // Another alternative would be to come up with some form of entity category mechanism
    // in brick to avoid that all components are allocated for all entities but rather on
    // a per category basis.

    // class STICK_API Paint : public brick::SharedTypedEntity
    // {
    // public:

    //     Paint clone() const;

    //     PaintType paintType() const;

    //     void remove();

    // private:

    //     virtual Paint cloneImpl() const;
    // };

    // class STICK_API NoPaint : public Paint
    // {
    // public:

    //     static constexpr PaintType paintType = PaintType::None;

    //     NoPaint clone() const;
    // };

    // class STICK_API ColorPaint : public Paint
    // {
    // public:

    //     static constexpr PaintType paintType = PaintType::Color;


    //     void setColor(const ColorRGBA & _color);

    //     const ColorRGBA & color() const;

    //     ColorPaint clone() const;
    // };

    struct STICK_API ColorStop
    {
        ColorRGBA color;
        Float offset;
    };

    using ColorStopArray = stick::DynamicArray<ColorStop>;

    class STICK_API LinearGradient
    {
    public:

        static constexpr PaintType paintType = PaintType::LinearGradient;


        void setOrigin(const Vec2f & _position);

        void setDestination(const Vec2f & _position);

        void addStop(const ColorRGBA & _color, Float _offset);


        const Vec2f & origin() const;

        const Vec2f & destination() const;

        const ColorStopArray & stops() const;

    private:

        Vec2f m_origin;
        Vec2f m_destination;
        ColorStopArray m_stops;
    };

    struct STICK_API NoPaint{};
    using Paint = stick::Variant<NoPaint, ColorRGBA, LinearGradient>;
}

#endif //PAPER_PAINT_HPP
