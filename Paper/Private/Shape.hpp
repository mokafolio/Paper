#ifndef PAPER_PRIVATE_SHAPE_HPP
#define PAPER_PRIVATE_SHAPE_HPP

#include <Paper/BasicTypes.hpp>

namespace paper
{
    class Path;

    namespace detail
    {
        STICK_LOCAL_ENUM_CLASS(ShapeType)
        {
            None,
            Rectangle, //also rounded rect
            Circle,
            Ellipse
        };

        class STICK_LOCAL Shape
        {
        public:

            struct Circle
            {
                Vec2f position;
                Float radius;
            };

            struct Ellipse
            {
                Vec2f position;
                Vec2f size;
            };

            struct Rectangle
            {
                Vec2f position;
                Vec2f size;
                Vec2f cornerRadius;
            };

            Shape();

            Shape(const Path & _path);

            ShapeType shapeType() const;

            const Circle & circle() const;

            const Ellipse & ellipse() const;

            const Rectangle & rectangle() const;


        private:

            ShapeType m_type;

            union Data
            {
                Data()
                {

                }

                Circle circle;
                Ellipse ellipse;
                Rectangle rectangle;
            };

            Data m_data;
        };
    }
}

#endif //PAPER_PRIVATE_SHAPE_HPP
