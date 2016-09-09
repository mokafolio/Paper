#include <Paper/SVG/SVGImport.hpp>
#include <Paper/Document.hpp>

namespace paper
{
    namespace svg
    {
        using namespace stick;
        using namespace crunch;

        struct NamedColor
        {
            String name;
            ColorRGB color;
        };

        static ColorRGB rgb(UInt8 _r, UInt8 _g, UInt8 _b)
        {
            return ColorRGB(_r / 255.0, _r / 255.0, _b / 255.0);
        }

        NamedColor svgColors[] =
        {
            {"aliceblue", rgb(240, 248, 255)},
            {"antiquewhite", rgb(250, 235, 215)},
            {"aqua", rgb(0, 255, 255)},
            {"aquamarine", rgb(127, 255, 212)},
            {"azure", rgb(240, 255, 255)},
            {"beige", rgb(245, 245, 220)},
            {"bisque", rgb(255, 228, 196)},
            {"black", rgb(0, 0, 0)},
            {"blanchedalmond", rgb(255, 235, 205)},
            {"blue", rgb(0, 0, 255)},
            {"blueviolet", rgb(138, 43, 226)},
            {"brown", rgb(165, 42, 42)},
            {"burlywood", rgb(222, 184, 135)},
            {"cadetblue", rgb(95, 158, 160)},
            {"chartreuse", rgb(127, 255, 0)},
            {"chocolate", rgb(210, 105, 30)},
            {"coral", rgb(255, 127, 80)},
            {"cornflowerblue", rgb(100, 149, 237)},
            {"cornsilk", rgb(255, 248, 220)},
            {"crimson", rgb(220, 20, 60)},
            {"cyan", rgb(0, 255, 255)},
            {"darkblue", rgb(0, 0, 139)},
            {"darkcyan", rgb(0, 139, 139)},
            {"darkgoldenrod", rgb(184, 134, 11)},
            {"darkgray", rgb(169, 169, 169)},
            {"darkgreen", rgb(0, 100, 0)},
            {"darkgrey", rgb(169, 169, 169)},
            {"darkkhaki", rgb(189, 183, 107)},
            {"darkmagenta", rgb(139, 0, 139)},
            {"darkolivegreen", rgb(85, 107, 47)},
            {"darkorange", rgb(255, 140, 0)},
            {"darkorchid", rgb(153, 50, 204)},
            {"darkred", rgb(139, 0, 0)},
            {"darksalmon", rgb(233, 150, 122)},
            {"darkseagreen", rgb(143, 188, 143)},
            {"darkslateblue", rgb(72, 61, 139)},
            {"darkslategray", rgb(47, 79, 79)},
            {"darkslategrey", rgb(47, 79, 79)},
            {"darkturquoise", rgb(0, 206, 209)},
            {"darkviolet", rgb(148, 0, 211)},
            {"deeppink", rgb(255, 20, 147)},
            {"deepskyblue", rgb(0, 191, 255)},
            {"dimgray", rgb(105, 105, 105)},
            {"dimgrey", rgb(105, 105, 105)},
            {"dodgerblue", rgb(30, 144, 255)},
            {"firebrick", rgb(178, 34, 34)},
            {"floralwhite", rgb(255, 250, 240)},
            {"forestgreen", rgb(34, 139, 34)},
            {"fuchsia", rgb(255, 0, 255)},
            {"gainsboro", rgb(220, 220, 220)},
            {"ghostwhite", rgb(248, 248, 255)},
            {"gold", rgb(255, 215, 0)},
            {"goldenrod", rgb(218, 165, 32)},
            {"gray", rgb(128, 128, 128)},
            {"grey", rgb(128, 128, 128)},
            {"green", rgb(0, 128, 0)},
            {"greenyellow", rgb(173, 255, 47)},
            {"honeydew", rgb(240, 255, 240)},
            {"hotpink", rgb(255, 105, 180)},
            {"indianred", rgb(205, 92, 92)},
            {"indigo", rgb(75, 0, 130)},
            {"ivory", rgb(255, 255, 240)},
            {"khaki", rgb(240, 230, 140)},
            {"lavender", rgb(230, 230, 250)},
            {"lavenderblush", rgb(255, 240, 245)},
            {"lawngreen", rgb(124, 252, 0)},
            {"lemonchiffon", rgb(255, 250, 205)},
            {"lightblue", rgb(173, 216, 230)},
            {"lightcoral", rgb(240, 128, 128)},
            {"lightcyan", rgb(224, 255, 255)},
            {"lightgoldenrodyellow", rgb(250, 250, 210)},
            {"lightgray", rgb(211, 211, 211)},
            {"lightgreen", rgb(144, 238, 144)},
            {"lightgrey", rgb(211, 211, 211)},
            {"lightpink", rgb(255, 182, 193)},
            {"lightsalmon", rgb(255, 160, 122)},
            {"lightseagreen", rgb(32, 178, 170)},
            {"lightskyblue", rgb(135, 206, 250)},
            {"lightslategray", rgb(119, 136, 153)},
            {"lightslategrey", rgb(119, 136, 153)},
            {"lightsteelblue", rgb(176, 196, 222)},
            {"lightyellow", rgb(255, 255, 224)},
            {"lime", rgb(0, 255, 0)},
            {"limegreen", rgb(50, 205, 50)},
            {"linen", rgb(250, 240, 230)},
            {"magenta", rgb(255, 0, 255)},
            {"maroon", rgb(128, 0, 0)},
            {"mediumaquamarine", rgb(102, 205, 170)},
            {"mediumblue", rgb(0, 0, 205)},
            {"mediumorchid", rgb(186, 85, 211)},
            {"mediumpurple", rgb(147, 112, 219)},
            {"mediumseagreen", rgb(60, 179, 113)},
            {"mediumslateblue", rgb(123, 104, 238)},
            {"mediumspringgreen", rgb(0, 250, 154)},
            {"mediumturquoise", rgb(72, 209, 204)},
            {"mediumvioletred", rgb(199, 21, 133)},
            {"midnightblue", rgb(25, 25, 112)},
            {"mintcream", rgb(245, 255, 250)},
            {"mistyrose", rgb(255, 228, 225)},
            {"moccasin", rgb(255, 228, 181)},
            {"navajowhite", rgb(255, 222, 173)},
            {"navy", rgb(0, 0, 128)},
            {"oldlace", rgb(253, 245, 230)},
            {"olive", rgb(128, 128, 0)},
            {"olivedrab", rgb(107, 142, 35)},
            {"orange", rgb(255, 165, 0)},
            {"orangered", rgb(255, 69, 0)},
            {"orchid", rgb(218, 112, 214)},
            {"palegoldenrod", rgb(238, 232, 170)},
            {"palegreen", rgb(152, 251, 152)},
            {"paleturquoise", rgb(175, 238, 238)},
            {"palevioletred", rgb(219, 112, 147)},
            {"papayawhip", rgb(255, 239, 213)},
            {"peachpuff", rgb(255, 218, 185)},
            {"peru", rgb(205, 133, 63)},
            {"pink", rgb(255, 192, 203)},
            {"plum", rgb(221, 160, 221)},
            {"powderblue", rgb(176, 224, 230)},
            {"purple", rgb(128, 0, 128)},
            {"red", rgb(255, 0, 0)},
            {"rosybrown", rgb(188, 143, 143)},
            {"royalblue", rgb(65, 105, 225)},
            {"saddlebrown", rgb(139, 69, 19)},
            {"salmon", rgb(250, 128, 114)},
            {"sandybrown", rgb(244, 164, 96)},
            {"seagreen", rgb(46, 139, 87)},
            {"seashell", rgb(255, 245, 238)},
            {"sienna", rgb(160, 82, 45)},
            {"silver", rgb(192, 192, 192)},
            {"skyblue", rgb(135, 206, 235)},
            {"slateblue", rgb(106, 90, 205)},
            {"slategray", rgb(112, 128, 144)},
            {"slategrey", rgb(112, 128, 144)},
            {"snow", rgb(255, 250, 250)},
            {"springgreen", rgb(0, 255, 127)},
            {"steelblue", rgb(70, 130, 180)},
            {"tan", rgb(210, 180, 140)},
            {"teal", rgb(0, 128, 128)},
            {"thistle", rgb(216, 191, 216)},
            {"tomato", rgb(255, 99, 71)},
            {"turquoise", rgb(64, 224, 208)},
            {"violet", rgb(238, 130, 238)},
            {"wheat", rgb(245, 222, 179)},
            {"white", rgb(255, 255, 255)},
            {"whitesmoke", rgb(245, 245, 245)},
            {"yellow", rgb(255, 255, 0)},
            {"yellowgreen", rgb(154, 205, 50)}
        };

