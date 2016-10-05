#include <Paper/SVG/SVGImportResult.hpp>

namespace paper
{
    namespace svg
    {
        using namespace stick;

        SVGImportResult::SVGImportResult()
        {

        }

        SVGImportResult::SVGImportResult(const Error & _err) :
        m_group(Group()),
        m_width(0),
        m_height(0),
        m_error(_err)
        {

        }

        SVGImportResult::SVGImportResult(Group _grp, Float _width, Float _height, const Error & _err) :
            m_group(_grp),
            m_width(_width),
            m_height(_height),
            m_error(_err)
        {

        }

        SVGImportResult::operator bool() const
        {
            return static_cast<bool>(m_error);
        }

        Group SVGImportResult::group() const
        {
            return m_group;
        }

        Float SVGImportResult::width() const
        {
            return m_width;
        }

        Float SVGImportResult::height() const
        {
            return m_height;
        }

        const Error & SVGImportResult::error() const
        {
            return m_error;
        }
    }
}
