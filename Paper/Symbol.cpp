#include <Paper/Symbol.hpp>
#include <Paper/PlacedSymbol.hpp>
#include <Paper/Document.hpp>

namespace paper
{
    Symbol::Symbol()
    {

    }

    void Symbol::assignEntity(const brick::Entity & _e)
    {
        static_cast<brick::Entity *>(this)->operator=(_e);
    }

    PlacedSymbol Symbol::place(const Vec2f & _position)
    {
        STICK_ASSERT(isValid());
        STICK_ASSERT(hasComponent<comps::Doc>());
        Document doc = get<comps::Doc>();
        PlacedSymbol ret = brick::reinterpretEntity<PlacedSymbol>(doc.hub().createEntity());
        ret.set<comps::ReferencedSymbol>(*this);
        ret.set<comps::ItemType>(EntityType::PlacedSymbol);
        ret.set<comps::StrokeBounds>(comps::BoundsData{true, Rect(0, 0, 0, 0)});
        ret.set<comps::Bounds>(comps::BoundsData{true, Rect(0, 0, 0, 0)});
        ret.set<comps::LocalBounds>(comps::BoundsData{true, Rect(0, 0, 0, 0)});
        ret.set<comps::HandleBounds>(comps::BoundsData{true, Rect(0, 0, 0, 0)});

        if (!hasComponent<comps::PlacedSymbols>())
            set<comps::PlacedSymbols>(PlacedSymbolArray());

        PlacedSymbolArray & ps = get<comps::PlacedSymbols>();
        ps.append(ret);

        doc.addChild(ret);
        ret.translateTransform(_position);

        return ret;
    }

    void Symbol::remove()
    {
        // remove all placed symbols that use this symbol
        for (PlacedSymbol & s : get<comps::PlacedSymbols>())
        {
            s.remove();
        }
        //free all components and invalidate this entity handle
        destroy();
    }

    Item Symbol::item() const
    {
        if (hasComponent<comps::ReferencedItem>())
            return get<comps::ReferencedItem>();
        return Item();
    }
}
