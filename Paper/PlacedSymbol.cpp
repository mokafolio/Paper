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
}
