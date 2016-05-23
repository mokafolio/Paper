#include <Paper/SVG/SVGExport.hpp>
#include <Paper/Private/Shape.hpp>
#include <Paper/Document.hpp>
#include <Paper/Path.hpp>
#include <Paper/Curve.hpp>
#include <Paper/Group.hpp>
#include <Crunch/Colors.hpp>
#include <Crunch/CommonFunc.hpp>

namespace paper
{
    namespace svg
    {
        using namespace stick;

        SVGExport::SVGExport()
        {

        }

        TextResult SVGExport::exportDocument(const Document & _document)
        {
            m_clipMaskID = 0;
            m_tree = Shrub();
            m_tree.setName("svg");
            m_tree.set("version", "1.1", ValueHint::XMLAttribute);
            m_tree.set("xmlns", "http://www.w3.org/2000/svg", ValueHint::XMLAttribute);
            m_tree.set("xmlns:xlink", "http://www.w3.org/1999/xlink", ValueHint::XMLAttribute);
            m_tree.set("width", _document.width(), ValueHint::XMLAttribute);
            m_tree.set("height", _document.height(), ValueHint::XMLAttribute);

            Shrub * parent = &m_tree;

            //if the document has a transform, we wrap everything in a group
            if (_document.transform() != Mat3f::identity())
            {
                Shrub group("g");
                setTransform(_document, group);
                parent = &m_tree.append(group);
            }

            for (const Item & item : _document.children())
            {
                exportItem(item, *parent, false);
            }

            return exportXML(m_tree);
        }

        void SVGExport::addToDefsNode(Shrub & _node)
        {

        }

        void SVGExport::exportItem(const Item & _item, Shrub & _parentTreeNode, bool _bIsClipMask)
        {
            auto it = _item.get<comps::ItemType>();
            if (it == EntityType::Group)
            {
                exportGroup(Group(_item), _parentTreeNode);
            }
            else if (it == EntityType::Path)
            {
                exportPath(Path(_item), _parentTreeNode, _bIsClipMask, true);
            }
        }

        void SVGExport::setTransform(const Item & _item, Shrub & _node)
        {
            Mat3f tm = _item.transform();
            if (tm != Mat3f::identity())
            {
                _node.set("transform", String::concat("matrix(", toString(tm.element(0, 0)), ", ",
                                                      toString(tm.element(0, 1)), ", ",
                                                      toString(tm.element(1, 0)), ", ",
                                                      toString(tm.element(1, 1)), ", ",
                                                      toString(tm.element(2, 0)), ", ",
                                                      toString(tm.element(2, 1)), ")"), ValueHint::XMLAttribute);
            }
        }

        void SVGExport::exportGroup(const Group & _group, Shrub & _parentTreeNode)
        {
            if(!_group.children().count())
                return;
            Shrub groupNode("g");
            auto it = _group.children().begin();
            if (_group.isClipped())
            {
                Shrub clipMaskNode;
                clipMaskNode.setName("clipPath");
                String idstr = String::concat("clip-", toString((UInt64)m_clipMaskID));
                clipMaskNode.set("id", idstr, ValueHint::XMLAttribute);
                exportItem(*it, clipMaskNode, true);
                addToDefsNode(clipMaskNode);
                groupNode.set("clip-path", String::concat("url(#", idstr, ")"), ValueHint::XMLAttribute);
                m_clipMaskID++;
                it++;
            }
            for (; it != _group.children().end(); ++it)
            {
                exportItem(*it, groupNode, false);
            }
            setTransform(_group, groupNode);
            _parentTreeNode.append(groupNode);
        }

