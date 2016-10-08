#include <Paper/PlacedSymbol.hpp>
#include <Paper/Symbol.hpp>

namespace paper
{
    PlacedSymbol::PlacedSymbol()
    {

    }

    Symbol PlacedSymbol::symbol() const
    {
        STICK_ASSERT(isValid());
        return get<comps::ReferencedSymbol>();
    }

    void PlacedSymbol::remove()
    {
        STICK_ASSERT(isValid());
        Symbol & s = get<comps::ReferencedSymbol>();
        if (s.isValid())
        {
            auto & ps = s.get<comps::PlacedSymbols>();
            auto it = stick::find(ps.begin(), ps.end(), *this);
            if (it != ps.end())
                ps.remove(it);
        }
        Item::remove();
    }
}
