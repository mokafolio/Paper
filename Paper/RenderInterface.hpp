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

        RenderInterface(const Document & _doc);

        virtual ~RenderInterface();


        stick::Error draw();

        virtual void setViewport(Float _widthInPixels, Float _heightInPixels) = 0;

        virtual void setTransform(const Mat3f & _transform) = 0;

        virtual void setProjection(const Mat4f & _projection) = 0;

        virtual void reserveItems(stick::Size _count) = 0;


    protected:

        //these have to be implemented
        virtual stick::Error drawPath(const Path & _path, const Mat3f * _transform) = 0;
        virtual stick::Error beginClipping(const Path & _clippingPath, const Mat3f * _transform) = 0;
        virtual stick::Error endClipping(const Path & _clippingPath, const Mat3f * _transform) = 0;

        //these can be implemented
        virtual stick::Error prepareDrawing() { return stick::Error(); }
        virtual stick::Error finishDrawing() { return stick::Error(); }

        stick::Error drawChildren(const Item & _item, const Mat3f * _transform);
        stick::Error drawItem(const Item & _item, const Mat3f * _transform);

        Document m_document;
    };
}

#endif //PAPER_RENDERINTERFACE_HPP