        void SVGExport::addCurveToPathData(const Curve & _curve, String & _currentData, bool _bApplyTransform)
        {
            if (_curve.isLinear())
            {
                Vec2f stp = _curve.segmentTwo().position();
                Vec2f sop = _curve.segmentOne().position();

                //this happens if the curve is part of a compound path
                if (_bApplyTransform)
                {
                    Mat3f transform = _curve.path().transform();
                    stp = transform * stp;
                    sop = transform * sop;
                }

                //relative line to
                _currentData.append(AppendVariadicFlag(), " l", toSVGPoint(stp - sop));
            }
            else
            {
                Vec2f stp = _curve.segmentTwo().position();
                Vec2f sop = _curve.segmentOne().position();
                Vec2f ho = _curve.handleOneAbsolute();
                Vec2f ht = _curve.handleTwoAbsolute();

                //this happens if the curve is part of a compound path
                if (_bApplyTransform)
                {
                    Mat3f transform = _curve.path().transform();
                    stp = transform * stp;
                    sop = transform * sop;
                    ho = transform * ho;
                    ht = transform * ht;
                }

                //relative curve to
                _currentData.append(AppendVariadicFlag(), " c", toSVGPoint(ho - sop), " ", toSVGPoint(ht - sop), " ", toSVGPoint(stp - sop));
            }
        }

        void SVGExport::addPathToPathData(const Path & _path, String & _currentData, bool _bIsCompoundPath)
        {
            const auto & curves = _path.curves();
            if (curves.count())
            {
                Mat3f transform = _path.transform();
                //absolute move to
                Vec2f to = curves[0].segmentOne().position();
                bool bApplyTransform = false;
                //for compound paths we need to transform the segment vertices directly
                if (transform != Mat3f::identity() && _bIsCompoundPath)
                {
                    bApplyTransform = true;
                    to = transform * to;
                }
                _currentData.append(AppendVariadicFlag(), "M", toSVGPoint(to));

                for (const auto & c : curves)
                {
                    addCurveToPathData(c, _currentData, bApplyTransform);
                }
            }
        }

        void SVGExport::exportCurveData(const Path & _path, Shrub & _parentTreeNode, Shrub *& _pn)
        {
            if (_path.isPolygon())
            {
                if (_path.segments().count() > 2)
                {
                    String type = _path.isClosed() ? "polygon" : "polyline";
                    Shrub pathNode(type);
                    SegmentArray::ConstIter it = _path.segments().begin();
                    String points;
                    for (; it != _path.segments().end(); ++it)
                    {
                        if (it != _path.segments().end() - 1)
                            points.append(AppendVariadicFlag(), toSVGPoint((*it).position()), " ");
                        else
                            points.append(toSVGPoint((*it).position()));
                    }

                    pathNode.set("points", points, ValueHint::XMLAttribute);
                    _pn = &_parentTreeNode.append(pathNode);
                }
                else
                {
                    if (_path.segments().count() == 2)
                    {
                        Shrub pathNode("line");
                        //pathNode.set("x1", String::concat(toSVGPoint(_path.segments()[0].position()), " ", toSVGPoint(_path.segments()[1].position())), ValueHint::XMLAttribute);
                        pathNode.set("x1", toString(_path.segments()[0].position().x), ValueHint::XMLAttribute);
                        pathNode.set("y1", toString(_path.segments()[0].position().y), ValueHint::XMLAttribute);
                        pathNode.set("x2", toString(_path.segments()[1].position().x), ValueHint::XMLAttribute);
                        pathNode.set("y2", toString(_path.segments()[1].position().y), ValueHint::XMLAttribute);
                        _pn = &_parentTreeNode.append(pathNode);
                    }
                }
            }
            else
            {
                Shrub pathNode("path");
                String curveString;
                addPathToPathData(_path, curveString, false);
                pathNode.set("d", curveString, ValueHint::XMLAttribute);
                _pn = &_parentTreeNode.append(pathNode);
            }
        }

