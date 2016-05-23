#ifndef PAPER_DOCUMENT_HPP
#define PAPER_DOCUMENT_HPP

#include <Stick/String.hpp>
#include <Stick/DynamicArray.hpp>
#include <Stick/Result.hpp>
#include <Stick/URI.hpp>
#include <Brick/Hub.hpp>

#include <Paper/Components.hpp>
#include <Paper/Path.hpp>
#include <Paper/Segment.hpp>
#include <Paper/Curve.hpp>
#include <Paper/Group.hpp>

namespace paper
{
    class STICK_API Document : public Item
    {
    public:

        Document();

        Document(const brick::Entity & _e);

        Group createGroup(const stick::String & _name = "");

        Path createPath(const stick::String & _name = "");

        Path createEllipse(Vec2f _center, Vec2f _size, const stick::String & _name = "");

        Path createCircle(Vec2f _center, Float _radius, const stick::String & _name = "");

        Path createRectangle(Vec2f _from, Vec2f _to, const stick::String & _name = "");

        void setSize(Float _width, Float _height);

        Float width() const;

        Float height() const;

        const Vec2f size() const;

        brick::Hub & hub();
        
        stick::Allocator & allocator() const;

        stick::TextResult exportSVG() const;

        stick::Error saveSVG(const stick::URI & _uri) const;
    };

    STICK_API brick::Hub & defaultHub();

    STICK_API Document createDocument(brick::Hub & _hub = defaultHub(), const stick::String & _name = "");
}

#endif //PAPER_DOCUMENT_HPP
