#ifndef PAPER_SYMBOL_HPP
#define PAPER_SYMBOL_HPP

#include <Paper/Item.hpp>

namespace paper
{
    class PlacedSymbol;

    namespace comps
    {
        //symbol components
        using ReferencedItem = brick::Component<ComponentName("ReferencedItem"), Item>;
        using PlacedSymbols = brick::Component <ComponentName("PlacedSymbols"), stick::DynamicArray<PlacedSymbol> >;
    }

    class STICK_API Symbol : public brick::Entity
    {
    public:

        Symbol();

        Symbol(const brick::Entity & _e);

        PlacedSymbol place(const Vec2f & _position);

        void remove();
    };
}

#endif //PAPER_SYMBOL_HPP
