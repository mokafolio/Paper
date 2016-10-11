#include <Stick/HashMap.hpp>
#include <Paper/SVG/SVGImport.hpp>
#include <Paper/Document.hpp>

namespace paper
{
    namespace svg
    {
        using namespace stick;
        using namespace crunch;

        namespace detail
        {
            static ColorRGB rgb(UInt8 _r, UInt8 _g, UInt8 _b)
            {
                return ColorRGB(_r / 255.0, _g / 255.0, _b / 255.0);
            }

            HashMap<String, ColorRGB> svgColors =
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

            static String::ConstIter skipWhitespaceAndCommas(String::ConstIter _it, String::ConstIter _end)
            {
                while (_it != _end && (std::isspace(*_it) || *_it == ','))
                    ++_it;
                return _it;
            }

            template<class F>
            static String::ConstIter parseNumbers(String::ConstIter _it, String::ConstIter _end, F _endCondition, DynamicArray<Float> & _outNumbers)
            {
                _outNumbers.clear();
                ++_it; //skip the command character
                _it = skipWhitespaceAndCommas(_it, _end);

                Float value;
                while (_it != _end)
                {
                    std::sscanf(_it, "%f", &value);
                    String tmp(_it, _end);
                    printf("VALUE %f %s\n", value, tmp.cString());
                    _outNumbers.append(value);

                    //skip the sign part
                    if (*_it == '+' || *_it == '-')
                        ++_it;
                    //skip integer part
                    while (_it != _end && std::isdigit(*_it)) ++_it;
                    //skip fractional part
                    if (*_it == '.')
                    {
                        ++_it;
                        while (_it != _end && std::isdigit(*_it)) ++_it;
                    }
                    //skip exponent
                    if (*_it == 'E' || *_it == 'e')
                    {
                        ++_it;
                        if (*_it == '+' || *_it == '-')
                            ++_it;

                        while (_it != _end && std::isdigit(*_it)) ++_it;
                    }
                    _it = skipWhitespaceAndCommas(_it, _end);
                    if (_endCondition(*_it))
                        break;
                }

                printf("NUM NUMBERS %lu\n", _outNumbers.count());

                //return advanceToNextCommand(_it, _end);
                return _it;
            }
            enum class TransformAction
            {
                Matrix,
                Translate,
                Scale,
                Rotate,
                SkewX,
                SkewY,
                None
            };

            static Mat3f parseTransform(stick::String::ConstIter _it,
                                        stick::String::ConstIter _end)
            {
                Mat3f ret = Mat3f::identity();
                TransformAction action = TransformAction::None;
                DynamicArray<Float> numbers;
                numbers.reserve(64);
                while (_it != _end)
                {
                    Mat3f tmp;
                    if (std::strncmp(_it, "matrix", 6) == 0)
                    {
                        action = TransformAction::Matrix;
                    }
                    else if (std::strncmp(_it, "translate", 9) == 0)
                    {
                        action = TransformAction::Translate;
                    }
                    else if (std::strncmp(_it, "scale", 5) == 0)
                    {
                        action = TransformAction::Scale;
                    }
                    else if (std::strncmp(_it, "rotate", 6) == 0)
                    {
                        action = TransformAction::Rotate;
                    }
                    else if (std::strncmp(_it, "skewX", 5) == 0)
                    {
                        action = TransformAction::SkewX;
                    }
                    else if (std::strncmp(_it, "skewY", 5) == 0)
                    {
                        action = TransformAction::SkewY;
                    }
                    else
                    {
                        ++_it;
                        continue;
                    }

                    //advance to the opening bracket
                    while (_it != _end && *_it != '(') ++_it;
                    _it = parseNumbers(_it, _end, [](char _c) { return _c == ')'; }, numbers);
                    if (_it != _end) ++_it; //skip ')'

                    if (action == TransformAction::Matrix && numbers.count() == 6)
                    {
                        tmp = Mat3f(Vec3f(numbers[0], numbers[1], numbers[2]),
                                    Vec3f(numbers[3], numbers[4], numbers[5]),
                                    Vec3f(0, 0, 1));
                    }
                    else if (action == TransformAction::Translate && numbers.count() >= 1)
                    {
                        tmp = Mat3f::translation2D(Vec2f(numbers[0], numbers.count() < 2 ? 0.0 : numbers[1]));
                    }
                    else if (action == TransformAction::Scale && numbers.count() >= 1)
                    {
                        tmp = Mat3f::scaling2D(Vec2f(numbers[0], numbers.count() < 2 ? numbers[0] : numbers[1]));
                    }
                    else if (action == TransformAction::Rotate && numbers.count() >= 1)
                    {
                        if (numbers.count() == 3)
                        {
                            Vec2f center = Vec2f(numbers[1], numbers[2]);
                            tmp = Mat3f::translation2D(center);
                            tmp.rotate2D(toRadians(numbers[0]));
                            tmp.translate2D(-center);
                        }
                        else
                        {
                            tmp = Mat3f::rotation2D(toRadians(numbers[0]));
                        }
                    }
                    else if (action == TransformAction::SkewX && numbers.count() == 1)
                    {
                        tmp = Mat3f::skewMatrix2D(Vec2f(numbers[0], 0));
                    }
                    else if (action == TransformAction::SkewY && numbers.count() == 1)
                    {
                        tmp = Mat3f::skewMatrix2D(Vec2f(0, numbers[0]));
                    }
                    else
                    {
                        continue;
                    }

                    //multiply with the current matrix (open gl style, right to left)
                    ret = tmp * ret;
                }
                return ret;
            }

