#ifndef PAPER_GROUP_HPP
#define PAPER_GROUP_HPP

#include <Paper/Item.hpp>
#include <Paper/Constants.hpp>

namespace paper
{
    class Document;

    class Group : public Item
    {
        friend class Item;

    public:

        static constexpr EntityType itemType = EntityType::Group;


        Group();

        Group(const brick::Entity & _e);

        void setClipped(bool _b);
        
        bool isClipped() const;
    };
}

#endif //PAPER_GROUP_HPP
