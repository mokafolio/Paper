#ifndef PAPER_SVG_SVGIMPORT_HPP
#define PAPER_SVG_SVGIMPORT_HPP

#include <Stick/Result.hpp>
#include <Stick/URI.hpp>
#include <Stick/HashMap.hpp>
#include <Scrub/Shrub.hpp>
#include <Paper/BasicTypes.hpp>

namespace paper
{
    class Document;
    class Item;
    class Group;
    class Path;
    class Curve;

    namespace svg
    {
        using namespace scrub;

        using GroupResult = stick::Result<Group>;

        class STICK_LOCAL SVGImport
        {
        public:

            SVGImport();

            SVGImport(Document & _doc);

            GroupResult importFromString(const stick::String & _svg);

            GroupResult importFromFile(const stick::URI & _uri);

            Item recursivelyImportNode(const Shrub & _node, stick::Error & _error);

            Group importGroup(const Shrub & _node, stick::Error & _error);

            Path importPath(const Shrub & _node, stick::Error & _error);

            Path importPolygon(const Shrub & _node, stick::Error & _error);

            Path importPolyline(const Shrub & _node, bool _bIsPolygon, stick::Error & _error);

            Path importCircle(const Shrub & _node, stick::Error & _error);

            Path importEllipse(const Shrub & _node, stick::Error & _error);

            Path importRectangle(const Shrub & _node, stick::Error & _error);

            Path importLine(const Shrub & _node, stick::Error & _error);

            // // Not supported yet
            // void importText();
            // // Not supported yet
            // void importSymbol();
            // // Not supported yet
            // void importUse();

        private:

            void parseAttributes(const Shrub & _node, Item & _item);

            Document * m_document;
            stick::HashMap<stick::String, Item> m_defs;
        };
    }
}

#endif //PAPER_SVG_SVGIMPORT_HPP
