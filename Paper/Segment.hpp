#ifndef PAPER_SEGMENT_HPP
#define PAPER_SEGMENT_HPP

#include <Paper/Path.hpp>

namespace paper
{
    class Path;
    class Curve;

    class STICK_API Segment
    {
        friend class Path;
        friend class Curve;
        
    public:

        void setPosition(const Vec2f & _pos);

        void setHandleIn(const Vec2f & _pos);

        void setHandleOut(const Vec2f & _pos);

        const Vec2f & position() const;

        const Vec2f & handleIn() const;

        const Vec2f & handleOut() const;

        Vec2f handleInAbsolute() const;
            
        Vec2f handleOutAbsolute() const;

        Curve * curveIn();

        const Curve * curveIn() const;

        Curve * curveOut();

        const Curve * curveOut() const;

        bool isLinear() const;

        void remove();


    private:

        Segment(const Path & _path, const Vec2f & _pos, const Vec2f & _handleIn, const Vec2f & _handleOut, stick::Size _idx);

        void transform(const Mat3f & _transform);


        Path m_path;
        Vec2f m_position;
        Vec2f m_handleIn;
        Vec2f m_handleOut;
        stick::Size m_index;
    };
}

#endif //PAPER_SEGMENT_HPP
