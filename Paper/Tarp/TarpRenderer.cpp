#include <Paper/Tarp/TarpRenderer.hpp>

#include <GL/gl3w.h>

#define TARP_IMPLEMENTATION_OPENGL
#include <Tarp/Tarp.h>

namespace paper
{
    namespace tarp
    {
        using namespace stick;

        namespace detail
        {
            struct TarpStuff
            {
                tpContext ctx;
                tpStyle style;
            };

            struct TarpRenderData
            {
                tpPath path;
            };
        }

        namespace comps
        {
            using TarpRenderData = brick::Component<ComponentName("TarpRenderData"), detail::TarpRenderData>;
        }

        struct Gl3wInitializer
        {
            Gl3wInitializer() :
                bError(true)
            {
                bError = gl3wInit();
            }

            bool bError;
        };

        static bool ensureGl3w()
        {
            static Gl3wInitializer s_initializer;
            STICK_ASSERT(!s_initializer.bError);
            return s_initializer.bError;
        }

        TarpRenderer::TarpRenderer()
        {

        }

        TarpRenderer::~TarpRenderer()
        {

        }

        Error TarpRenderer::init(Document _doc)
        {
            if (ensureGl3w())
            {
                //@TODO: Better error code
                return Error(ec::InvalidOperation, "Could not initialize opengl", STICK_FILE, STICK_LINE);
            }

            m_tarp = makeUnique<detail::TarpStuff>();

            tpBool error = tpContextInit(&m_tarp->ctx);
            if (error)
            {
                return Error(ec::InvalidOperation,
                             String::formatted("Could not init Tarp context: %s\n",
                                               tpContextErrorMessage(&m_tarp->ctx)),
                             STICK_FILE, STICK_LINE);
            }

            m_tarp->style = tpStyleCreate();

            this->m_document = _doc;

            return Error();
        }

        void TarpRenderer::setViewport(Float _x, Float _y, Float _widthInPixels, Float _heightInPixels)
        {
            m_viewport = Rect(_x, _y, _x + _widthInPixels, _y + _widthInPixels);
        }

        void TarpRenderer::setProjection(const Mat4f & _projection)
        {
            tpSetProjection(&m_tarp->ctx, (const tpMat4 *)&_projection);
        }

        void TarpRenderer::reserveItems(Size _count)
        {
            if (m_document)
            {
                m_document.reserveItems<comps::TarpRenderData>(_count);
            }
        }

        static detail::TarpRenderData & ensureRenderData(Path _path)
        {
            if (!_path.hasComponent<comps::TarpRenderData>())
                _path.set<comps::TarpRenderData>(detail::TarpRenderData{tpPathCreate()});

            return _path.get<comps::TarpRenderData>();
        }

        Error TarpRenderer::drawPath(Path _path, const Mat3f & _transform)
        {
            tpSetTransform(&m_tarp->ctx, (tpMat3 *)&_transform);
            detail::TarpRenderData & rd = ensureRenderData(_path);

            if (_path.fill().is<ColorRGBA>())
            {
                ColorRGBA & col = _path.fill().get<ColorRGBA>();
                tpStyleSetFillColor(m_tarp->style, col.r, col.g, col.b, col.a);
                tpStyleSetFillRule(m_tarp->style, _path.windingRule() == WindingRule::NonZero ? kTpFillRuleNonZero : kTpFillRuleEvenOdd);
            }
            else
            {
                tpStyleRemoveFill(m_tarp->style);
            }

            if (!_path.stroke().is<NoPaint>())
            {
                tpStyleSetStrokeWidth(m_tarp->style, _path.strokeWidth());
                if (_path.stroke().is<ColorRGBA>())
                {
                    ColorRGBA & col = _path.stroke().get<ColorRGBA>();
                    tpStyleSetStrokeColor(m_tarp->style, col.r, col.g, col.b, col.a);
                }

                tpStyleSetMiterLimit(m_tarp->style, _path.miterLimit());

                switch (_path.strokeJoin())
                {
                    case StrokeJoin::Round:
                        tpStyleSetStrokeJoin(m_tarp->style, kTpStrokeJoinRound);
                        break;
                    case StrokeJoin::Miter:
                        tpStyleSetStrokeJoin(m_tarp->style, kTpStrokeJoinMiter);
                        break;
                    case StrokeJoin::Bevel:
                    default:
                        tpStyleSetStrokeJoin(m_tarp->style, kTpStrokeJoinBevel);
                        break;
                }

                switch (_path.strokeCap())
                {
                    case StrokeCap::Round:
                        tpStyleSetStrokeCap(m_tarp->style, kTpStrokeCapRound);
                        break;
                    case StrokeCap::Square:
                        tpStyleSetStrokeCap(m_tarp->style, kTpStrokeCapSquare);
                        break;
                    case StrokeCap::Butt:
                    default:
                        tpStyleSetStrokeCap(m_tarp->style, kTpStrokeCapButt);
                        break;
                }

                auto & da = _path.dashArray();
                if (da.count())
                {
                    tpStyleSetDashArray(m_tarp->style, &da[0], da.count());
                    tpStyleSetDashOffset(m_tarp->style, _path.dashOffset());
                }
                else
                {
                    tpStyleSetDashArray(m_tarp->style, NULL, 0);
                }
            }
            else
            {
                tpStyleRemoveStroke(m_tarp->style);
            }

            tpPathClear(rd.path);
            for(auto & seg : _path.segments())
            {
                Vec2f hi = seg.handleInAbsolute();
                Vec2f ho = seg.handleOutAbsolute();
                tpPathAddSegment(rd.path, hi.x, hi.y, seg.position().x, seg.position().y, ho.x, ho.y);
            }
            if(_path.isClosed())
                tpPathClose(rd.path);

            tpBool err = tpDrawPath(&m_tarp->ctx, rd.path, m_tarp->style);
            if (err) return Error(ec::InvalidOperation, "Failed to draw tarp path", STICK_FILE, STICK_LINE);
            return Error();
        }

        Error TarpRenderer::beginClipping(Path _clippingPath, const Mat3f & _transform)
        {
            STICK_ASSERT(tpSetTransform(&m_tarp->ctx, (tpMat3 *)&_transform));
            detail::TarpRenderData & rd = ensureRenderData(_clippingPath);
            tpBool err = tpBeginClipping(&m_tarp->ctx, rd.path);
            if (err) return Error(ec::InvalidOperation, "Failed to draw tarp clip path", STICK_FILE, STICK_LINE);
            return Error();
        }

        Error TarpRenderer::endClipping()
        {
            tpBool err = tpEndClipping(&m_tarp->ctx);
            if (err) return Error(ec::InvalidOperation, "Failed to draw tarp clip path", STICK_FILE, STICK_LINE);
            return Error();
        }

        Error TarpRenderer::prepareDrawing()
        {
            glViewport(m_viewport.min().x, m_viewport.min().y, m_viewport.width(), m_viewport.height());
            tpPrepareDrawing(&m_tarp->ctx);

            return Error();
        }

        Error TarpRenderer::finishDrawing()
        {
            tpFinishDrawing(&m_tarp->ctx);
            return Error();
        }
    }
}