            static stick::Maybe<ColorRGB> parseColor(String::ConstIter _begin, String::ConstIter _end)
            {
                _begin = skipWhitespaceAndCommas(_begin, _end);
                if (*_begin == '#')
                {
                    ++_begin; //skip the pound sign
                    UInt32 hexCountPerChannel = _end - _begin <= 4 ? 1 : 2;
                    String tmp(_begin, _begin + hexCountPerChannel);
                    auto r = std::strtol(tmp.cString(), NULL, 16);
                    tmp = String(_begin + hexCountPerChannel, _begin + hexCountPerChannel * 2);
                    auto g = std::strtol(tmp.cString(), NULL, 16);
                    tmp = String(_begin + hexCountPerChannel * 2, _begin + hexCountPerChannel * 3);
                    auto b = std::strtol(tmp.cString(), NULL, 16);
                    return ColorRGB(r / 255.0, g / 255.0, b / 255.0);
                }
                else if (String(_begin, _begin + 3) == "rgb")
                {
                    //code adjusted from nanosvg: https://github.com/memononen/nanosvg/blob/master/src/nanosvg.h
                    Int32 r = -1, g = -1, b = -1;
                    char s1[32] = "", s2[32] = "";
                    Int32 n = std::sscanf(_begin + 4, "%d%[%%, \t]%d%[%%, \t]%d", &r, s1, &g, s2, &b);
                    STICK_ASSERT(n == 5);
                    if (std::strchr(s1, '%'))
                        return ColorRGB(r / 100.0, g / 100.0, b / 100);
                    else
                        return ColorRGB(r / 255.0, g / 255.0, b / 255.0);
                }
                else
                {
                    printf("SVG COLOR\n");
                    //is this a named svg color?
                    auto it = svgColors.find(String(_begin, _end));
                    if (it != svgColors.end())
                        return it->value;
                }

                return stick::Maybe<ColorRGB>();
            }

            class StringView
            {
            public:

                StringView()
                {

                }

                StringView(String::ConstIter _begin, String::ConstIter _end) :
                    m_begin(_begin),
                    m_end(_end)
                {

                }

                String::ConstIter begin() const
                {
                    return m_begin;
                }

                String::ConstIter end() const
                {
                    return m_end;
                }

            private:

                String::ConstIter m_begin;
                String::ConstIter m_end;
            };

            static StringView parseURL(String::ConstIter _begin, String::ConstIter _end)
            {
                while (_begin != _end && *_begin != '(') ++_begin;
                while (_begin != _end && *_begin != '#') ++_begin;
                ++_begin;
                auto begin = _begin;
                while (_begin != _end && *_begin != ')') ++_begin;
                return StringView(begin, _begin);
            };

            template<class Functor>
            static bool findXMLAttrCB(const Shrub & _node, const String & _path, Item & _item, Functor _cb)
            {
                if (auto ch = _node.child(_path))
                {
                    if ((*ch).valueHint() == ValueHint::XMLAttribute)
                    {
                        //_item.set<Comp>((*ch).value<typename Comp::ValueType>());
                        _cb(_item, *ch);
                        return true;
                    }
                }
                return false;
            }

