#ifndef PAPER_SYMBOL_HPP
#define PAPER_SYMBOL_HPP

#include <Paper/Item.hpp>

namespace paper
{
    class PlacedSymbol;
    class Document;

    namespace comps
    {
        //symbol components
        using ReferencedItem = brick::Component<ComponentName("ReferencedItem"), Item>;
        using PlacedSymbols = brick::Component<ComponentName("PlacedSymbols"), stick::DynamicArray<PlacedSymbol> >;
        using Doc = brick::Component<ComponentName("Doc"), Document>;
    }

    //Symbol is not an item as it lives outside of the DOM
    class STICK_API Symbol : public brick::Entity
    {
    public:

        Symbol();

        void assignEntity(const brick::Entity & _e);

        PlacedSymbol place(const Vec2f & _position);

        void remove();

        Item item() const;
    };
}

#endif //PAPER_SYMBOL_HPP
