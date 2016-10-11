#include <Paper/Fill.hpp>

namespace paper
{
    Fill Fill::clone() const
    {
        return cloneImpl();
    }

    FillType Fill::fillType() const
    {

    }

    Fill Fill::cloneImpl() const
    {
        //return Entity::clone();
    }

    NoFill NoFill::clone() const
    {
        
    }

    Fill NoFill::cloneImpl() const
    {

    }

    ColorFill::ColorFill()
    {

    }

    ColorFill::ColorFill(const ColorRGBA & _color)
    {

    }

    void ColorFill::setColor(const ColorRGBA & _color)
    {

    }

    const ColorRGBA & ColorFill::color() const
    {

    }

    ColorFill ColorFill::clone() const
    {

    }

    Fill ColorFill::cloneImpl() const
    {

    }
}