            static Vec2f reflect(const Vec2f & _position, const Vec2f & _around)
            {
                return _around + (_around - _position);
            }
        }

        SVGImport::SVGImport() :
            m_document(nullptr),
            m_dpi(72)
        {

        }

        SVGImport::SVGImport(Document & _doc) :
            m_document(&_doc),
            m_dpi(72)
        {

        }

        Float SVGImport::toPixels(Float _value, SVGUnits _units, Float _start, Float _length)
        {
            switch (_units)
            {
                case SVGUnits::PX:
                case SVGUnits::User:
                default:
                    return _value;
                case SVGUnits::PT:
                    return _value / (Float)72.0 * m_dpi;
                case SVGUnits::PC:
                    return _value / (Float)6.0 * m_dpi;
                case SVGUnits::EM:
                    return _value / (Float)6.0 * m_dpi;
                case SVGUnits::EX:
                    return _value / (Float)6.0 * m_dpi;
                case SVGUnits::CM:
                    return _value / (Float)2.54 * m_dpi;
                case SVGUnits::MM:
                    return _value / (Float)25.4 * m_dpi;
                case SVGUnits::IN:
                    return _value * m_dpi;
                case SVGUnits::Percent:
                    return _start + _value / 100.0 * _length;
            }
        }

        SVGImportResult SVGImport::parse(const String & _svg, Size _dpi)
        {
            m_dpi = _dpi;
            m_namedItems.clear();

            //TODO: It might be worthwhile to use pugixml directly to parse the svg
            //as that should be a lot faster as it would skip allocating the shrub tree etc.
            auto shrubRes = parseXML(_svg);
            if (!shrubRes)
                return shrubRes.error();
            Shrub & svg = shrubRes.get();

            Error err;
            Group grp = brick::reinterpretEntity<Group>(recursivelyImportNode(svg, svg, err));

            auto mw = svg.child("width");
            auto mh = svg.child("height");
            Float w, h;
            if (mw)
                w = (*mw).value<Float>();
            if (mh)
                h = (*mh).value<Float>();

            // remove all tmp items
            for(auto & item : m_tmpItems)
                item.remove();
            m_tmpItems.clear();

            return SVGImportResult(grp, w, h, err);
        }