        SVGImport::SVGImport() :
            m_document(nullptr)
        {

        }

        SVGImport::SVGImport(Document & _doc) :
            m_document(&_doc)
        {

        }

        GroupResult SVGImport::importFromString(const String & _svg)
        {
            m_defs.clear();
            auto shrubRes = parseXML(_svg);
            if (!shrubRes)
                return shrubRes.error();
            Shrub & svg = shrubRes.get();

            // auto maybe = svg.child("defs");
            // Shrub * defs = nullptr;
            // if (maybe) defs = &*maybe;

            Error err;
            Group ret = Group(recursivelyImportNode(svg, err));
            if (err) return err;
            return ret;
        }

        GroupResult SVGImport::importFromFile(const URI & _uri)
        {

        }

        template<class Comp, class Functor>
        static bool setComponentFromXMLAttrCB(const Shrub & _node, const String & _path, Item & _item, Functor _cb)
        {
            if (auto ch = _node.child(_path))
            {
                if ((*ch).valueHint() == ValueHint::XMLAttribute)
                {
                    //_item.set<Comp>((*ch).value<typename Comp::ValueType>());
                    _cb(_item, (*ch).value<typename Comp::ValueType>());
                    return true;
                }
            }
            return false;
        }

        template<class Comp>
        static bool setComponentFromXMLAttr(const Shrub & _node, const String & _path, Item & _item)
        {
            return setComponentFromXMLAttrCB<Comp>(_node, _path, _item, [](Item & _it, const typename Comp::ValueType & _val) { _it.set<Comp>(_val); });
        }

