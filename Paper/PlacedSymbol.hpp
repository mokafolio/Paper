#ifndef PAPER_PLACEDSYMBOL_HPP
#define PAPER_PLACEDSYMBOL_HPP

#include <Paper/Item.hpp>
#include <Brick/Component.hpp>

namespace paper
{
    class Symbol;

    namespace comps
    {
        using ReferencedSymbol = brick::Component<ComponentName("ReferencedSymbol"), Symbol>;
    }

    class STICK_API PlacedSymbol : public Item
    {
    public:

        PlacedSymbol();

        Symbol symbol() const;

        void remove();
    };
}

#endif //PAPER_PLACEDSYMBOL_HPP