        void SVGExport::exportPath(const Path & _path, Shrub & _parentTreeNode, bool _bIsClipPath, bool _bMatchShape)
        {   
            Shrub * pn = nullptr;
            bool bIsCompoundPath = _path.children().count();
            if (bIsCompoundPath)
            {
                auto it = _path.children().begin();
                String pathsString;
                addPathToPathData(_path, pathsString, true);
                for (; it != _path.children().end(); ++it)
                {
                    if (it != _path.children().begin())
                        pathsString.append(" ");
                    addPathToPathData(Path(*it), pathsString, true);
                }
                Shrub pathNode("path");
                pathNode.set("d", pathsString, ValueHint::XMLAttribute);
                pn = &_parentTreeNode.append(pathNode);
            }
            else
            {
                if (_bMatchShape)
                {
                    //this does shape matching
                    detail::Shape shape(_path);
                    detail::ShapeType shapeType = shape.shapeType();
                    if (shapeType == detail::ShapeType::Circle)
                    {
                        Shrub & circleNode = _parentTreeNode.append("circle");
                        circleNode.set("cx", shape.circle().position.x, ValueHint::XMLAttribute);
                        circleNode.set("cy", shape.circle().position.y, ValueHint::XMLAttribute);
                        circleNode.set("r", shape.circle().radius, ValueHint::XMLAttribute);
                        pn = &circleNode;

                    }
                    else if (shapeType == detail::ShapeType::Ellipse)
                    {
                        Shrub & ellipseNode = _parentTreeNode.append("ellipse");
                        ellipseNode.set("cx", shape.ellipse().position.x, ValueHint::XMLAttribute);
                        ellipseNode.set("cy", shape.ellipse().position.y, ValueHint::XMLAttribute);
                        ellipseNode.set("rx", shape.ellipse().size.x * 0.5, ValueHint::XMLAttribute);
                        ellipseNode.set("ry", shape.ellipse().size.y * 0.5, ValueHint::XMLAttribute);
                        pn = &ellipseNode;
                    }
                    else if (shapeType == detail::ShapeType::Rectangle)
                    {
                        Shrub & rectangleNode = _parentTreeNode.append("rect");
                        rectangleNode.set("x", shape.rectangle().position.x - shape.rectangle().size.x * 0.5, ValueHint::XMLAttribute);
                        rectangleNode.set("y", shape.rectangle().position.y - shape.rectangle().size.y * 0.5, ValueHint::XMLAttribute);
                        rectangleNode.set("width", shape.rectangle().size.x, ValueHint::XMLAttribute);
                        rectangleNode.set("height", shape.rectangle().size.y, ValueHint::XMLAttribute);

                        if (shape.rectangle().cornerRadius.x != 0)
                        {
                            rectangleNode.set("rx", shape.rectangle().cornerRadius.x, ValueHint::XMLAttribute);
                        }

                        if (shape.rectangle().cornerRadius.y != 0)
                        {
                            rectangleNode.set("ry", shape.rectangle().cornerRadius.y, ValueHint::XMLAttribute);
                        }

                        pn = &rectangleNode;
                    }
                    else
                    {
                        exportCurveData(_path, _parentTreeNode, pn);
                    }
                }
                else
                {
                    exportCurveData(_path, _parentTreeNode, pn);
                }
            }

            if (pn && !_bIsClipPath)
            {
                setTransform(_path, *pn);
                applyStyle(_path, *pn);
            }
        }