        void SVGImport::parseAttributes(const Shrub & _node, Item & _item)
        {
            //order is important to some degree
            //setComponentFromXMLAttrCB<comps::Fill>(_node, "fill", _item);
            // setComponentFromXMLAttr<comps::WindingRule>(_node, "fill-rule", _item);
            // setComponentFromXMLAttr<comps::Stroke>(_node, "stroke", _item);
            setComponentFromXMLAttr<comps::StrokeWidth>(_node, "stroke-width", _item);
            //setComponentFromXMLAttribute<comps::StrokeCap>(_node, "stroke-linecap", _item);


            if (auto ch = _node.child("id"))
            {
                if ((*ch).valueHint() == ValueHint::XMLAttribute)
                {
                    const String & name = (*ch).valueString();
                    _item.setName(name);
                    m_defs.insert(name, _item);
                }
            }
        }

        Item SVGImport::recursivelyImportNode(const Shrub & _node, Error & _error)
        {
            Item item;
            if (_node.name() == "svg")
            {
                item = importGroup(_node, _error);
            }
            else if (_node.name() == "g")
            {
                item = importGroup(_node, _error);
            }
            else if (_node.name() == "rect")
            {
                item = importRectangle(_node, _error);
            }
            else if (_node.name() == "circle")
            {
                item = importCircle(_node, _error);
            }
            else if (_node.name() == "ellipse")
            {
                item = importEllipse(_node, _error);
            }
            else if (_node.name() == "line")
            {
                item = importLine(_node, _error);
            }
            else if (_node.name() == "polyline")
            {
                item = importPolyline(_node, false, _error);
            }
            else if (_node.name() == "polygon")
            {
                item = importPolygon(_node, _error);
            }
            else if (_node.name() == "path")
            {
                item = importPath(_node, _error);
            }
            else
            {
                //?
            }

            if (item.isValid())
            {
                parseAttributes(_node, item);
            }

            return item;
        }

        Group SVGImport::importGroup(const Shrub & _node, Error & _error)
        {
            Group grp = m_document->createGroup();
            for (auto & child : _node)
            {
                if (child.valueHint() != ValueHint::XMLAttribute)
                {
                    Item item = recursivelyImportNode(child, _error);
                    if (_error) break;
                    else grp.addChild(item);
                }
            }
            return grp;
        }

        static bool isCommand(const char _c)
        {
            return  _c == 'M' || _c == 'm' ||
                    _c == 'L' || _c == 'l' ||
                    _c == 'H' || _c == 'h' ||
                    _c == 'V' || _c == 'v' ||
                    _c == 'C' || _c == 'c' ||
                    _c == 'S' || _c == 's' ||
                    _c == 'Q' || _c == 'q' ||
                    _c == 'T' || _c == 't' ||
                    _c == 'A' || _c == 'a' ||
                    _c == 'Z' || _c == 'z';
        }

