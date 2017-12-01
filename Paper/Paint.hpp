#ifndef PAPER_PAINT_HPP
#define PAPER_PAINT_HPP

#include <Stick/Variant.hpp>
#include <Brick/TypedEntity.hpp>
#include <Brick/Component.hpp>
#include <Paper/BasicTypes.hpp>
#include <Paper/Constants.hpp>

namespace paper
{
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


    namespace comps
    {
        using Origin = brick::Component<ComponentName("Origin"), Vec2f>;
        using Destination = brick::Component<ComponentName("Destination"), Vec2f>;
        using ColorStops = brick::Component<ComponentName("ColorStops"), ColorStopArray>;
        struct STICK_LOCAL GradientDirtyFlagsData
        {
            bool bStopsDirty;
            bool bGeometryDirty;
        };
        using  GradientDirtyFlags = brick::Component<ComponentName("GradientDirtyFlags"), GradientDirtyFlagsData>;
    }

    class STICK_API BaseGradient : public brick::SharedTypedEntity
    {
    public:

        void setOrigin(const Vec2f & _position);

        void setDestination(const Vec2f & _position);

        void setOriginAndDestination(const Vec2f & _orig, const Vec2f & _dest);

        void addStop(const ColorRGBA & _color, Float _offset);


        const Vec2f & origin() const;

        const Vec2f & destination() const;

        const ColorStopArray & stops() const;

    protected:

        void markStopsDirty();

        void markGeometryDirty();
    };

    class STICK_API LinearGradient : public BaseGradient
    {
    };

    class STICK_API RadialGradient : public BaseGradient
    {
    };

    struct STICK_API NoPaint {};
    using Paint = stick::Variant<NoPaint, ColorRGBA, LinearGradient, RadialGradient>;
}

#endif //PAPER_PAINT_HPP
