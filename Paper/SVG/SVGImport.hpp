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

        class STICK_LOCAL SVGImport
        {
        public:

            SVGImport();

            SVGImport(Document & _doc);

            SVGImportResult parse(const stick::String & _svg, stick::Size _dpi = 72);

            Item recursivelyImportNode(const Shrub & _node, stick::Error & _error);

            Group importGroup(const Shrub & _node, stick::Error & _error);

            Path importPath(const Shrub & _node, stick::Error & _error);

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

            Float toPixels(Float _value, SVGUnits _units, Float _start = 0.0, Float _length = 1.0);

            SVGCoordinate parseCoordinate(const char * _str);

            Float coordinatePixels(const char * _str, Float _start = 0.0, Float _length = 1.0);

            stick::String::ConstIter parseNumbers(stick::String::ConstIter _it,
                                                  stick::String::ConstIter _end,
                                                  stick::DynamicArray<Float> & _outNumbers);

            void pushAttributes(const Shrub & _node, Item & _item);

            void popAttributes();


            Document * m_document;
            stick::Size m_dpi;
            stick::DynamicArray<SVGAttributes> m_attributeStack;
            stick::HashMap<stick::String, Item> m_defs;
        };
    }
}

#endif //PAPER_SVG_SVGIMPORT_HPP