        void SVGImport::pushAttributes(const Shrub & _node, const Shrub & _rootNode, Item & _item)
        {
            SVGAttributes attr;
            if (m_attributeStack.count()) attr = m_attributeStack.last();

            //order is important to some degree
            //setColorComponentFromXMLAttr<comps::Fill>(_node, "fill", _item);
            detail::findXMLAttrCB(_node, "fill", _item, [&attr](Item & _it, const Shrub & _child)
            {
                printf("SETTING FILL!\n");
                const String & str = _child.valueString();
                auto col = detail::parseColor(str.begin(), str.end());
                if (col)
                {
                    auto c =toRGBA(*col);
                    _it.set<comps::Fill>(c);
                    attr.fillColor = c;
                }
            });
            detail::findXMLAttrCB(_node, "fill-rule", _item, [&attr](Item & _it, const Shrub & _child)
            {
                if (_child.valueString() == "nonzero")
                    attr.windingRule = WindingRule::NonZero;
                else if (_child.valueString() == "evenodd")
                    attr.windingRule = WindingRule::EvenOdd;
                _it.setWindingRule(attr.windingRule);
            });
            detail::findXMLAttrCB(_node, "stroke", _item, [&attr](Item & _it, const Shrub & _child)
            {
                const String & str = _child.valueString();
                auto col = detail::parseColor(str.begin(), str.end());
                printf("SETTING DEM STROKE\n");
                if (col)
                {
                    printf("DONE DEM STROKE\n");
                    auto c = toRGBA(*col);
                    _it.set<comps::Stroke>(c);
                    attr.strokeColor = c;
                }
            });
            detail::findXMLAttrCB(_node, "stroke-width", _item, [&attr](Item & _it, const Shrub & _child)
            {
                attr.strokeWidth = _child.value<Float>();
                _it.setStrokeWidth(attr.strokeWidth);
            });
            detail::findXMLAttrCB(_node, "stroke-linecap", _item, [&attr](Item & _it, const Shrub & _child)
            {
                if (_child.valueString() == "butt")
                    attr.strokeCap = StrokeCap::Butt;
                else if (_child.valueString() == "round")
                    attr.strokeCap = StrokeCap::Round;
                else if (_child.valueString() == "square")
                    attr.strokeCap = StrokeCap::Square;

                _it.setStrokeCap(attr.strokeCap);
            });
            detail::findXMLAttrCB(_node, "stroke-linejoin", _item, [&attr](Item & _it, const Shrub & _child)
            {
                if (_child.valueString() == "miter")
                    attr.strokeJoin = StrokeJoin::Miter;
                else if (_child.valueString() == "round")
                    attr.strokeJoin = StrokeJoin::Round;
                else if (_child.valueString() == "bevel")
                    attr.strokeJoin = StrokeJoin::Bevel;

                _it.setStrokeJoin(attr.strokeJoin);
            });
            detail::findXMLAttrCB(_node, "vector-effect", _item, [&attr](Item & _it, const Shrub & _child)
            {
                attr.bScalingStroke = _child.valueString() != "non-scaling-stroke";
                _it.setStrokeScaling(attr.bScalingStroke);
            });
            detail::findXMLAttrCB(_node, "stroke-miterlimit", _item, [&attr](Item & _it, const Shrub & _child)
            {
                attr.miterLimit = _child.value<Float>();
                _it.setMiterLimit(attr.miterLimit);
            });
            detail::findXMLAttrCB(_node, "stroke-dasharray", _item, [&](Item & _it, const Shrub & _child)
            {
                printf("SETTING DASH ARRAY\n");
                attr.dashArray.clear();

                auto it = _child.valueString().begin();
                auto end = _child.valueString().end();
                it = detail::skipWhitespaceAndCommas(it, end);

                //handle none case
                if (it != end && *it != 'n')
                {
                    while (it != end)
                    {
                        //TODO take percentage start and length into account and pass it to toPixels
                        attr.dashArray.append(coordinatePixels(it));
                        while (it != end && !std::isspace(*it) && *it != ',') ++it;
                        it = detail::skipWhitespaceAndCommas(it, end);
                    }
                }

                _it.setDashArray(attr.dashArray);
            });

            detail::findXMLAttrCB(_node, "stroke-dashoffset", _item, [&attr](Item & _it, const Shrub & _child)
            {
                attr.dashOffset = _child.value<Float>();
                _it.setDashOffset(attr.dashOffset);
            });

            detail::findXMLAttrCB(_node, "font-size", _item, [&attr](Item & _it, const Shrub & _child)
            {
                attr.fontSize = _child.value<Float>();
            });

            detail::findXMLAttrCB(_node, "transform", _item, [](Item & _it, const Shrub & _child)
            {
                _it.setTransform(detail::parseTransform(_child.valueString().begin(), _child.valueString().end()));
            });

            detail::findXMLAttrCB(_node, "id", _item, [&](Item & _it, const Shrub & _child)
            {
                _it.setName(_child.valueString());
                m_namedItems.insert(_child.valueString(), _item);
            });

            m_attributeStack.append(attr);
        }

        void SVGImport::popAttributes()
        {
            m_attributeStack.removeLast();
        }

