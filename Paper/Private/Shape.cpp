#include <Paper/Private/Shape.hpp>
#include <Paper/Path.hpp>
#include <Paper/Segment.hpp>
#include <Paper/Curve.hpp>
#include <Crunch/CommonFunc.hpp>

namespace paper
{
    namespace detail
    {
        Shape::Shape() :
            m_type(ShapeType::None)
        {

        }

        Shape::Shape(const Path & _path) :
            m_type(ShapeType::None)
        {
            const auto & curves = _path.curves();
            const auto & segments = _path.segments();
            if (curves.count() == 4 &&
                    curves[0].isArc() &&
                    curves[1].isArc() &&
                    curves[2].isArc() &&
                    curves[3].isArc())
            {
                if (crunch::isClose(crunch::length(segments[0].position() - segments[2].position()) -
                                    crunch::length(segments[1].position() - segments[3].position()), (Float)0, PaperConstants::epsilon()))
                {
                    m_type = ShapeType::Circle;
                    m_data.circle.position = _path.localBounds().center();
                    m_data.circle.radius = crunch::distance(segments[0].position(), segments[2].position()) * 0.5;
                }
                else
                {
                    if (crunch::isClose(crunch::distance(segments[0].position(), segments[2].position()), (Float)0, PaperConstants::epsilon()))
                    {
                        m_type = ShapeType::Circle;
                        m_data.circle.position = _path.localBounds().center();
                        m_data.circle.radius = crunch::distance(segments[0].position(), segments[2].position()) * 0.5;
                    }
                    else
                    {
                        m_type = ShapeType::Ellipse;
                        m_data.ellipse.position = _path.localBounds().center();
                        m_data.ellipse.size = Vec2f(crunch::distance(segments[0].position(), segments[2].position()),
                                                    crunch::distance(segments[1].position(), segments[3].position()));
                    }
                }
            }
            else if (_path.isPolygon() &&
                     curves.count() == 4 &&
                     curves[0].isColinear(curves[2]) &&
                     curves[1].isColinear(curves[3]) &&
                     curves[1].isOrthogonal(curves[0]))
            {
                m_type = ShapeType::Rectangle;
                m_data.rectangle.position = _path.localBounds().center();
                m_data.rectangle.size = Vec2f(crunch::distance(segments[0].position(),
                                              segments[1].position()),
                                              crunch::distance(segments[1].position(),
                                                      segments[2].position()));
                m_data.rectangle.cornerRadius = Vec2f(0);
            }
            else if (curves.count() == 8 &&
                     curves[1].isArc() &&
                     curves[3].isArc() &&
                     curves[5].isArc() &&
                     curves[7].isArc() &&
                     curves[0].isColinear(curves[4]) &&
                     curves[2].isColinear(curves[6]))
            {

                //rounded rect
                m_type = ShapeType::Rectangle;

                m_data.rectangle.position = _path.localBounds().center();
                m_data.rectangle.size = Vec2f(crunch::distance(segments[7].position(),
                                              segments[2].position()),
                                              crunch::distance(segments[0].position(),
                                                      segments[5].position()));
                m_data.rectangle.cornerRadius = (m_data.rectangle.size - Vec2f(crunch::distance(segments[0].position(),
                                                 segments[1].position()),
                                                 crunch::distance(segments[2].position(),
                                                         segments[3].position()))) * 0.5;

            }

        }

        ShapeType Shape::shapeType() const
        {
            return m_type;
        }

        const Shape::Circle & Shape::circle() const
        {
            return m_data.circle;
        }

        const Shape::Ellipse & Shape::ellipse() const
        {
            return m_data.ellipse;
        }

        const Shape::Rectangle & Shape::rectangle() const
        {
            return m_data.rectangle;
        }
    }
}
