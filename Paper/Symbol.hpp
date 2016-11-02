#ifndef PAPER_SYMBOL_HPP
#define PAPER_SYMBOL_HPP

#include <Paper/Item.hpp>
#include <Brick/Component.hpp>

namespace paper
{
    class PlacedSymbol;
    class Document;
    using PlacedSymbolArray = stick::DynamicArray<PlacedSymbol>;
    
    namespace comps
    {
        //symbol components
        using ReferencedItem = brick::Component<ComponentName("ReferencedItem"), Item>;
        using PlacedSymbols = brick::Component<ComponentName("PlacedSymbols"), PlacedSymbolArray>;
    }

    //Symbol is not an item as it lives outside of the DOM
    class STICK_API Symbol : public brick::TypedEntityT<Symbol>
    {
    public:

        Symbol();

        PlacedSymbol place(const Vec2f & _position);

        void remove();

        Item item() const;
    };
}

#endif //PAPER_SYMBOL_HPP
