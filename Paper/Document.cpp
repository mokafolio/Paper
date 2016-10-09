#include <Paper/Document.hpp>
#include <Paper/Constants.hpp>
#include <Paper/SVG/SVGExport.hpp>
#include <Paper/SVG/SVGImport.hpp>
#include <Crunch/StringConversion.hpp>
#include <Stick/FileUtilities.hpp>

namespace paper
{
    using namespace stick;

    Document::Document()
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

    Symbol Document::createSymbol(const Item & _item)
    {
        if(_item.parent().isValid())
            _item.parent().removeChild(_item);
        brick::Hub * hub = get<comps::HubPointer>();
        Symbol ret;
        ret.assignEntity(hub->createEntity());
        ret.set<comps::Doc>(*this);
        ret.set<comps::ReferencedItem>(_item);
        return ret;
    }

    Group Document::createGroup(const String & _name)
    {
        brick::Hub * hub = get<comps::HubPointer>();
        Group ret = reinterpretItem<Group>(hub->createEntity());
        addDefaultComponents(ret);
        ret.set<comps::Name>(_name);
        ret.set<comps::ItemType>(EntityType::Group);
        ret.set<comps::ClippedFlag>(false);
        addChild(ret);
        return ret;
    }

    Path Document::createPath(const String & _name)
    {
        brick::Hub * hub = get<comps::HubPointer>();
        Path ret = reinterpretItem<Path>(hub->createEntity());
        addDefaultComponents(ret);
        ret.set<comps::Name>(_name);
        ret.set<comps::ItemType>(EntityType::Path);
        ret.set<comps::Segments>(SegmentArray());
        ret.set<comps::Curves>(CurveArray());
        ret.set<comps::ClosedFlag>(false);
        //ret.set<comps::Children>(ItemArray());
        ret.set<comps::PathLength>((comps::PathLengthData) {true, 0.0});
        addChild(ret);
        return ret;
    }

    Path Document::createEllipse(Vec2f _center, Vec2f _size, const String & _name)
    {
        static Float s_kappa = detail::PaperConstants::kappa();
        static Vec2f s_unitSegments[12] = { Vec2f(1, 0), Vec2f(0, -s_kappa), Vec2f(0, s_kappa),
                                            Vec2f(0, 1), Vec2f(s_kappa, 0), Vec2f(-s_kappa, 0),
                                            Vec2f(-1, 0), Vec2f(0, s_kappa), Vec2f(0, -s_kappa),
                                            Vec2f(0, -1), Vec2f(-s_kappa, 0), Vec2f(s_kappa, 0)
                                          };
        Path ret = createPath(_name);
        Vec2f rad = _size * 0.5;
        for (Int32 i = 0; i < 4; ++i)
        {
            ret.addSegment(s_unitSegments[i * 3] * rad + _center, s_unitSegments[i * 3 + 1] * rad, s_unitSegments[i * 3 + 2] * rad);
        }
        ret.closePath();
        return ret;
    }

    Path Document::createCircle(Vec2f _center, Float _radius, const String & _name)
    {
        return createEllipse(_center, Vec2f(_radius) * 2.0f, _name);
    }

    Path Document::createRectangle(Vec2f _from, Vec2f _to, const String & _name)
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

    Allocator & Document::allocator() const
    {
        STICK_ASSERT(isValid());
        STICK_ASSERT(hasComponent<comps::HubPointer>());
        return get<comps::HubPointer>()->allocator();
    }

    svg::SVGImportResult Document::parseSVG(const String & _svg, Size _dpi)
    {
        svg::SVGImport importer(*this);
        return importer.parse(_svg, _dpi);
    }

    TextResult Document::exportSVG() const
    {
        STICK_ASSERT(isValid());
        svg::SVGExport exporter;
        return exporter.exportDocument(*this);
    }

    Error Document::saveSVG(const URI & _uri) const
    {
        auto res = exportSVG();
        if (res)
            return saveTextFile(res.get(), _uri);
        return res.error();
    }

    brick::Hub & defaultHub()
    {
        static brick::Hub s_hub;
        return s_hub;
    }

    Document createDocument(brick::Hub & _hub, const String & _name)
    {
        Document doc = reinterpretItem<Document>(_hub.createEntity());
        addDefaultComponents(doc);
        doc.set<comps::Name>(_name);
        doc.set<comps::ItemType>(EntityType::Document);
        doc.set<comps::HubPointer>(&_hub);
        doc.set<comps::DocumentSize>(Vec2f(800, 600));
        return doc;
    }
}