        void SVGExport::applyStyle(const Item & _item, Shrub & _node)
        {
            //We dont export the name for now: items with the same name will break valid SVG as the name has to be unique in that case.
            //Additionally valid svg names can't be arbitrary strings. I don't think there is a good way of dealing with this
            //so we don't for now.
            /*if(_item.name().length())
            {
                _node.addChild("id", _item.name(), ValueHint::XMLAttribute);
            }*/
            if (!_item.isVisible())
            {
                _node.set("visibility", "hidden", ValueHint::XMLAttribute);
            }

            //TODO: As soon as we support gradients, we should add them to the svg exporter, too
            //fill related things
            if (!_item.hasFill())
            {
                _node.set("fill", "none", ValueHint::XMLAttribute);
            }
            else
            {
                const crunch::ColorRGBA & fillColor = _item.fill();
                _node.set("fill", colorToHexCSSString(crunch::toRGB(fillColor)), ValueHint::XMLAttribute);
                if (fillColor.a < 1.0)
                {
                    _node.set("fill-opacity", fillColor.a, ValueHint::XMLAttribute);
                }

                if (_item.windingRule() == WindingRule::NonZero)
                {
                    _node.set("fill-rule", "nonzero", ValueHint::XMLAttribute);
                }
                else
                {
                    _node.set("fill-rule", "evenodd", ValueHint::XMLAttribute);
                }
            } 

            //stroke related things
            if (!_item.hasStroke())
            {
                _node.set("stroke", "none", ValueHint::XMLAttribute);
            }
            else
            {
                const crunch::ColorRGBA & strokeColor = _item.stroke();
                _node.set("stroke", colorToHexCSSString(crunch::toRGB(strokeColor)), ValueHint::XMLAttribute);
                if (strokeColor.a < 1.0)
                {
                    _node.set("stroke-opacity", strokeColor.a, ValueHint::XMLAttribute);
                }

                _node.set("stroke-width", _item.strokeWidth(), ValueHint::XMLAttribute);

                StrokeCap cap = _item.strokeCap();
                if (cap == StrokeCap::Butt)
                {
                    _node.set("stroke-linecap", "butt", ValueHint::XMLAttribute);
                }
                else if (cap == StrokeCap::Square)
                {
                    _node.set("stroke-linecap", "square", ValueHint::XMLAttribute);
                }
                else if (cap == StrokeCap::Round)
                {
                    _node.set("stroke-linecap", "round", ValueHint::XMLAttribute);
                }

                StrokeJoin join = _item.strokeJoin();
                if (join == StrokeJoin::Bevel)
                {
                    _node.set("stroke-linejoin", "bevel", ValueHint::XMLAttribute);
                }
                else if (join == StrokeJoin::Miter)
                {
                    _node.set("stroke-linejoin", "miter", ValueHint::XMLAttribute);
                }
                else if (join == StrokeJoin::Round)
                {
                    _node.set("stroke-linejoin", "round", ValueHint::XMLAttribute);
                }

                _node.set("stroke-miterlimit", _item.miterLimit(), ValueHint::XMLAttribute);

                const auto & dashArray = _item.dashArray();
                if (dashArray.count())
                {
                    String dashString;
                    auto it = dashArray.begin();
                    for (; it != dashArray.end(); ++it)
                    {
                        if (it != dashArray.begin())
                            dashString.append(AppendVariadicFlag(), ", ", toString(*it));
                        else
                            dashString.append(toString(*it));
                    }
                    _node.set("stroke-dasharray", dashString, ValueHint::XMLAttribute);
                }

                _node.set("stroke-dashoffset", _item.dashOffset(), ValueHint::XMLAttribute);

                if (!_item.isScalingStroke())
                {
                    _node.set("vector-effect", "non-scaling-stroke", ValueHint::XMLAttribute);
                }
            }
        }

        String SVGExport::colorToHexCSSString(const crunch::ColorRGB & _color)
        {
            UInt32 r = crunch::min(static_cast<UInt32>(255), crunch::max(static_cast<UInt32>(0),
                                   static_cast<UInt32>(_color.r * 255)));
            UInt32 g = crunch::min(static_cast<UInt32>(255), crunch::max(static_cast<UInt32>(0),
                                   static_cast<UInt32>(_color.g * 255)));
            UInt32 b = crunch::min(static_cast<UInt32>(255), crunch::max(static_cast<UInt32>(0),
                                   static_cast<UInt32>(_color.b * 255)));

            return String::concat("#", toHexString(r, 2), toHexString(g, 2), toHexString(b, 2));
        }

        String SVGExport::toSVGPoint(const Vec2f & _p)
        {
            return String::concat(toString(_p.x), ",", toString(_p.y));
        }
    }
}
