#include <Paper/Symbol.hpp>
#include <Paper/PlacedSymbol.hpp>
#include <Paper/Document.hpp>

namespace paper
{
    Symbol::Symbol()
    {

    }

    Symbol::Symbol(const brick::Entity & _e) :
    brick::Entity(_e)
    {

    }

    void Symbol::assignEntity(const brick::Entity & _e)
    {
        static_cast<brick::Entity*>(this)->operator=(_e);
    }

    PlacedSymbol Symbol::place(const Vec2f & _position)
    {
        STICK_ASSERT(isValid());
        STICK_ASSERT(hasComponent<comps::Doc>());
        Document doc = get<comps::Doc>();
        PlacedSymbol ret = reinterpretItem<PlacedSymbol>(doc.hub().createEntity());
        ret.set<comps::ReferredSymbol>(*this);
        ret.translateTransform(_position);
        doc.addChild(ret);
        return ret;
    }

    void Symbol::remove()
    {
        // remove all placed symbols that use this symbol
        for(PlacedSymbol & s : get<comps::PlacedSymbols>())
        {
            s.remove();
        }
        //free all components and invalidate this entity handle
        destroy();
    }

    Item Symbol::item() const
    {
        if(hasComponent<comps::ReferencedItem>())
            return get<comps::ReferencedItem>();
        return Item();
    }
}
