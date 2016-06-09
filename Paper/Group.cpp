#include <Paper/Group.hpp>
#include <Paper/Components.hpp>

namespace paper
{
    Group::Group()
    {
    }

    Group::Group(const brick::Entity & _e) :
        Item(_e)
    {

    }

    void Group::setClipped(bool _b)
    {
        set<comps::ClippedFlag>(_b);
    }

    bool Group::isClipped() const
    {
        return get<comps::ClippedFlag>();
    }

    Group Group::clone() const
    {
        return Group(Item::clone());
    }
}