        Item SVGImport::recursivelyImportNode(const Shrub & _node, const Shrub & _rootNode, Error & _error)
        {
            Item item;
            if (_node.name() == "svg")
            {
                item = importGroup(_node, _rootNode, _error);
            }
            else if (_node.name() == "g")
            {
                item = importGroup(_node, _rootNode, _error);
            }
            else if (_node.name() == "rect")
            {
                item = importRectangle(_node, _rootNode, _error);
            }
            else if (_node.name() == "circle")
            {
                item = importCircle(_node, _rootNode, _error);
            }
            else if (_node.name() == "ellipse")
            {
                item = importEllipse(_node, _rootNode, _error);
            }
            else if (_node.name() == "line")
            {
                item = importLine(_node, _rootNode, _error);
            }
            else if (_node.name() == "polyline")
            {
                item = importPolyline(_node, _rootNode, false, _error);
            }
            else if (_node.name() == "polygon")
            {
                item = importPolyline(_node, _rootNode, true, _error);
            }
            else if (_node.name() == "path")
            {
                item = importPath(_node, _rootNode, _error);
            }
            else if (_node.name() == "clipPath")
            {
                item = importClipPath(_node, _rootNode, _error);
            }
            else if (_node.name() == "defs")
            {
                item = importGroup(_node, _rootNode, _error);
                if (!_error)
                    m_tmpItems.append(item);
            }
            // else if (_node.name() == "symbol")
            // {
            //     item = importGroup(_node, _error);
            // }
            else
            {
                //?
            }

            if (item.isValid())
            {   
                // we take care of the clip-path attribute after parsing finished, as it might
                // have us nest the item in a group that needs to be returned instead of the item
                detail::findXMLAttrCB(_node, "clip-path", item, [&](Item & _it, const Shrub & _child)
                {
                    auto url = detail::parseURL(_child.valueString().begin(), _child.valueString().end());
                    String str(url.begin(), url.end());
                    auto it = m_namedItems.find(str);
                    Path mask;
                    //check if we allready imported the clip path
                    if (it != m_namedItems.end())
                    {
                        mask = brick::reinterpretEntity<Path>(it->value);
                    }
                    else
                    {
                        //if not, find it in the document and import it
                        auto maybe = _rootNode.find([&str](const Shrub & _s)
                        {
                            for (auto & _c : _s)
                            {
                                if (_c.valueHint() == ValueHint::XMLAttribute && _c.name() == "id" && _c.valueString() == str)
                                    return true;
                            }
                            return false;
                        });

                        if (maybe)
                        {
                            Error err;
                            mask = importClipPath(*maybe, _rootNode, err);
                            //TODO: HANDLE ERROR?
                        }
                    }

                    if (mask.isValid())
                    {
                        if (_it.itemType() == EntityType::Group)
                        {
                            Group grp = brick::reinterpretEntity<Group>(_it);
                            grp.setClipped(true);
                            if (grp.children().count())
                            {
                                mask.clone().insertBelow(grp.children().first());
                            }
                            else
                            {
                                grp.addChild(mask.clone());
                            }
                        }
                        else
                        {
                            Group grp = m_document->createGroup();
                            grp.addChild(mask.clone());
                            grp.addChild(_it);
                            grp.setClipped(true);
                            item = grp;
                        }
                    }
                    else
                    {
                        //WARNING? ERROR?
                    }
                });
            }

            return item;
        }

        Group SVGImport::importGroup(const Shrub & _node, const Shrub & _rootNode, Error & _error)
        {
            Group grp = m_document->createGroup();
            pushAttributes(_node, _rootNode, grp);
            for (auto & child : _node)
            {
                if (child.valueHint() != ValueHint::XMLAttribute)
                {
                    Item item = recursivelyImportNode(child, _rootNode, _error);
                    if (_error) break;
                    else grp.addChild(item);
                    
                    //set the default fill if none is inherited
                    if(!item.hasFill())
                        item.setFill(ColorRGBA(0, 0, 0, 1));
                }
            }
            popAttributes();
            return grp;
        }