        static String::ConstIter advanceToNextCommand(String::ConstIter _it, String::ConstIter _end)
        {
            while (_it != _end && !isCommand(*_it))
            {
                ++_it;
            }
            return _it;
        }

        static String::ConstIter skipWhitespace(String::ConstIter _it, String::ConstIter _end)
        {
            while (_it != _end && std::isspace(*_it))
                ++_it;
            return _it;
        }

        static String::ConstIter parseNumbers(String::ConstIter _it, String::ConstIter _end, DynamicArray<Float> & _outNumbers)
        {
            _outNumbers.clear();
            ++_it; //skip the command character
            _it = skipWhitespace(_it, _end);

            String number;
            while (_it != _end)
            {
                auto begin = _it;
                //find the end of the current number
                do
                {
                    ++_it;
                }
                while (_it != _end && !std::isspace(*_it) && *_it != ',' && !isCommand(*_it));

                number = String(begin, _it - 1);
                _outNumbers.append(toFloat64(number));

                //if we reached the next command or end, we are done
                if (isCommand(*_it) || _it == _end)
                    break;

                //otherwise skip the comma
                ++_it;
                //and whitespace
                _it = skipWhitespace(_it, _end);
            }

            //return advanceToNextCommand(_it, _end);
            return _it;
        }

        static Vec2f reflect(const Vec2f & _position, const Vec2f & _around)
        {
            return _around + (_around - _position);
        }

