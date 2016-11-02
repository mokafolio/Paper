#include <Paper/RenderInterface.hpp>
#include <Paper/Document.hpp>
#include <Paper/Path.hpp>
#include <Paper/PlacedSymbol.hpp>

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
        if (ret) return ret;
        ret = drawChildren(m_document, nullptr);
        if (ret) return ret;
        ret = finishDrawing();
        return ret;
    }

    stick::Error RenderInterface::drawChildren(const Item & _item, const Mat3f * _transform)
    {
        const auto & children = _item.children();
        stick::Error err;
        for (const auto & c : children)
        {
            err = drawItem(c, _transform);
            if (err) return err;
        }
        return err;
    }

    stick::Error RenderInterface::drawItem(const Item & _item, const Mat3f * _transform)
    {
        auto et = _item.get<comps::ItemType>();
        stick::Error ret;

        Mat3f tmp;
        if (_transform)
            tmp = _item.absoluteTransform() * *_transform;

        if (et == EntityType::Group)
        {
            Group grp = brick::reinterpretEntity<Group>(_item);
            if (!grp.isVisible())
                return ret;

            if (grp.isClipped())
            {
                const auto & c2 = grp.children();
                STICK_ASSERT(c2.first().get<comps::ItemType>() == EntityType::Path);
                Path mask = brick::reinterpretEntity<Path>(c2.first());
                Mat3f tmp2;
                if (_transform)
                    tmp2 = mask.absoluteTransform() * tmp;
                ret = beginClipping(mask, _transform ? &tmp2 : nullptr);
                if (ret) return ret;
                auto it = c2.begin() + 1;
                for (; it != c2.end(); ++it)
                {
                    ret = drawItem(Item(*it), _transform);
                    if (ret) return ret;
                }
                ret = endClipping(mask, _transform ? &tmp2 : nullptr);
            }
            else
                drawChildren(_item, _transform);
        }
        else if (et == EntityType::Path)
        {
            Path p = brick::reinterpretEntity<Path>(_item);
            if (p.isVisible() && p.segmentArray().count() > 1)
            {
                ret = drawPath(p, _transform ? &tmp : nullptr);
            }
        }
        else if (et == EntityType::PlacedSymbol)
        {
            PlacedSymbol ps = brick::reinterpretEntity<PlacedSymbol>(_item);
            drawItem(ps.symbol().item(), _transform ? &tmp : &ps.absoluteTransform());
        }
        return ret;
    }
}
