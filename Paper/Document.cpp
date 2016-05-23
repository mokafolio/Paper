#include <Paper/Document.hpp>
#include <Paper/Constants.hpp>
#include <Paper/SVG/SVGExport.hpp>
#include <Crunch/StringConversion.hpp>
#include <Stick/FileUtilities.hpp>

namespace paper
{
    Document::Document()
    {

    }

    Document::Document(const brick::Entity & _e) :
        Item(_e)
    {

    }

    void addDefaultComponents(brick::Entity & _e)
    {
        //TODO: Only add these when they are used to save memory
        /*_e.set<comps::Transform>(Mat3f::identity());
        _e.set<comps::AbsoluteTransform>(Mat3f::identity());
        _e.set<comps::AbsoluteTransformDirtyFlag>(true);*/
        _e.set<comps::StrokeBounds>(comps::BoundsData{true, Rect(0, 0, 0, 0)});
        _e.set<comps::Bounds>(comps::BoundsData{true, Rect(0, 0, 0, 0)});
        _e.set<comps::LocalBounds>(comps::BoundsData{true, Rect(0, 0, 0, 0)});
        _e.set<comps::HandleBounds>(comps::BoundsData{true, Rect(0, 0, 0, 0)});
    }

    Group Document::createGroup(const stick::String & _name)
    {
        brick::Hub * hub = get<comps::HubPointer>();
        Group ret = hub->createEntity();
        addDefaultComponents(ret);
        ret.set<comps::Name>(_name);
        ret.set<comps::ItemType>(EntityType::Group);
        ret.set<comps::Children>(EntityArray());
        ret.set<comps::ClippedFlag>(false);
        addChild(ret);
        return ret;
    }

    Path Document::createPath(const stick::String & _name)
    {
        brick::Hub * hub = get<comps::HubPointer>();
        Path ret = hub->createEntity();
        addDefaultComponents(ret);
        ret.set<comps::Name>(_name);
        ret.set<comps::ItemType>(EntityType::Path);
        ret.set<comps::Segments>(SegmentArray());
        ret.set<comps::Curves>(CurveArray());
        ret.set<comps::ClosedFlag>(false);
        ret.set<comps::Children>(EntityArray());
        ret.set<comps::PathLength>((comps::PathLengthData) {true, 0.0});
        addChild(ret);
        return ret;
    }

    Path Document::createEllipse(Vec2f _center, Vec2f _size, const stick::String & _name)
    {
        static Float s_kappa = detail::PaperConstants::kappa();
        static Vec2f s_unitSegments[12] = { Vec2f(1, 0), Vec2f(0, -s_kappa), Vec2f(0, s_kappa),
                                            Vec2f(0, 1), Vec2f(s_kappa, 0), Vec2f(-s_kappa, 0),
                                            Vec2f(-1, 0), Vec2f(0, s_kappa), Vec2f(0, -s_kappa),
                                            Vec2f(0, -1), Vec2f(-s_kappa, 0), Vec2f(s_kappa, 0)
                                          };
        Path ret = createPath(_name);
        Vec2f rad = _size * 0.5;
        for (stick::Int32 i = 0; i < 4; ++i)
        {
            ret.addSegment(s_unitSegments[i * 3] * rad + _center, s_unitSegments[i * 3 + 1] * rad, s_unitSegments[i * 3 + 2] * rad);
        }
        ret.closePath();
        return ret;
    }

    Path Document::createCircle(Vec2f _center, Float _radius, const stick::String & _name)
    {
        return createEllipse(_center, Vec2f(_radius) * 2.0f, _name);
    }

    Path Document::createRectangle(Vec2f _from, Vec2f _to, const stick::String & _name)
    {
        Path ret = createPath(_name);

        ret.addPoint(_from);
        ret.addPoint(Vec2f(_to.x, _from.y));
        ret.addPoint(_to);
        ret.addPoint(Vec2f(_from.x, _to.y));
        ret.closePath();

        return ret;
    }

    void Document::setSize(Float _width, Float _height)
    {
        set<comps::DocumentSize>(Vec2f(_width, _height));
    }

    Float Document::width() const
    {
        return size().x;
    }

    Float Document::height() const
    {
        return size().y;
    }

    const Vec2f Document::size() const
    {
        return get<comps::DocumentSize>();
    }

    brick::Hub & Document::hub()
    {
        STICK_ASSERT(isValid());
        STICK_ASSERT(hasComponent<comps::HubPointer>());
        return *get<comps::HubPointer>();
    }

    stick::Allocator & Document::allocator() const
    {
        STICK_ASSERT(isValid());
        STICK_ASSERT(hasComponent<comps::HubPointer>());
        return get<comps::HubPointer>()->allocator();
    }

    stick::TextResult Document::exportSVG() const
    {
        STICK_ASSERT(isValid());
        svg::SVGExport exporter;
        return exporter.exportDocument(*this);
    }

    stick::Error Document::saveSVG(const stick::URI & _uri) const
    {
        auto res = exportSVG();
        if (res)
            return saveTextFile(res.text(), _uri);
        return res.error();
    }

    brick::Hub & defaultHub()
    {
        static brick::Hub s_hub;
        return s_hub;
    }

    Document createDocument(brick::Hub & _hub, const stick::String & _name)
    {
        Document doc = _hub.createEntity();
        addDefaultComponents(doc);
        doc.set<comps::Name>(_name);
        doc.set<comps::ItemType>(EntityType::Document);
        doc.set<comps::Children>(EntityArray());
        doc.set<comps::HubPointer>(&_hub);
        doc.set<comps::DocumentSize>(Vec2f(800, 600));
        return doc;
    }
}
