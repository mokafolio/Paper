#ifndef PAPER_SVG_SVGEXPORT_HPP
#define PAPER_SVG_SVGEXPORT_HPP

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

        class STICK_LOCAL SVGExport
        {
        public:

            SVGExport();

            stick::TextResult exportDocument(const Document & _document);

            void addToDefsNode(Shrub & _node);

            void exportItem(const Item & _item, Shrub & _parentTreeNode, bool _bIsClipMask);

            void setTransform(const Item & _item, Shrub & _node);

            void exportGroup(const Group & _group, Shrub & _parentTreeNode);

            void exportCurveData(const Path & _path, Shrub & _parentTreeNode, Shrub *& _pn);

            void exportPath(const Path & _path, Shrub & _parentTreeNode, bool _bIsClipPath, bool _bMatchShape);

            void applyStyle(const Item & _item, Shrub & _node);



            Shrub m_tree;
            stick::Size m_clipMaskID;
        };
    }
}

#endif //PAPER_SVG_SVGEXPORT_HPP
