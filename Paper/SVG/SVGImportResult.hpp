#ifndef PAPER_SVG_SVGIMPORTRESULT_HPP
#define PAPER_SVG_SVGIMPORTRESULT_HPP

#include <Stick/Error.hpp>
#include <Paper/Group.hpp>

namespace paper
{
    namespace svg
    {
        class STICK_API SVGImportResult
        {
        public:

            SVGImportResult();

            SVGImportResult(const stick::Error & _err);

            SVGImportResult(Group _grp, Float _width, Float _height, const stick::Error & _err = stick::Error());

            explicit operator bool() const;

            Group group() const;

            Float width() const;

            Float height() const;

            const stick::Error & error() const;

        private:

            Group m_group;
            Float m_width, m_height;
            stick::Error m_error;
        };
    }
}

#endif //PAPER_SVG_SVGIMPORTRESULT_HPP
