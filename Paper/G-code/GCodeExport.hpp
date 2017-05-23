#ifndef PAPER_GCODE_GCODEEXPORT_HPP
#define PAPER_GCODE_GCODEEXPORT_HPP

namespace paper
{
    namespace gcode
    {
        class STICK_LOCAL GCodeExport
        {
        public:

            GCodeExport();

            stick::TextResult exportDocument(const Document & _document);
        };
    }
}

#endif //PAPER_GCODE_GCODEEXPORT_HPP
