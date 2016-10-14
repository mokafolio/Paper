#ifndef PAPER_SVG_SVGIMPORT_HPP
#define PAPER_SVG_SVGIMPORT_HPP

#include <Stick/Result.hpp>
#include <Stick/URI.hpp>
#include <Stick/HashMap.hpp>
#include <Stick/DynamicArray.hpp>
#include <Scrub/Shrub.hpp>
#include <Paper/BasicTypes.hpp>
#include <Paper/Constants.hpp>
#include <Paper/Group.hpp>
#include <Paper/SVG/SVGImportResult.hpp>

namespace paper
{
    class Document;
    class Path;

    namespace svg
    {
        using namespace scrub;

        enum class SVGUnits
        {
            EM,
            EX,
            PX,
            PT,
            PC,
            CM,
            MM,
            IN,
            Percent,
            User
        };

        struct STICK_LOCAL SVGAttributes
        {
            ColorRGBA fillColor;
            WindingRule windingRule;
            ColorRGBA strokeColor;
            Float strokeWidth;
            StrokeCap strokeCap;
            StrokeJoin strokeJoin;
            bool bScalingStroke;
            Float miterLimit;
            stick::DynamicArray<Float> dashArray;
            Float dashOffset;
            // since we dont support text yet, we only care
            // about font size for em/ex calculations
            Float fontSize;
        };

        struct STICK_LOCAL SVGCoordinate
        {
            SVGUnits units;
            Float value;
        };

        struct STICK_LOCAL SVGView
        {
            Rect rectangle;
            Mat3f viewBoxTransform;
        };

        class STICK_LOCAL SVGImport
        {
        public:

            SVGImport();

            SVGImport(Document & _doc);

            SVGImportResult parse(const stick::String & _svg, stick::Size _dpi = 72);

            Item recursivelyImportNode(const Shrub & _node, const Shrub & _rootNode, stick::Error & _error);

            Group importGroup(const Shrub & _node, const Shrub & _rootNode, bool _bSVGNode, stick::Error & _error);

            Path importClipPath(const Shrub & _node, const Shrub & _rootNode, stick::Error & _error);

            Path importPath(const Shrub & _node, const Shrub & _rootNode, stick::Error & _error);

            Path importPolyline(const Shrub & _node, const Shrub & _rootNode, bool _bIsPolygon, stick::Error & _error);

            Path importCircle(const Shrub & _node, const Shrub & _rootNode, stick::Error & _error);

            Path importEllipse(const Shrub & _node, const Shrub & _rootNode, stick::Error & _error);

            Path importRectangle(const Shrub & _node, const Shrub & _rootNode, stick::Error & _error);

            Path importLine(const Shrub & _node, const Shrub & _rootNode, stick::Error & _error);

            // // Not supported yet
            // void importText();
            // // Not supported yet
            // void importSymbol();
            // // Not supported yet
            // void importUse();

        private:

            Float toPixels(Float _value, SVGUnits _units, Float _start = 0.0, Float _length = 1.0);

            SVGCoordinate parseCoordinate(const char * _str);

            Float coordinatePixels(const char * _str, Float _start = 0.0, Float _length = 1.0);

            void parseAttribute(const stick::String & _name, const stick::String & _value,
                                SVGAttributes & _attr, Item & _item);

            void parseStyle(const stick::String & _style, SVGAttributes & _attr, Item & _item);

            void pushAttributes(const Shrub & _node, const Shrub & _rootNode, Item & _item);

            void popAttributes();


            Document * m_document;
            stick::Size m_dpi;
            stick::DynamicArray<SVGAttributes> m_attributeStack;
            stick::DynamicArray<Rect> m_viewStack;
            // for items that are only temporary in the dom and should
            // be removed at the end of the import (i.e. clipping masks, defs nodes etc.)
            stick::DynamicArray<Item> m_tmpItems;
            stick::HashMap<stick::String, Item> m_namedItems;
        };
    }
}

#endif //PAPER_SVG_SVGIMPORT_HPP