        Path SVGImport::importPath(const Shrub & _node, Error & _error)
        {
            auto maybe = _node.child("d");
            if (maybe)
            {
                const String & val = (*maybe).valueString();
                DynamicArray<Float> numbers;
                auto end = val.end();
                auto it = skipWhitespace(val.begin(), end);
                Path p = m_document->createPath();
                Path currentPath = p;
                Vec2f last;
                Vec2f lastHandle;
                do
                {
                    char cmd = *it;
                    it = parseNumbers(it, end, numbers);
                    if (cmd == 'M' || cmd == 'm')
                    {
                        if (currentPath.segments().count())
                        {
                            Path tmp = m_document->createPath();
                            currentPath.addChild(tmp);
                            currentPath = tmp;
                        }

                        bool bRelative = cmd == 'm';
                        for (int i = 0; i < numbers.count(); i += 2)
                        {
                            if (bRelative)
                                last = last + Vec2f(numbers[i], numbers[i + 1]);
                            else
                                last = Vec2f(numbers[i], numbers[i + 1]);
                            currentPath.addPoint(last);
                        }
                        lastHandle = last;
                    }
                    else if (cmd == 'L' || cmd == 'l')
                    {
                        bool bRelative = cmd == 'l';
                        for (int i = 0; i < numbers.count(); i += 2)
                        {
                            if (bRelative)
                                last = last + Vec2f(numbers[i], numbers[i + 1]);
                            else
                                last = Vec2f(numbers[i], numbers[i + 1]);
                            currentPath.addPoint(last);
                        }
                        lastHandle = last;
                    }
                    else if (cmd == 'H' || cmd == 'h' || cmd == 'V' || cmd == 'v')
                    {
                        bool bRelative = cmd == 'h' || cmd == 'v';
                        bool bVert = cmd == 'V' || cmd == 'v';
                        for (int i = 0; i < numbers.count(); ++i)
                        {
                            if (bVert)
                            {
                                if (bRelative)
                                    last.y = last.y + numbers[i];
                                else
                                    last.y = numbers[i];
                            }
                            else
                            {
                                if (bRelative)
                                    last.x = last.x + numbers[i];
                                else
                                    last.x = numbers[i];
                            }
                            currentPath.addPoint(last);
                        }
                        lastHandle = last;
                    }
                    else if (cmd == 'C' || cmd == 'c')
                    {
                        bool bRelative = cmd == 'c';
                        Vec2f start = last;
                        for (int i = 0; i < numbers.count(); i += 6)
                        {
                            if (!bRelative)
                            {
                                last = Vec2f(numbers[i + 4], numbers[i + 5]);
                                lastHandle = Vec2f(numbers[i + 2], numbers[i + 3]);
                                currentPath.cubicCurveTo(Vec2f(numbers[i], numbers[i + 1]),
                                                         lastHandle,
                                                         last);
                            }
                            else
                            {
                                last = start + Vec2f(numbers[i + 4], numbers[i + 5]);
                                lastHandle = Vec2f(start.x + numbers[i + 2], start.y + numbers[i + 3]);
                                currentPath.cubicCurveTo(Vec2f(start.x + numbers[i], start.y + numbers[i + 1]),
                                                         lastHandle,
                                                         last);
                            }
                        }
                    }
                    else if (cmd == 'S' || cmd == 's')
                    {
                        bool bRelative = cmd == 's';
                        Vec2f start = last;
                        for (int i = 0; i < numbers.count(); i += 4)
                        {

                            Vec2f nextLast, nextHandle;
                            if (!bRelative)
                            {
                                nextLast = Vec2f(numbers[i + 2], numbers[i + 3]);
                                nextHandle = Vec2f(numbers[i], numbers[i + 1]);
                            }
                            else
                            {
                                nextLast = start + Vec2f(numbers[i + 2], numbers[i + 3]);
                                nextHandle = start + Vec2f(numbers[i], numbers[i + 1]);
                            }
                            currentPath.cubicCurveTo(reflect(lastHandle, last), nextHandle, nextLast);
                            lastHandle = nextHandle;
                            last = nextLast;
                        }
                    }
                    else if (cmd == 'Q' || cmd == 'q')
                    {
                        bool bRelative = cmd == 'q';
                        Vec2f start = last;
                        for (int i = 0; i < numbers.count(); i += 4)
                        {
                            if (!bRelative)
                            {
                                last = Vec2f(numbers[i + 2], numbers[i + 3]);
                                lastHandle = Vec2f(numbers[i], numbers[i + 1]);
                            }
                            else
                            {
                                last = start + Vec2f(numbers[i + 2], numbers[i + 3]);
                                lastHandle = start + Vec2f(numbers[i], numbers[i + 1]);
                            }
                            currentPath.quadraticCurveTo(lastHandle, last);
                        }
                    }
                    else if (cmd == 'T' || cmd == 't')
                    {
                        bool bRelative = cmd == 't';
                        Vec2f start = last;
                        for (int i = 0; i < numbers.count(); i += 2)
                        {
                            Vec2f nextLast = !bRelative ? Vec2f(numbers[i], numbers[i + 1]) : start + Vec2f(numbers[i], numbers[i + 1]);
                            lastHandle = reflect(lastHandle, last);
                            currentPath.quadraticCurveTo(lastHandle, nextLast);
                            last = nextLast;
                        }
                    }
                    else if (cmd == 'A' || cmd == 'a')
                    {
                        bool bRelative = cmd == 'a';
                        for (int i = 0; i < numbers.count(); i += 7)
                        {
                            last = !bRelative ? Vec2f(numbers[i + 5], numbers[i + 6]) : last + Vec2f(numbers[i + 5], numbers[i + 6]);
                            currentPath.arcTo(last, Vec2f(numbers[i], numbers[i + 1]),
                                              crunch::toRadians(numbers[i + 2]), (bool)numbers[i + 3], (bool)numbers[i + 4]);
                            lastHandle = currentPath.segments().last().handleOutAbsolute();
                        }
                    }
                    else if (cmd == 'Z' || cmd == 'z')
                    {
                        currentPath.closePath();
                        last = currentPath.segments().last().position();
                        lastHandle = currentPath.segments().last().handleOutAbsolute();
                    }
                    else
                    {
                        //unknown command??
                        //warning? ignore silently?
                    }
                }
                while (it != end);

                //return the path
                return p;
            }
            else
            {
                //TODO: Error
            }
            return Path();
        }

        Path SVGImport::importPolygon(const Shrub & _node, Error & _error)
        {

        }

        Path SVGImport::importPolyline(const Shrub & _node, bool _bIsPolygon, Error & _error)
        {

        }

        Path SVGImport::importCircle(const Shrub & _node, Error & _error)
        {

        }

        Path SVGImport::importEllipse(const Shrub & _node, Error & _error)
        {

        }

        Path SVGImport::importRectangle(const Shrub & _node, Error & _error)
        {

        }

        Path SVGImport::importLine(const Shrub & _node, Error & _error)
        {

        }

        // void SVGImport::importText()
        // {

        // }

        // void SVGImport::importSymbol()
        // {

        // }

        // void SVGImport::importUse()
        // {

        // }
    }
}