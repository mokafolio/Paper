#include <Paper/SVG/SVGImport.hpp>
#include <Paper/Document.hpp>

namespace paper
{
    namespace svg
    {
        using namespace stick;

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
            auto shrubRes = parseXML(_svg);
            if (!shrubRes)
                return shrubRes.error();
            Shrub & svg = shrubRes.get();
            auto maybe = svg.child("defs");
            Shrub * defs = nullptr;
            if (maybe) defs = &*maybe;

            Group ret = m_document->createGroup();
            Error err;
            recursivelyImportNode(svg, ret, defs, err);
            if (err) return err;
            return ret;
        }

        GroupResult SVGImport::importFromFile(const URI & _uri)
        {

        }

        void SVGImport::recursivelyImportNode(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, Error & _error)
        {
            if (_node.name() == "svg")
            {
                importGroup(_node, _parentToAddTo, _defsNode, _error);
            }
            else if (_node.name() == "g")
            {
                importGroup(_node, _parentToAddTo, _defsNode, _error);
            }
            else if (_node.name() == "rect")
            {
                importRectangle(_node, _parentToAddTo, _defsNode, _error);
            }
            else if (_node.name() == "circle")
            {
                importCircle(_node, _parentToAddTo, _defsNode, _error);
            }
            else if (_node.name() == "ellipse")
            {
                importEllipse(_node, _parentToAddTo, _defsNode, _error);
            }
            else if (_node.name() == "line")
            {
                importLine(_node, _parentToAddTo, _defsNode, _error);
            }
            else if (_node.name() == "polyline")
            {
                importPolyline(_node, _parentToAddTo, _defsNode, false, _error);
            }
            else if (_node.name() == "polygon")
            {
                importPolygon(_node, _parentToAddTo, _defsNode, _error);
            }
            else if (_node.name() == "path")
            {
                importPath(_node, _parentToAddTo, _defsNode, _error);
            }
            else
            {

            }
        }

        void SVGImport::importGroup(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, Error & _error)
        {
            Group grp = m_document->createGroup();
            for (auto & child : _node)
            {
                if (child.valueHint() != ValueHint::XMLAttribute)
                {
                    recursivelyImportNode(child, grp, _defsNode, _error);
                    if (_error) break;
                }
            }
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

        void SVGImport::importPath(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, Error & _error)
        {
            auto maybe = _node.child("d");
            // MmLlHhVvCcSsQqTtAaZz
            if (maybe)
            {
                const String & val = (*maybe).value();
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

                    }
                    else if (cmd == 'Z' || cmd == 'z')
                    {

                    }
                    else
                    {
                        //unknown command??
                        //it = advanceToNextCommand(it, end);
                    }
                }
                while (it != end);
            }
            else
            {
                //TODO: Error
            }
        }

        void SVGImport::importPolygon(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, Error & _error)
        {

        }

        void SVGImport::importPolyline(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, bool _bIsPolygon, Error & _error)
        {

        }

        void SVGImport::importCircle(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, Error & _error)
        {

        }

        void SVGImport::importEllipse(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, Error & _error)
        {

        }

        void SVGImport::importRectangle(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, Error & _error)
        {

        }

        void SVGImport::importLine(const Shrub & _node, Group & _parentToAddTo, Shrub * _defsNode, Error & _error)
        {

        }

        void SVGImport::importText()
        {

        }

        void SVGImport::importSymbol()
        {

        }

        void SVGImport::importUse()
        {

        }
    }
}
