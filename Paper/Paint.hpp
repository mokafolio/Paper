#ifndef PAPER_PAINT_HPP
#define PAPER_PAINT_HPP

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

    class STICK_API Paint : public brick::TypedEntityT<Paint>
    {
    public:

        Paint clone() const;

        PaintType paintType() const;

        void remove();

    private:

        virtual Paint cloneImpl() const;
    };

    class STICK_API NoPaint : public Paint
    {
    public:

        static constexpr PaintType paintType = PaintType::None;


        NoPaint clone() const;
    };

    class STICK_API ColorPaint : public Paint
    {
    public:

        static constexpr PaintType paintType = PaintType::Color;

        
        void setColor(const ColorRGBA & _color);

        const ColorRGBA & color() const;

        ColorPaint clone() const;
    };
}

#endif //PAPER_PAINT_HPP
