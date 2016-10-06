#ifndef PAPER_PLACEDSYMBOL_HPP
#define PAPER_PLACEDSYMBOL_HPP

#include <Paper/Item.hpp>

namespace paper
{
    class Symbol;

    class STICK_API PlacedSymbol : public Item
    {
    public:

        PlacedSymbol();

        PlacedSymbol(const brick::Entity & _e);

        Symbol symbol() const;
        

    private:

        Symbol m_symbol;
    };
}

#endif //PAPER_PLACEDSYMBOL_HPP
