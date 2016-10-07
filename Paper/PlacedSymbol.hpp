#ifndef PAPER_PLACEDSYMBOL_HPP
#define PAPER_PLACEDSYMBOL_HPP

#include <Paper/Item.hpp>

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
    };
}

#endif //PAPER_PLACEDSYMBOL_HPP
