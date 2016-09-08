#ifndef PAPER_SVG_SVGIMPORT_HPP
#define PAPER_SVG_SVGIMPORT_HPP

#include <Stick/Result.hpp>
#include <Stick/URI.hpp>
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

            void recursivelyImportNode(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, stick::Error & _error);

            void importGroup(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, stick::Error & _error);

            void importPath(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, stick::Error & _error);

            void importPolygon(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, stick::Error & _error);

            void importPolyline(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, bool _bIsPolygon, stick::Error & _error);

            void importCircle(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, stick::Error & _error);

            void importEllipse(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, stick::Error & _error);

            void importRectangle(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, stick::Error & _error);

            void importLine(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, stick::Error & _error);

            void importText();

            void importSymbol();

            void importUse();

        private:

            Document * m_document;
        };
    }
}

#endif //PAPER_SVG_SVGIMPORT_HPP
