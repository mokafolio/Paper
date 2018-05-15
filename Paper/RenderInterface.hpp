#ifndef PAPER_RENDERINTERFACE_HPP
#define PAPER_RENDERINTERFACE_HPP

#include <Paper/Document.hpp>
#include <Stick/Error.hpp>

namespace paper
{
    class Document;
    class Path;
    class Item;

    class STICK_API RenderInterface
    {
    public:

        RenderInterface();

        virtual ~RenderInterface();


        virtual stick::Error init(Document _doc) = 0;

        stick::Error draw();

        virtual void setViewport(Float _x, Float _y,
                                 Float _widthInPixels, Float _heightInPixels) = 0;

        virtual void setProjection(const Mat4f & _projection) = 0;

        virtual void reserveItems(stick::Size _count) = 0;


    protected:

        //these have to be implemented
        virtual stick::Error drawPath(Path _path, const Mat3f & _transform) = 0;
        virtual stick::Error beginClipping(Path _clippingPath, const Mat3f & _transform) = 0;
        virtual stick::Error endClipping() = 0;

        //these can be implemented
        virtual stick::Error prepareDrawing() { return stick::Error(); }
        virtual stick::Error finishDrawing() { return stick::Error(); }

        stick::Error drawChildren(Item _item, const Mat3f * _transform);
        stick::Error drawItem(Item _item, const Mat3f * _transform);

        Document m_document;
    };
}

#endif //PAPER_RENDERINTERFACE_HPP
