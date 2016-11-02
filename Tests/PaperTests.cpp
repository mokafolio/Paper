//#include <Paper/Components.hpp>
#include <Paper/Document.hpp>
#include <Paper/Path.hpp>
#include <Stick/Test.hpp>
#include <Crunch/StringConversion.hpp>

using namespace stick;
using namespace brick;
using namespace paper;
using namespace crunch;

const Suite spec[] =
{
    SUITE("DOM Tests")
    {
        Hub hub;
        Document doc = createDocument(hub);

        Group grp = doc.createGroup("Group");
        EXPECT(doc.children().count() == 1);
        EXPECT(grp.name() == "Group");
        EXPECT(grp.parent() == doc);
        EXPECT(grp.document() == doc);

        Group grp2 = doc.createGroup("Group2");
        EXPECT(grp2.parent() == doc);
        EXPECT(doc.children().count() == 2);
        grp.addChild(grp2);
        EXPECT(grp2.parent() == grp);
        EXPECT(doc.children().count() == 1);

        Group grp3 = doc.createGroup("Group3");
        Group grp4 = doc.createGroup("Group4");
        grp.addChild(grp4);
        grp3.insertBelow(grp2);
        EXPECT(grp.children().count() == 3);
        EXPECT(grp.children()[0] == grp3);
        EXPECT(grp.children()[1] == grp2);
        EXPECT(grp.children()[2] == grp4);
        grp3.insertAbove(grp4);
        EXPECT(grp.children().count() == 3);
        EXPECT(grp.children()[0] == grp2);
        EXPECT(grp.children()[1] == grp4);
        EXPECT(grp.children()[2] == grp3);
        EXPECT(grp3.parent() == grp);

        grp2.insertAbove(grp4);
        EXPECT(grp.children()[0] == grp4);
        EXPECT(grp.children()[1] == grp2);
        EXPECT(grp.children()[2] == grp3);
        grp2.sendToFront();
        EXPECT(grp.children()[0] == grp4);
        EXPECT(grp.children()[1] == grp3);
        EXPECT(grp.children()[2] == grp2);
        grp2.sendToBack();
        EXPECT(grp.children()[0] == grp2);
        EXPECT(grp.children()[1] == grp4);
        EXPECT(grp.children()[2] == grp3);

        grp2.insertAbove(grp);
        EXPECT(grp.children().count() == 2);
        EXPECT(doc.children().count() == 2);
        EXPECT(grp2.parent() == doc);
        grp2.remove();
        EXPECT(!grp2.isValid());
        EXPECT(doc.children().count() == 1);
    },
    SUITE("Basic Path Tests")
    {
        Hub hub;
        Document doc = createDocument(hub);

        Path p = doc.createPath("test");
        p.addPoint(Vec2f(100.0f, 30.0f));
        p.addPoint(Vec2f(200.0f, 30.0f));
        EXPECT(p.segments().count() == 2);
        EXPECT(p.segments()[0]->position() == Vec2f(100.0f, 30.0f));
        EXPECT(p.segments()[1]->position() == Vec2f(200.0f, 30.0f));
        EXPECT(p.isPolygon());

        p.addSegment(Vec2f(150.0f, 150.0f), Vec2f(-5.0f, -3.0f), Vec2f(5.0f, 3.0f));
        EXPECT(p.segments().count() == 3);
        EXPECT(p.segments()[2]->position() == Vec2f(150.0f, 150.0f));
        EXPECT(p.segments()[2]->handleIn() == Vec2f(-5.0f, -3.0f));
        EXPECT(p.segments()[2]->handleOut() == Vec2f(5.0f, 3.0f));

        EXPECT(p.curves().count() == 2);
        EXPECT(!p.isPolygon());
        EXPECT(!p.isClosed());

        stick::DynamicArray<Vec2f> expectedCurves =
        {
            Vec2f(100.0f, 30.0f), Vec2f(0.0f, 0.0f), Vec2f(0.0f, 0.0f), Vec2f(200.0f, 30.0f),
            Vec2f(200.0f, 30.0f), Vec2f(0.0f, 0.0f), Vec2f(-5.0f, -3.0f), Vec2f(150.0f, 150.0f)
        };
        Size i = 0;
        for (const auto & c : p.curves())
        {
            EXPECT(c->positionOne() == expectedCurves[i++]);
            EXPECT(c->handleOne() == expectedCurves[i++]);
            EXPECT(c->handleTwo() == expectedCurves[i++]);
            EXPECT(c->positionTwo() == expectedCurves[i++]);
        }

        p.closePath();
        EXPECT(p.isClosed());
        EXPECT(p.curves().count() == 3);
        EXPECT(p.curves().last()->positionOne() == Vec2f(150.0f, 150.0f));
        EXPECT(p.curves().last()->handleOne() == Vec2f(5.0f, 3.0f));
        EXPECT(p.curves().last()->positionTwo() == Vec2f(100.0f, 30.0f));
    },
    SUITE("Attribute Tests")
    {
        Hub hub;
        Document doc = createDocument(hub);
        Path child = doc.createPath();
        EXPECT(!child.hasFill());
        EXPECT(!child.hasStroke());
        child.setFill(ColorRGBA(1.0f, 0.5f, 0.3f, 1.0f));
        child.setStroke(ColorRGBA(1.0f, 0.0f, 0.75f, 1.0f));
        EXPECT(reinterpretEntity<ColorPaint>(child.fill()).color() == ColorRGBA(1.0f, 0.5f, 0.3f, 1.0f));
        EXPECT(reinterpretEntity<ColorPaint>(child.stroke()).color() == ColorRGBA(1.0f, 0.0f, 0.75f, 1.0f));
        EXPECT(child.hasFill());
        EXPECT(child.hasStroke());
        Group grp = doc.createGroup();
        grp.addChild(child);
        grp.setFill(ColorRGBA(0.34f, 0.25f, 1.0f, 0.5f));
        EXPECT(reinterpretEntity<ColorPaint>(grp.fill()).color() == ColorRGBA(0.34f, 0.25f, 1.0f, 0.5f));
        EXPECT(reinterpretEntity<ColorPaint>(child.fill()).color() == ColorRGBA(0.34f, 0.25f, 1.0f, 0.5f));
        child.removeFill();
        EXPECT(child.hasFill());
        EXPECT(reinterpretEntity<ColorPaint>(child.fill()).color() == ColorRGBA(0.34f, 0.25f, 1.0f, 0.5f));
        grp.removeFill();
        EXPECT(!child.hasFill());
        EXPECT(!grp.hasFill());
    },
    SUITE("Path Length Tests")
    {
        Hub hub;
        Document doc = createDocument(hub);
        Path p = doc.createPath();
        p.addPoint(Vec2f(0.0f, 0.0f));
        p.addPoint(Vec2f(200.0f, 0.0f));
        p.addPoint(Vec2f(200.0f, 200.0f));
        EXPECT(isClose(p.length(), 400.0f));
    },
    SUITE("Path Bounds Tests")
    {
        Hub hub;
        Document doc = createDocument(hub);
        Path p = doc.createPath();
        p.addPoint(Vec2f(0.0f, 0.0f));
        p.addPoint(Vec2f(200.0f, 0.0f));
        p.addPoint(Vec2f(200.0f, 100.0f));

        const Rect & bounds = p.bounds();
        EXPECT(isClose(bounds.min(), Vec2f(0.0f)));
        EXPECT(isClose(bounds.width(), 200.0f));
        EXPECT(isClose(bounds.height(), 100.0f));

        p.addPoint(Vec2f(200.0f, 200.0f));
        const Rect & bounds2 = p.bounds();
        EXPECT(isClose(bounds2.min(), Vec2f(0.0f)));
        EXPECT(isClose(bounds2.width(), 200.0f));
        EXPECT(isClose(bounds2.height(), 200.0f));

        p.closePath();
        p.setStroke(ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
        p.setStrokeWidth(20.0);
        p.setStrokeJoin(StrokeJoin::Round);
        const Rect & strokeBounds = p.strokeBounds();
        EXPECT(isClose(strokeBounds.min(), Vec2f(-10.0f)));
        EXPECT(isClose(strokeBounds.width(), 220.0f));
        EXPECT(isClose(strokeBounds.height(), 220.0f));
    },
    SUITE("Transformed Path Bounds Tests")
    {
        Hub hub;
        Document doc = createDocument(hub);
        Path p = doc.createPath();
        p.addPoint(Vec2f(0.0f, 0.0f));
        p.addPoint(Vec2f(100.0f, 0.0f));
        p.addPoint(Vec2f(100.0f, 100.0f));
        p.addPoint(Vec2f(0.0f, 100.0f));
        p.closePath();
        EXPECT(isClose(p.position(), Vec2f(50.0f, 50.0f)));

        p.translateTransform(100, 150);
        EXPECT(isClose(p.position(), Vec2f(150.0f, 200.0f)));
        const Rect & bounds = p.bounds();
        EXPECT(isClose(bounds.min(), Vec2f(100.0f, 150.0f)));
        EXPECT(isClose(bounds.width(), 100.0f));
        EXPECT(isClose(bounds.height(), 100.0f));

        float diagonal = sqrt(100.0f * 100.0f + 100.0f * 100.0f);
        p.rotateTransform(Constants<Float>::pi() * 0.25);
        const Rect & bounds2 = p.bounds();
        EXPECT(isClose(bounds2.min(), Vec2f(150.0f, 200.0f) - Vec2f(diagonal * 0.5)));
        EXPECT(isClose(bounds2.width(), diagonal));
        EXPECT(isClose(bounds2.height(), diagonal));

        p.scaleTransform(2.0f);
        const Rect & bounds4 = p.bounds();
        EXPECT(isClose(bounds4.width(), diagonal * 2));
        EXPECT(isClose(bounds4.height(), diagonal * 2));

        p.setStroke(ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
        p.setStrokeWidth(20.0);
        p.setStrokeJoin(StrokeJoin::Round);
        p.setStrokeCap(StrokeCap::Round);
        const Rect & bounds5 = p.strokeBounds();
        EXPECT(isClose(bounds5.width(), diagonal * 2 + 40.0f));
        EXPECT(isClose(bounds5.height(), diagonal * 2 + 40.0f));
    },
    SUITE("Clone Tests")
    {
        Hub hub;
        Document doc = createDocument(hub);
        Group grp = doc.createGroup("grp");
        Path p = doc.createPath("yessaa");
        p.addPoint(Vec2f(100.0f, 30.0f));
        p.addPoint(Vec2f(200.0f, 30.0f));
        p.setStroke(ColorRGBA(1.0f, 0.5f, 0.75f, 0.75f));
        p.setStrokeCap(StrokeCap::Square);
        grp.addChild(p);
        grp.setFill(ColorRGBA(0.25f, 0.33f, 0.44f, 1.0f));
        grp.setStrokeWidth(10.0);
        grp.setStrokeJoin(StrokeJoin::Round);

        printf("WOOOP\n");
        Path p2 = p.clone();
        EXPECT(p2.name() == "yessaa");
        EXPECT(reinterpretEntity<ColorPaint>(p2.stroke()).color() == ColorRGBA(1.0f, 0.5f, 0.75f, 0.75f));
        EXPECT(p2.parent() == grp);
        EXPECT(p2.segments().count() == 2);
        EXPECT(p2.curves().count() == 1);
        EXPECT(p2.segments()[0]->position() == Vec2f(100.0f, 30.0f));
        EXPECT(p2.segments()[1]->position() == Vec2f(200.0f, 30.0f));
        p2.set<comps::Name>("p2");

        Group grp2 = grp.clone();
        EXPECT(grp2.name() == "grp");
        EXPECT(grp2.children().count() == 2);
        EXPECT(grp2.parent() == doc);
        EXPECT(grp2.children()[0].get<comps::Name>() == "yessaa");
        EXPECT(grp2.children()[1].get<comps::Name>() == "p2");

        EXPECT(doc.children().count() == 2);
        EXPECT(doc.children()[0] == grp);
        EXPECT(doc.children()[1] == grp2);
    },
    SUITE("SVG Export Tests")
    {
        //TODO: Turn this into an actual test
        Hub hub;
        Document doc = createDocument(hub);
        doc.translateTransform(Vec2f(100, 200));
        doc.scaleTransform(3.0);
        Path c = doc.createCircle(Vec2f(100, 100), 10);
        c.setFill(ColorRGBA(1.0, 1.0, 0.0, 1.0));
        auto res = doc.exportSVG();
        printf("%s\n", res.ensure().cString());
    },
    SUITE("SVG Import Tests")
    {
        {
            Hub hub;
            Document doc = createDocument(hub);
            String svg = "<svg width='100px' height='50px'><path d='M10 20 L100 20 100 120 Z'/></svg>";
            printf("SVG:\n%s\n", svg.cString());
            auto svgdata = doc.parseSVG(svg);
            EXPECT(svgdata.width() == 100);
            EXPECT(svgdata.height() == 50);
            EXPECT(svgdata.group().isValid());
            EXPECT(svgdata.group().children().count() == 1);
            EXPECT(Item(svgdata.group().children()[0]).itemType() == EntityType::Path);
            Path p = reinterpretEntity<Path>(svgdata.group().children()[0]);
            EXPECT(p.segments().count() == 3);
            EXPECT(isClose(p.segments()[0]->position(), Vec2f(10, 20)));
            EXPECT(isClose(p.segments()[1]->position(), Vec2f(100, 20)));
            EXPECT(isClose(p.segments()[2]->position(), Vec2f(100, 120)));
            EXPECT(p.isClosed());
        }
        {
            //TODO: test transforms on the group element
            Hub hub;
            Document doc = createDocument(hub);
            String svg = "<svg><g transform='translate(10, 10) rotate(30)'><path d='M10 20 L100 20'/><path d='M-30 30.0e4 L100 20'/></g></svg>";
            printf("SVG:\n%s\n", svg.cString());
            auto svgdata = doc.parseSVG(svg);
            EXPECT(!svgdata.error());
            EXPECT(svgdata.group().children().count() == 1);
            EXPECT(Item(svgdata.group().children()[0]).itemType() == EntityType::Group);
            Group grp = reinterpretEntity<Group>(svgdata.group().children()[0]);
            EXPECT(grp.children().count() == 2);
        }
        {
            //test basic colors and attribute import
            Hub hub;
            Document doc = createDocument(hub);
            String svg = "<svg><path d='M10 20 L100 20' fill='red' style='stroke: #333; stroke-width: 2px'/><circle cx='100' cy='200' r='20' fill='#4286f4' fill-rule='nonzero' stroke='black' stroke-miterlimit='33.5' stroke-dasharray='1, 2,3 4 5' stroke-dashoffset='20.33' vector-effect='non-scaling-stroke' stroke-linejoin='miter' stroke-linecap='round'/></svg>";
            printf("SVG:\n%s\n", svg.cString());
            auto svgdata = doc.parseSVG(svg);
            Path p = reinterpretEntity<Path>(svgdata.group().children()[0]);
            EXPECT(reinterpretEntity<ColorPaint>(p.fill()).color() == ColorRGBA(1, 0, 0, 1));
            auto s = reinterpretEntity<ColorPaint>(p.stroke()).color();
            EXPECT(isClose(s.r, 51.0f / 255.0f) && isClose(s.g, 51.0f / 255.0f) && isClose(s.b, 51.0f / 255.0f));
            EXPECT(p.strokeWidth() == 2.0f);
            Path p2 = reinterpretEntity<Path>(svgdata.group().children()[1]);
            auto & c2 = reinterpretEntity<ColorPaint>(p2.fill()).color();
            EXPECT(isClose(c2.r, 66.0f / 255.0f) && isClose(c2.g, 134.0f / 255.0f) && isClose(c2.b, 244.0f / 255.0f));
            EXPECT(reinterpretEntity<ColorPaint>(p2.stroke()).color() == ColorRGBA(0, 0, 0, 1));
            EXPECT(p2.isScalingStroke() == false);
            EXPECT(isClose(p2.miterLimit(), 33.5f));
            EXPECT(isClose(p2.dashOffset(), 20.33f));
            EXPECT(p2.windingRule() == WindingRule::NonZero);
            printf("DA COUNT %lu\n", p2.dashArray().count());
            EXPECT(p2.dashArray().count() == 5);
            EXPECT(p2.dashArray()[0] == 1);
            EXPECT(p2.dashArray()[1] == 2);
            EXPECT(p2.dashArray()[2] == 3);
            EXPECT(p2.dashArray()[3] == 4);
            EXPECT(p2.dashArray()[4] == 5);
            EXPECT(p2.strokeCap() == StrokeCap::Round);
            EXPECT(p2.strokeJoin() == StrokeJoin::Miter);
        }
    }
};

int main(int _argc, const char * _args[])
{
    return runTests(spec, _argc, _args);
}