        Path SVGImport::importClipPath(const Shrub & _node, const Shrub & _rootNode, Error & _error)
        {
            Path ret = m_document->createPath();
            for (auto & child : _node)
            {
                if (child.valueHint() != ValueHint::XMLAttribute)
                {
                    Item item = recursivelyImportNode(child, _rootNode, _error);
                    if (_error) break;
                    //only add paths as children, ignore the rest (there should be no rest though, safety first :))
                    if (Path p = brick::entityCast<Path>(item))
                    {
                        ret.addChild(p);
                    }
                }
            }
            pushAttributes(_node, _rootNode, ret);
            popAttributes();
            m_tmpItems.append(ret);
            return ret;
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

        SVGCoordinate SVGImport::parseCoordinate(const char * _str)
        {
            SVGCoordinate ret;
            char units[32] = "";
            std::sscanf(_str, "%f%s", &ret.value, units);

            if (units[0] == 'e' && units[1] == 'm')
                ret.units = SVGUnits::EM;
            else if (units[0] == 'e' && units[1] == 'x')
                ret.units = SVGUnits::EX;
            else if (units[0] == 'p' && units[1] == 'x')
                ret.units = SVGUnits::PX;
            else if (units[0] == 'p' && units[1] == 't')
                ret.units = SVGUnits::PT;
            else if (units[0] == 'p' && units[1] == 'c')
                ret.units = SVGUnits::PC;
            else if (units[0] == 'c' && units[1] == 'm')
                ret.units = SVGUnits::CM;
            else if (units[0] == 'm' && units[1] == 'm')
                ret.units = SVGUnits::MM;
            else if (units[0] == 'i' && units[1] == 'n')
                ret.units = SVGUnits::IN;
            else if (units[0] == '%')
                ret.units = SVGUnits::Percent;
            else
                ret.units = SVGUnits::User;

            return ret;
        }

        Float SVGImport::coordinatePixels(const char * _str, Float _start, Float _length)
        {
            auto coord = parseCoordinate(_str);
            return toPixels(coord.value, coord.units, _start, _length);
        }

        Path SVGImport::importPath(const Shrub & _node, const Shrub & _rootNode, Error & _error)
        {
            printf("IMPORT PATH\n");
            auto maybe = _node.child("d");
            if (maybe)
            {
                printf("GOT DATA\n");
                const String & val = (*maybe).valueString();
                DynamicArray<Float> numbers;
                auto end = val.end();
                auto it = detail::skipWhitespaceAndCommas(val.begin(), end);
                Path p = m_document->createPath();
                pushAttributes(_node, _rootNode, p);
                Path currentPath = p;
                Vec2f last;
                Vec2f lastHandle;
                do
                {
                    char cmd = *it;
                    //auto tend = advanceToNextCommand(it + 1, end);
                    it = detail::parseNumbers(it, end, [](char _c) { return isCommand(_c); }, numbers);
                    // STICK_ASSERT(it == tend);
                    if (cmd == 'M' || cmd == 'm')
                    {
                        printf("MOVE CMD\n");
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
                        printf("L CMD\n");
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
                        printf("H CMD\n");
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
                        printf("C CMD\n");
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
                        printf("S CMD\n");
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
                            currentPath.cubicCurveTo(detail::reflect(lastHandle, last), nextHandle, nextLast);
                            lastHandle = nextHandle;
                            last = nextLast;
                        }
                    }
                    else if (cmd == 'Q' || cmd == 'q')
                    {
                        printf("Q CMD\n");
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
                        printf("T CMD\n");
                        bool bRelative = cmd == 't';
                        Vec2f start = last;
                        for (int i = 0; i < numbers.count(); i += 2)
                        {
                            Vec2f nextLast = !bRelative ? Vec2f(numbers[i], numbers[i + 1]) : start + Vec2f(numbers[i], numbers[i + 1]);
                            lastHandle = detail::reflect(lastHandle, last);
                            currentPath.quadraticCurveTo(lastHandle, nextLast);
                            last = nextLast;
                        }
                    }
                    else if (cmd == 'A' || cmd == 'a')
                    {
                        printf("A CMD\n");
                        bool bRelative = cmd == 'a';
                        for (int i = 0; i < numbers.count(); i += 7)
                        {
                            last = !bRelative ? Vec2f(numbers[i + 5], numbers[i + 6]) : last + Vec2f(numbers[i + 5], numbers[i + 6]);

                            Float rads = crunch::toRadians(numbers[i + 2]);
                            printf("ARC MOTHERFUCKER %f %f %f %f %f %f %f\n", numbers[i + 0], numbers[i + 1], numbers[i + 2], numbers[i + 3], numbers[i + 4], numbers[i + 5], numbers[i + 6]);
                            printf("LAST %f %f\n", last.x, last.y);
                            currentPath.arcTo(last, Vec2f(numbers[i], numbers[i + 1]),
                                              crunch::toRadians(numbers[i + 2]), (bool)numbers[i + 4], (bool)numbers[i + 3]);
                            lastHandle = currentPath.segments().last().handleOutAbsolute();
                        }
                    }
                    else if (cmd == 'Z' || cmd == 'z')
                    {
                        printf("Z CMD\n");
                        currentPath.closePath();
                        last = currentPath.segments().last().position();
                        lastHandle = currentPath.segments().last().handleOutAbsolute();
                    }
                    else
                    {

                    }
                }
                while (it != end);

                popAttributes();

                //return the path
                return p;
            }
            else
            {
                _error = Error(ec::ParseFailed, "SVG path is missing d attribute", STICK_FILE, STICK_LINE);
            }
            return Path();
        }

        Path SVGImport::importPolyline(const Shrub & _node, const Shrub & _rootNode, bool _bIsPolygon, Error & _error)
        {
            auto mpoints = _node.child("points");
            if (mpoints)
            {
                DynamicArray<Float> numbers;
                numbers.reserve(64);
                detail::parseNumbers((*mpoints).valueString().begin(), (*mpoints).valueString().end(), [](char) { return false; }, numbers);
                Path ret = m_document->createPath();
                for (Size i = 0; i < numbers.count(); i += 2)
                    ret.addPoint(Vec2f(numbers[i], numbers[i + 1]));

                if (_bIsPolygon)
                    ret.closePath();

                pushAttributes(_node, _rootNode, ret);
                popAttributes();
                return ret;
            }
            else
            {
                _error = Error(ec::ParseFailed, "SVG polyline/polygon is missing points attribute", STICK_FILE, STICK_LINE);
            }
            return Path();
        }

