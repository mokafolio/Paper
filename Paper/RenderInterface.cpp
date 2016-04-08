#include <Paper/RenderInterface.hpp>
#include <Paper/Document.hpp>
#include <Paper/Path.hpp>

namespace paper
{
    RenderInterface::RenderInterface()
    {

    }

    RenderInterface::RenderInterface(const Document & _doc) :
        m_document(_doc)
    {

    }

    RenderInterface::~RenderInterface()
    {

    }

    stick::Error RenderInterface::draw()
    {
        STICK_ASSERT(m_document.isValid());
        stick::Error ret = prepareDrawing();
        if(ret) return ret;
        ret = drawChildren(m_document);
        if(ret) return ret;
        ret = finishDrawing();
        return ret;
    }

    stick::Error RenderInterface::drawChildren(const Item & _item)
    {
        const auto & children = _item.children();
        stick::Error err;
        for (const auto & c : children)
        {
            err = drawItem(c);
            if(err) return err;
        }
        return err;
    }

    stick::Error RenderInterface::drawItem(const Item & _item)
    {
        auto et = _item.get<comps::ItemType>();
        stick::Error ret;
        if (et == EntityType::Group)
        {
            Group grp(_item);
            if (grp.isClipped())
            {
                const auto & c2 = grp.children();
                STICK_ASSERT(c2.first().get<comps::ItemType>() == EntityType::Path);
                ret = beginClipping(c2.first());
                if(ret) return ret;
                auto it = c2.begin() + 1;
                for(; it != c2.end(); ++it)
                {
                    ret = drawItem(Item(*it));
                    if(ret) return ret;
                }
                ret = endClipping(c2.first());
            }
            else
                drawChildren(_item);
        }
        else if (et == EntityType::Path)
        {
            ret = drawPath(Path(_item));
        }
        return ret;
    }
}
