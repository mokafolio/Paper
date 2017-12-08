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
                _it = skipWhitespaceAndCommas(_it, _end);

                Float value;
                while (_it != _end)
                {
                    std::sscanf(_it, "%f", &value);
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

                //@TODO: Use document allocator
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
                    ++_it;
                    _it = parseNumbers(_it, _end, [](char _c) { return _c == ')'; }, numbers);
                    if (_it != _end) ++_it; //skip ')'

                    if (action == TransformAction::Matrix && numbers.count() == 6)
                    {
                        tmp = Mat3f(Vec3f(numbers[0], numbers[1], 0),
                                    Vec3f(numbers[2], numbers[3], 0),
                                    Vec3f(numbers[4], numbers[5], 1));
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
                        tmp = Mat3f::skewMatrix2D(Vec2f(toRadians(numbers[0]), 0));
                    }
                    else if (action == TransformAction::SkewY && numbers.count() == 1)
                    {
                        tmp = Mat3f::skewMatrix2D(Vec2f(0, toRadians(numbers[0])));
                    }
                    else
                    {
                        continue;
                    }

                    //multiply with the current matrix (open gl style, right to left)
                    ret = ret * tmp;
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
                    if (hexCountPerChannel == 2)
                    {
                        String tmp(_begin, _begin + 2);
                        auto r = std::strtoul(tmp.cString(), NULL, 16);
                        tmp = String(_begin + 2, _begin + 4);
                        auto g = std::strtoul(tmp.cString(), NULL, 16);
                        tmp = String(_begin + 4, _begin + 6);
                        auto b = std::strtoul(tmp.cString(), NULL, 16);
                        return ColorRGB(r / 255.0, g / 255.0, b / 255.0);
                    }
                    else
                    {
                        String tmp = String::concat(_begin[0], _begin[0]);
                        auto r = std::strtoul(tmp.cString(), NULL, 16);
                        tmp = String::concat(_begin[1], _begin[1]);
                        auto g = std::strtoul(tmp.cString(), NULL, 16);
                        tmp = String::concat(_begin[2], _begin[2]);
                        auto b = std::strtoul(tmp.cString(), NULL, 16);
                        return ColorRGB(r / 255.0, g / 255.0, b / 255.0);
                    }
                }
                else if (std::strncmp(_begin, "rgb", 3) == 0)
                {
                    //code adjusted from nanosvg: https://github.com/memononen/nanosvg/blob/master/src/nanosvg.h
                    Int32 r = -1, g = -1, b = -1;
                    char s1[32] = "", s2[32] = "";
                    Int32 n = std::sscanf(_begin + 4, "%d%[%%, \t]%d%[%%, \t]%d", &r, s1, &g, s2, &b);
                    STICK_ASSERT(n == 5);
                    if (std::strchr(s1, '%'))
                        return ColorRGB(r / 100.0, g / 100.0, b / 100.0);
                    else
                        return ColorRGB(r / 255.0, g / 255.0, b / 255.0);
                }
                else
                {
                    //is this a named svg color?
                    auto col = crunch::svgColor<ColorRGB>(String(_begin, _end));
                    return crunch::svgColor<ColorRGB>(String(_begin, _end));
                }

                return Maybe<ColorRGB>();
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
            auto shrubRes = parseXML(_svg, m_document->allocator());
            if (!shrubRes)
                return shrubRes.error();
            Shrub & svg = shrubRes.get();

            auto mx = svg.child("x");
            auto my = svg.child("y");
            auto mw = svg.child("width");
            auto mh = svg.child("height");

            Float x, y, w, h;
            if (mx)
                x = (*mx).value<Float>();
            if (my)
                y = (*my).value<Float>();
            if (mw)
                w = (*mw).value<Float>();
            if (mh)
                h = (*mh).value<Float>();

            Error err;
            Group grp = brick::reinterpretEntity<Group>(recursivelyImportNode(svg, svg, err));

            // remove all tmp items
            for (auto & item : m_tmpItems)
                item.remove();
            m_tmpItems.clear();

            return SVGImportResult(grp, w, h, err);
        }

        void SVGImport::parseAttribute(const String & _name, const String & _value, SVGAttributes & _attr, Item & _item)
        {
            if (_name == "style")
            {
                parseStyle(_value, _attr, _item);
            }
            else if (_name == "fill")
            {
                if (_value == "none")
                {
                    _item.setNoFill();
                }
                else
                {
                    auto col = detail::parseColor(_value.begin(), _value.end());
                    if (col)
                    {
                        auto c = toRGBA(*col);
                        //we need to get the current alpha, as that might have been set allready
                        c.a = _item.fillOpacity();
                        _item.setFill(c);
                        _attr.fillColor = c;
                    }
                }
            }
            else if (_name == "fill-opacity")
            {
                _attr.fillColor.a = toFloat32(_value);
                if (auto mf = _item.fill().maybe<ColorRGBA>())
                {
                    _item.setFill(ColorRGBA((*mf).r, (*mf).g, (*mf).b, (*mf).a));
                }
            }
            else if (_name == "fill-rule")
            {
                if (_value == "nonzero")
                    _attr.windingRule = WindingRule::NonZero;
                else if (_value == "evenodd")
                    _attr.windingRule = WindingRule::EvenOdd;
                _item.setWindingRule(_attr.windingRule);
            }
            else if (_name == "stroke")
            {
                if (_value == "none")
                {
                    _item.setNoStroke();
                }
                else
                {
                    auto col = detail::parseColor(_value.begin(), _value.end());
                    if (col)
                    {
                        auto c = toRGBA(*col);
                        //we need to get the current alpha, as that might have been set allready
                        c.a = _item.strokeOpacity();
                        _item.setStroke(c);
                        _attr.strokeColor = c;
                    }
                }
            }
            else if (_name == "stroke-opacity")
            {
                _attr.strokeColor.a = toFloat32(_value);
                if (auto mf = _item.stroke().maybe<ColorRGBA>())
                {
                    _item.setStroke(ColorRGBA((*mf).r, (*mf).g, (*mf).b, _attr.strokeColor.a));
                }
            }
            else if (_name == "stroke-width")
            {
                _attr.strokeWidth = coordinatePixels(_value.cString());
                _item.setStrokeWidth(_attr.strokeWidth);
            }
            else if (_name == "stroke-linecap")
            {
                if (_value == "butt")
                    _attr.strokeCap = StrokeCap::Butt;
                else if (_value == "round")
                    _attr.strokeCap = StrokeCap::Round;
                else if (_value == "square")
                    _attr.strokeCap = StrokeCap::Square;

                _item.setStrokeCap(_attr.strokeCap);
            }
            else if (_name == "stroke-linejoin")
            {
                if (_value == "miter")
                    _attr.strokeJoin = StrokeJoin::Miter;
                else if (_value == "round")
                    _attr.strokeJoin = StrokeJoin::Round;
                else if (_value == "bevel")
                    _attr.strokeJoin = StrokeJoin::Bevel;

                _item.setStrokeJoin(_attr.strokeJoin);
            }
            else if (_name == "stroke-miterlimit")
            {
                _attr.miterLimit = coordinatePixels(_value.cString());
                _item.setMiterLimit(_attr.miterLimit);
            }
            else if (_name == "vector-effect")
            {
                _attr.bScalingStroke = _value != "non-scaling-stroke";
                _item.setStrokeScaling(_attr.bScalingStroke);
            }
            else if (_name == "stroke-dasharray")
            {
                _attr.dashArray.clear();

                auto it = _value.begin();
                auto end = _value.end();
                it = detail::skipWhitespaceAndCommas(it, end);

                //handle none case
                if (it != end && *it != 'n')
                {
                    while (it != end)
                    {
                        //TODO take percentage start and length into account and pass it to toPixels
                        _attr.dashArray.append(coordinatePixels(it));
                        while (it != end && !std::isspace(*it) && *it != ',') ++it;
                        it = detail::skipWhitespaceAndCommas(it, end);
                    }
                }

                _item.setDashArray(_attr.dashArray);
            }
            else if (_name == "stroke-dashoffset")
            {
                _attr.dashOffset = coordinatePixels(_value.cString());
                _item.setDashOffset(_attr.dashOffset);
            }
            else if (_name == "font-size")
            {
                _attr.fontSize = coordinatePixels(_value.cString());
            }
            else if (_name == "transform")
            {
                _item.setTransform(detail::parseTransform(_value.begin(), _value.end()));
            }
            else if (_name == "id")
            {
                _item.setName(_value);
                m_namedItems.insert(_value, _item);
            }
        }

        void SVGImport::parseStyle(const String & _style, SVGAttributes & _attr, Item & _item)
        {
            auto b = _style.begin();
            auto e = _style.end();
            String left, right;
            while (b != e)
            {
                while (b != e && std::isspace(*b)) ++b;
                auto ls = b;
                while (b != e && *b != ':') ++b;
                left = String(ls, b);
                ++b;
                while (b != e && std::isspace(*b)) ++b;
                ls = b;
                while (b != e && *b != ';') ++b;
                right = String(ls, b);
                parseAttribute(left, right, _attr, _item);
                if (b != e) ++b;
            }
        }

        void SVGImport::pushAttributes(const Shrub & _node, const Shrub & _rootNode, Item & _item)
        {
            SVGAttributes attr;
            if (m_attributeStack.count()) attr = m_attributeStack.last();

            for (const Shrub & c : _node)
            {
                if (c.valueHint() == ValueHint::XMLAttribute)
                    parseAttribute(c.name(), c.valueString(), attr, _item);
            }

            // //order is important to some degree
            // //setColorComponentFromXMLAttr<comps::Fill>(_node, "fill", _item);
            // detail::findXMLAttrCB(_node, "fill", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     printf("SETTING FILL!\n");
            //     const String & str = _child.valueString();
            //     if (str == "none")
            //     {
            //         _it.setNoFill();
            //     }
            //     else
            //     {
            //         auto col = detail::parseColor(str.begin(), str.end());
            //         if (col)
            //         {
            //             auto c = toRGBA(*col);
            //             _it.setFill(c);
            //             attr.fillColor = c;
            //         }
            //     }
            // });
            // detail::findXMLAttrCB(_node, "fill-opacity", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     attr.fillColor.a = _child.value<Float>();
            //     ColorPaint col = brick::reinterpretEntity<ColorPaint>(_it.fill());
            //     col.setColor(ColorRGBA(col.color().r, col.color().g, col.color().b, attr.fillColor.a));
            // });
            // detail::findXMLAttrCB(_node, "fill-rule", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     if (_child.valueString() == "nonzero")
            //         attr.windingRule = WindingRule::NonZero;
            //     else if (_child.valueString() == "evenodd")
            //         attr.windingRule = WindingRule::EvenOdd;
            //     _it.setWindingRule(attr.windingRule);
            // });
            // detail::findXMLAttrCB(_node, "stroke", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     const String & str = _child.valueString();
            //     if (str == "none")
            //     {
            //         _it.setNoStroke();
            //     }
            //     else
            //     {
            //         auto col = detail::parseColor(str.begin(), str.end());
            //         if (col)
            //         {
            //             printf("DONE DEM STROKE\n");
            //             auto c = toRGBA(*col);
            //             _it.setStroke(c);
            //             attr.strokeColor = c;
            //         }
            //     }
            // });
            // detail::findXMLAttrCB(_node, "stroke-opacity", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     attr.strokeColor.a = _child.value<Float>();
            //     ColorPaint col = brick::reinterpretEntity<ColorPaint>(_it.stroke());
            //     col.setColor(ColorRGBA(col.color().r, col.color().g, col.color().b, attr.strokeColor.a));
            // });
            // detail::findXMLAttrCB(_node, "stroke-width", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     attr.strokeWidth = _child.value<Float>();
            //     _it.setStrokeWidth(attr.strokeWidth);
            // });
            // detail::findXMLAttrCB(_node, "stroke-linecap", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     if (_child.valueString() == "butt")
            //         attr.strokeCap = StrokeCap::Butt;
            //     else if (_child.valueString() == "round")
            //         attr.strokeCap = StrokeCap::Round;
            //     else if (_child.valueString() == "square")
            //         attr.strokeCap = StrokeCap::Square;

            //     _it.setStrokeCap(attr.strokeCap);
            // });
            // detail::findXMLAttrCB(_node, "stroke-linejoin", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     if (_child.valueString() == "miter")
            //         attr.strokeJoin = StrokeJoin::Miter;
            //     else if (_child.valueString() == "round")
            //         attr.strokeJoin = StrokeJoin::Round;
            //     else if (_child.valueString() == "bevel")
            //         attr.strokeJoin = StrokeJoin::Bevel;

            //     _it.setStrokeJoin(attr.strokeJoin);
            // });
            // detail::findXMLAttrCB(_node, "vector-effect", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     attr.bScalingStroke = _child.valueString() != "non-scaling-stroke";
            //     _it.setStrokeScaling(attr.bScalingStroke);
            // });
            // detail::findXMLAttrCB(_node, "stroke-miterlimit", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     attr.miterLimit = _child.value<Float>();
            //     _it.setMiterLimit(attr.miterLimit);
            // });
            // detail::findXMLAttrCB(_node, "stroke-dasharray", _item, [&](Item & _it, const Shrub & _child)
            // {
            //     printf("SETTING DASH ARRAY\n");
            //     attr.dashArray.clear();

            //     auto it = _child.valueString().begin();
            //     auto end = _child.valueString().end();
            //     it = detail::skipWhitespaceAndCommas(it, end);

            //     //handle none case
            //     if (it != end && *it != 'n')
            //     {
            //         while (it != end)
            //         {
            //             //TODO take percentage start and length into account and pass it to toPixels
            //             attr.dashArray.append(coordinatePixels(it));
            //             while (it != end && !std::isspace(*it) && *it != ',') ++it;
            //             it = detail::skipWhitespaceAndCommas(it, end);
            //         }
            //     }

            //     _it.setDashArray(attr.dashArray);
            // });

            // detail::findXMLAttrCB(_node, "stroke-dashoffset", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     attr.dashOffset = _child.value<Float>();
            //     _it.setDashOffset(attr.dashOffset);
            // });

            // detail::findXMLAttrCB(_node, "font-size", _item, [&attr](Item & _it, const Shrub & _child)
            // {
            //     attr.fontSize = _child.value<Float>();
            // });

            // detail::findXMLAttrCB(_node, "transform", _item, [](Item & _it, const Shrub & _child)
            // {
            //     _it.setTransform(detail::parseTransform(_child.valueString().begin(), _child.valueString().end()));
            // });

            // detail::findXMLAttrCB(_node, "id", _item, [&](Item & _it, const Shrub & _child)
            // {
            //     _it.setName(_child.valueString());
            //     m_namedItems.insert(_child.valueString(), _item);
            // });

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
                item = importGroup(_node, _rootNode, true, _error);
            }
            else if (_node.name() == "g")
            {
                item = importGroup(_node, _rootNode, false, _error);
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
                item = importGroup(_node, _rootNode, false, _error);
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
                if (Path p = brick::entityCast<Path>(item))
                {
                    if (!p.findComponent<comps::WindingRule>())
                        p.setWindingRule(WindingRule::NonZero);
                }
                // we take care of the clip-path / viewBox (as it might induce clipping, too) attribute after parsing finished, as it might
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

        Group SVGImport::importGroup(const Shrub & _node, const Shrub & _rootNode, bool _bSVGNode, Error & _error)
        {
            Group grp = m_document->createGroup();

            //establish a new view based on the provided x,y,width,height (needed for viewbox calculation)
            if (_bSVGNode)
            {
                Float x, y, w, h;
                auto mx = _node.maybe<Float>("x");
                auto my = _node.maybe<Float>("y");
                auto mw = _node.maybe<Float>("width");
                auto mh = _node.maybe<Float>("height");
                if (mw && mh)
                {
                    w = *mw;
                    h = *mh;
                    x = mx ? *mx : 0;
                    y = my ? *my : 0;
                    m_viewStack.append(Rect(x, y, x + w, y + h));
                }
            }
            pushAttributes(_node, _rootNode, grp);
            for (auto & child : _node)
            {
                if (child.valueHint() != ValueHint::XMLAttribute)
                {
                    Item item = recursivelyImportNode(child, _rootNode, _error);
                    if (_error) break;
                    else grp.addChild(item);

                    //set the default fill if none is inherited
                    if (item.itemType() == EntityType::Path && !item.findComponent<comps::Fill>())
                        item.setFill(ColorRGBA(0, 0, 0, 1));
                }
            }

            detail::findXMLAttrCB(_node, "viewBox", grp, [&](Item & _it, const Shrub & _child)
            {
                //if no view is esablished, ignore viewbox
                if (m_viewStack.count())
                {
                    //TODO: take preserveAspectRatio attribute into account (argh NOOOOOOOOOO)
                    const Rect & r = m_viewStack.last();

                    stick::DynamicArray<Float> numbers(m_document->allocator());
                    numbers.reserve(4);

                    detail::parseNumbers(_child.valueString().begin(), _child.valueString().end(), [](char _c) { return false; }, numbers);
                    Mat3f viewTransform = Mat3f::identity();
                    if (m_viewStack.count() > 1)
                    {
                        viewTransform.translate2D(r.min());
                    }
                    Vec2f scale(r.width() / numbers[2], r.height() / numbers[3]);
                    viewTransform.scale2D(scale);
                    _it.setTransform(viewTransform);

                    auto mask = m_document->createRectangle(Vec2f(0, 0), r.size() * (Vec2f(1.0) / scale));
                    mask.insertBelow(grp.children().first());
                    //mask.setFill(ColorRGBA(0, 0, 1, 0.2));
                    grp.setClipped(true);
                }
            });

            if (_bSVGNode)
            {
                m_viewStack.removeLast();
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

        void SVGImport::parsePathData(Document & _doc, Path _path, const String & _data)
        {
            DynamicArray<Float> numbers(_doc.allocator());
            auto end = _data.end();
            auto it = detail::skipWhitespaceAndCommas(_data.begin(), end);
            Path currentPath = _path;
            Vec2f last;
            Vec2f lastHandle;
            do
            {
                char cmd = *it;
                //auto tend = advanceToNextCommand(it + 1, end);
                ++it;
                it = detail::parseNumbers(it, end, [](char _c) { return isCommand(_c); }, numbers);

                // STICK_ASSERT(it == tend);
                if (cmd == 'M' || cmd == 'm')
                {
                    if (currentPath.segmentArray().count())
                    {
                        Path tmp = _doc.createPath();
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
                        currentPath.cubicCurveTo(detail::reflect(lastHandle, last), nextHandle, nextLast);
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
                        lastHandle = detail::reflect(lastHandle, last);
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

                        Float rads = crunch::toRadians(numbers[i + 2]);
                        currentPath.arcTo(last, Vec2f(numbers[i], numbers[i + 1]),
                                          crunch::toRadians(numbers[i + 2]), (bool)numbers[i + 4], (bool)numbers[i + 3]);
                        lastHandle = currentPath.segmentArray().last()->handleOutAbsolute();
                    }
                }
                else if (cmd == 'Z' || cmd == 'z')
                {
                    currentPath.closePath();
                    last = currentPath.segmentArray().last()->position();
                    lastHandle = currentPath.segmentArray().last()->handleOutAbsolute();
                }
                else
                {
                    //@TODO?
                }
            }
            while (it != end);
        }

        Path SVGImport::importPath(const Shrub & _node, const Shrub & _rootNode, Error & _error)
        {
            auto maybe = _node.child("d");
            if (maybe)
            {
                // const String & val = (*maybe).valueString();
                // DynamicArray<Float> numbers(m_document->allocator());
                // auto end = val.end();
                // auto it = detail::skipWhitespaceAndCommas(val.begin(), end);
                // Path p = m_document->createPath();
                // pushAttributes(_node, _rootNode, p);
                // Path currentPath = p;
                // Vec2f last;
                // Vec2f lastHandle;
                // do
                // {
                //     char cmd = *it;
                //     //auto tend = advanceToNextCommand(it + 1, end);
                //     ++it;
                //     it = detail::parseNumbers(it, end, [](char _c) { return isCommand(_c); }, numbers);

                //     // STICK_ASSERT(it == tend);
                //     if (cmd == 'M' || cmd == 'm')
                //     {
                //         if (currentPath.segmentArray().count())
                //         {
                //             Path tmp = m_document->createPath();
                //             currentPath.addChild(tmp);
                //             currentPath = tmp;
                //         }

                //         bool bRelative = cmd == 'm';
                //         for (int i = 0; i < numbers.count(); i += 2)
                //         {
                //             if (bRelative)
                //                 last = last + Vec2f(numbers[i], numbers[i + 1]);
                //             else
                //                 last = Vec2f(numbers[i], numbers[i + 1]);
                //             currentPath.addPoint(last);
                //         }
                //         lastHandle = last;
                //     }
                //     else if (cmd == 'L' || cmd == 'l')
                //     {
                //         bool bRelative = cmd == 'l';
                //         for (int i = 0; i < numbers.count(); i += 2)
                //         {
                //             if (bRelative)
                //                 last = last + Vec2f(numbers[i], numbers[i + 1]);
                //             else
                //                 last = Vec2f(numbers[i], numbers[i + 1]);
                //             currentPath.addPoint(last);
                //         }
                //         lastHandle = last;
                //     }
                //     else if (cmd == 'H' || cmd == 'h' || cmd == 'V' || cmd == 'v')
                //     {
                //         bool bRelative = cmd == 'h' || cmd == 'v';
                //         bool bVert = cmd == 'V' || cmd == 'v';
                //         for (int i = 0; i < numbers.count(); ++i)
                //         {
                //             if (bVert)
                //             {
                //                 if (bRelative)
                //                     last.y = last.y + numbers[i];
                //                 else
                //                     last.y = numbers[i];
                //             }
                //             else
                //             {
                //                 if (bRelative)
                //                     last.x = last.x + numbers[i];
                //                 else
                //                     last.x = numbers[i];
                //             }
                //             currentPath.addPoint(last);
                //         }
                //         lastHandle = last;
                //     }
                //     else if (cmd == 'C' || cmd == 'c')
                //     {
                //         bool bRelative = cmd == 'c';
                //         Vec2f start = last;
                //         for (int i = 0; i < numbers.count(); i += 6)
                //         {
                //             if (!bRelative)
                //             {
                //                 last = Vec2f(numbers[i + 4], numbers[i + 5]);
                //                 lastHandle = Vec2f(numbers[i + 2], numbers[i + 3]);
                //                 currentPath.cubicCurveTo(Vec2f(numbers[i], numbers[i + 1]),
                //                                          lastHandle,
                //                                          last);
                //             }
                //             else
                //             {
                //                 last = start + Vec2f(numbers[i + 4], numbers[i + 5]);
                //                 lastHandle = Vec2f(start.x + numbers[i + 2], start.y + numbers[i + 3]);
                //                 currentPath.cubicCurveTo(Vec2f(start.x + numbers[i], start.y + numbers[i + 1]),
                //                                          lastHandle,
                //                                          last);
                //             }
                //         }
                //     }
                //     else if (cmd == 'S' || cmd == 's')
                //     {
                //         bool bRelative = cmd == 's';
                //         Vec2f start = last;
                //         for (int i = 0; i < numbers.count(); i += 4)
                //         {

                //             Vec2f nextLast, nextHandle;
                //             if (!bRelative)
                //             {
                //                 nextLast = Vec2f(numbers[i + 2], numbers[i + 3]);
                //                 nextHandle = Vec2f(numbers[i], numbers[i + 1]);
                //             }
                //             else
                //             {
                //                 nextLast = start + Vec2f(numbers[i + 2], numbers[i + 3]);
                //                 nextHandle = start + Vec2f(numbers[i], numbers[i + 1]);
                //             }
                //             currentPath.cubicCurveTo(detail::reflect(lastHandle, last), nextHandle, nextLast);
                //             lastHandle = nextHandle;
                //             last = nextLast;
                //         }
                //     }
                //     else if (cmd == 'Q' || cmd == 'q')
                //     {
                //         bool bRelative = cmd == 'q';
                //         Vec2f start = last;
                //         for (int i = 0; i < numbers.count(); i += 4)
                //         {
                //             if (!bRelative)
                //             {
                //                 last = Vec2f(numbers[i + 2], numbers[i + 3]);
                //                 lastHandle = Vec2f(numbers[i], numbers[i + 1]);
                //             }
                //             else
                //             {
                //                 last = start + Vec2f(numbers[i + 2], numbers[i + 3]);
                //                 lastHandle = start + Vec2f(numbers[i], numbers[i + 1]);
                //             }
                //             currentPath.quadraticCurveTo(lastHandle, last);
                //         }
                //     }
                //     else if (cmd == 'T' || cmd == 't')
                //     {
                //         bool bRelative = cmd == 't';
                //         Vec2f start = last;
                //         for (int i = 0; i < numbers.count(); i += 2)
                //         {
                //             Vec2f nextLast = !bRelative ? Vec2f(numbers[i], numbers[i + 1]) : start + Vec2f(numbers[i], numbers[i + 1]);
                //             lastHandle = detail::reflect(lastHandle, last);
                //             currentPath.quadraticCurveTo(lastHandle, nextLast);
                //             last = nextLast;
                //         }
                //     }
                //     else if (cmd == 'A' || cmd == 'a')
                //     {
                //         bool bRelative = cmd == 'a';
                //         for (int i = 0; i < numbers.count(); i += 7)
                //         {
                //             last = !bRelative ? Vec2f(numbers[i + 5], numbers[i + 6]) : last + Vec2f(numbers[i + 5], numbers[i + 6]);

                //             Float rads = crunch::toRadians(numbers[i + 2]);
                //             currentPath.arcTo(last, Vec2f(numbers[i], numbers[i + 1]),
                //                               crunch::toRadians(numbers[i + 2]), (bool)numbers[i + 4], (bool)numbers[i + 3]);
                //             lastHandle = currentPath.segmentArray().last()->handleOutAbsolute();
                //         }
                //     }
                //     else if (cmd == 'Z' || cmd == 'z')
                //     {
                //         currentPath.closePath();
                //         last = currentPath.segmentArray().last()->position();
                //         lastHandle = currentPath.segmentArray().last()->handleOutAbsolute();
                //     }
                //     else
                //     {
                //         //@TODO?
                //     }
                // }
                // while (it != end);

                const String & val = (*maybe).valueString();
                Path p = m_document->createPath();
                pushAttributes(_node, _rootNode, p);
                parsePathData(*m_document, p, val);
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
                DynamicArray<Float> numbers(m_document->allocator());
                numbers.reserve(64);
                detail::parseNumbers((*mpoints).valueString().begin(), (*mpoints).valueString().end(), [](char) { return false; }, numbers);
                Path ret = m_document->createPath();
                for (Size i = 0; i < numbers.count(); i += 2)
                {
                    ret.addPoint(Vec2f(numbers[i], numbers[i + 1]));
                }

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
            if (mr)
            {
                Float x = mcx ? coordinatePixels((*mcx).valueString().begin()) : 0;
                Float y = mcy ? coordinatePixels((*mcy).valueString().begin()) : 0;
                Path ret = m_document->createCircle(Vec2f(x, y),
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
            if (mrx && mry)
            {
                Float x = mcx ? coordinatePixels((*mcx).valueString().begin()) : 0;
                Float y = mcy ? coordinatePixels((*mcy).valueString().begin()) : 0;
                Path ret = m_document->createEllipse(Vec2f(x, y),
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
