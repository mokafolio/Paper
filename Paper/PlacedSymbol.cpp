#include <Paper/PlacedSymbol.hpp>

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
}