        Path SVGImport::importCircle(const Shrub & _node, const Shrub & _rootNode, Error & _error)
        {
            auto mcx = _node.child("cx");
            auto mcy = _node.child("cy");
            auto mr = _node.child("r");
            if (mcx && mcy && mr)
            {
                Path ret = m_document->createCircle(Vec2f(coordinatePixels((*mcx).valueString().begin()),
                                                    coordinatePixels((*mcy).valueString().begin())),
                                                    coordinatePixels((*mr).valueString().begin()));
                pushAttributes(_node, _rootNode, ret);
                popAttributes();
                return ret;
            }
            else
            {
                _error = Error(ec::ParseFailed, "SVG circle has to provide cx, cy and r", STICK_FILE, STICK_LINE);
            }
            return Path();
        }

        Path SVGImport::importEllipse(const Shrub & _node, const Shrub & _rootNode, Error & _error)
        {
            auto mcx = _node.child("cx");
            auto mcy = _node.child("cy");
            auto mrx = _node.child("rx");
            auto mry = _node.child("ry");
            if (mcx && mcy && mrx && mry)
            {
                printf("MAKING ELLIPSE YOOOOO\n");
                Path ret = m_document->createEllipse(Vec2f(coordinatePixels((*mcx).valueString().begin()),
                                                     coordinatePixels((*mcy).valueString().begin())),
                                                     Vec2f(coordinatePixels((*mrx).valueString().begin()) * 2,
                                                             coordinatePixels((*mry).valueString().begin()) * 2));
                pushAttributes(_node, _rootNode, ret);
                popAttributes();
                return ret;
            }
            else
            {
                _error = Error(ec::ParseFailed, "SVG ellipse has to provide cx, cy, rx and ry", STICK_FILE, STICK_LINE);
            }
            return Path();
        }

        Path SVGImport::importRectangle(const Shrub & _node, const Shrub & _rootNode, Error & _error)
        {
            auto mx = _node.child("x");
            auto my = _node.child("y");
            auto mw = _node.child("width");
            auto mh = _node.child("height");

            if (mx && my && mw && mh)
            {
                //TODO: as soon as we added the createRoundedRectangle function to Document,
                //take these into account!
                // auto mrx = _node.child("rx");
                // auto mry = _node.child("ry");

                Float x = coordinatePixels((*mx).valueString().begin());
                Float y = coordinatePixels((*my).valueString().begin());
                Path ret = m_document->createRectangle(Vec2f(x, y), Vec2f(x, y) +
                                                       Vec2f(coordinatePixels((*mw).valueString().begin()),
                                                               coordinatePixels((*mh).valueString().begin())));
                pushAttributes(_node, _rootNode, ret);
                popAttributes();
                return ret;
            }
            else
            {
                _error = Error(ec::ParseFailed, "SVG rect missing x, y, width or height attribute", STICK_FILE, STICK_LINE);
            }

            return Path();
        }

        Path SVGImport::importLine(const Shrub & _node, const Shrub & _rootNode, Error & _error)
        {
            auto mx1 = _node.child("x1");
            auto my1 = _node.child("y1");
            auto mx2 = _node.child("x2");
            auto my2 = _node.child("y2");
            if (mx1 && my1 && mx2 && my2)
            {
                Float x1 = coordinatePixels((*mx1).valueString().begin());
                Float y1 = coordinatePixels((*my1).valueString().begin());
                Float x2 = coordinatePixels((*mx2).valueString().begin());
                Float y2 = coordinatePixels((*my2).valueString().begin());
                Path ret = m_document->createPath();
                ret.addPoint(Vec2f(x1, y1));
                ret.addPoint(Vec2f(x2, y2));
                pushAttributes(_node, _rootNode, ret);
                popAttributes();
                return ret;
            }
            else
            {
                _error = Error(ec::ParseFailed, "SVG line missing x1, y1, x2 or y1 attribute", STICK_FILE, STICK_LINE);
            }
            return Path();
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
