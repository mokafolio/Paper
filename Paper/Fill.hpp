#ifndef PAPER_FILL_HPP
#define PAPER_FILL_HPP

#include <Brick/Entity.hpp>
#include <Brick/Component.hpp>
#include <Paper/BasicTypes.hpp>
#include <Paper/Constants.hpp>

namespace paper
{
    class STICK_API Fill : public brick::Entity
    {
    public:

        Fill clone() const;

        FillType fillType() const;

    private:

        virtual Fill cloneImpl() const;
    };

    class STICK_API NoFill : public Fill
    {
    public:

        static constexpr FillType fillType = FillType::None;

        NoFill clone() const;

    private:

        Fill cloneImpl() const;
    };

    class STICK_API ColorFill : public Fill
    {
    public:

        static constexpr FillType fillType = FillType::Color;

        ColorFill();

        ColorFill(const ColorRGBA & _color);
        
        void setColor(const ColorRGBA & _color);

        const ColorRGBA & color() const;

        ColorFill clone() const;


    private:

        Fill cloneImpl() const;
    };
}

#endif //PAPER_FILL_HPP
