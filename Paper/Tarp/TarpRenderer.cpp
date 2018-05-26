#include <Paper/Tarp/TarpRenderer.hpp>

#include <GL/gl3w.h>

#define TARP_IMPLEMENTATION_OPENGL
#include <Tarp/Tarp.h>

namespace paper
{
    namespace tarp
    {
        using namespace stick;

        using tpSegmentArray =  stick::DynamicArray<tpSegment>;

        namespace detail
        {
            struct TarpStuff
            {
                tpContext ctx;
                tpStyle style;
                //@TODO: allow to pass in an allocator for this
                tpSegmentArray tmpSegmentBuffer;
            };

            struct TarpRenderData
            {
                TarpRenderData()
                {
                    path = tpPathInvalidHandle();
                }

                ~TarpRenderData()
                {
                    tpPathDestroy(path);
                }

                tpPath path;
            };

            struct TarpGradientData
            {
                TarpGradientData()
                {
                    gradient = tpGradientInvalidHandle();
                }

                ~TarpGradientData()
                {
                    tpGradientDestroy(gradient);
                }

                tpGradient gradient;
            };
        }

        namespace comps
        {
            using TarpRenderData = brick::Component<ComponentName("TarpRenderData"), detail::TarpRenderData>;
            using TarpGradientData = brick::Component<ComponentName("TarpGradientData"), detail::TarpGradientData>;
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
            if (m_tarp)
            {
                tpStyleDestroy(m_tarp->style);
                tpContextDestroy(m_tarp->ctx);
            }
        }

        Error TarpRenderer::init(Document _doc)
        {
            if (ensureGl3w())
            {
                //@TODO: Better error code
                return Error(ec::InvalidOperation, "Could not initialize opengl", STICK_FILE, STICK_LINE);
            }

            m_tarp = makeUnique<detail::TarpStuff>();

            m_tarp->ctx = tpContextCreate();
            if (!tpContextIsValidHandle(m_tarp->ctx))
            {
                return Error(ec::InvalidOperation,
                             String::formatted("Could not init Tarp context: %s\n",
                                               tpContextErrorMessage(m_tarp->ctx)),
                             STICK_FILE, STICK_LINE);
            }

            m_tarp->style = tpStyleCreate();

            this->m_document = _doc;

            return Error();
        }

        void TarpRenderer::setViewport(Float _x, Float _y, Float _widthInPixels, Float _heightInPixels)
        {
            m_viewport = Rect(_x, _y, _x + _widthInPixels, _y + _heightInPixels);
        }

        void TarpRenderer::setProjection(const Mat4f & _projection)
        {
            tpSetProjection(m_tarp->ctx, (const tpMat4 *)&_projection);
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
            auto & ret = _path.ensureComponent<comps::TarpRenderData>();
            if (!tpPathIsValidHandle(ret.path))
                ret.path = tpPathCreate();

            return ret;
        }

        static void toTarpSegments(tpSegmentArray & _tmpData, Path _path, const Mat3f * _transform)
        {
            _tmpData.clear();
            if (!_transform)
            {
                for (auto & seg : _path.segments())
                {
                    Vec2f hi = seg.handleInAbsolute();
                    Vec2f ho = seg.handleOutAbsolute();
                    _tmpData.append((tpSegment)
                    {
                        {hi.x, hi.y},
                        {seg.position().x, seg.position().y},
                        {ho.x, ho.y}
                    });
                }
            }
            else
            {
                //tarp does not support per contour transforms, so we need to bring child paths segments
                //to path space before adding it as a contour!
                for (auto & seg : _path.segments())
                {
                    Vec2f hi = *_transform * seg.handleInAbsolute();
                    Vec2f pos = *_transform * seg.position();
                    Vec2f ho = *_transform * seg.handleOutAbsolute();
                    _tmpData.append((tpSegment)
                    {
                        {hi.x, hi.y},
                        {pos.x, pos.y},
                        {ho.x, ho.y}
                    });
                }
            }
        }

        static void recursivelyUpdateTarpPath(tpSegmentArray & _tmpData, Path _path, tpPath _tarpPath, const Mat3f * _transform, UInt32 & _contourIndex)
        {
            if (_path.hasComponent<paper::comps::FillGeometryDirtyFlag>())
            {
                _path.removeComponent<paper::comps::FillGeometryDirtyFlag>();

                toTarpSegments(_tmpData, _path, _transform);
                tpPathSetContour(_tarpPath, _contourIndex, &_tmpData[0], _tmpData.count(), (tpBool)_path.isClosed());
            }

            _contourIndex += 1;

            for (auto & c : _path.children())
            {
                STICK_ASSERT(c.itemType() == EntityType::Path);
                Path p = brick::reinterpretEntity<Path>(c);

                const Mat3f * t = _transform;
                Mat3f tmp;
                if (p.hasTransform())
                {
                    if (_transform)
                    {
                        tmp = *_transform * p.transform();
                        t = &tmp;
                    }
                    else
                    {
                        t = &p.transform();
                    }
                }

                recursivelyUpdateTarpPath(_tmpData, p, _tarpPath, t, _contourIndex);
            }
        }

        static void updateTarpPath(tpSegmentArray & _tmpData, Path _path, tpPath _tarpPath, const Mat3f * _transform)
        {
            UInt32 contourIndex = 0;
            recursivelyUpdateTarpPath(_tmpData, _path, _tarpPath, _transform, contourIndex);

            // remove contours that are not used anymore
            if (contourIndex < tpPathContourCount(_tarpPath))
            {
                for (Size i = tpPathContourCount(_tarpPath) - 1; i >= contourIndex; --i)
                {
                    tpPathRemoveContour(_tarpPath, i);
                }
            }
        }

        static detail::TarpGradientData & updateTarpGradient(BaseGradient _grad)
        {
            detail::TarpGradientData & gd = _grad.ensureComponent<comps::TarpGradientData>();
            if (!tpGradientIsValidHandle(gd.gradient))
            {
                gd.gradient = tpGradientCreateLinear(0, 0, 0, 0);
            }

            auto & dfs = _grad.get<paper::comps::GradientDirtyFlags>();
            if (dfs.bPositionsDirty)
            {
                dfs.bPositionsDirty = false;
                tpGradientSetPositions(gd.gradient, _grad.origin().x, _grad.origin().y, _grad.destination().x, _grad.destination().y);
            }
            if (dfs.bStopsDirty)
            {
                dfs.bStopsDirty = false;
                tpGradientClearColorStops(gd.gradient);
                for (auto & stop : _grad.stops())
                {
                    tpGradientAddColorStop(gd.gradient, stop.color.r, stop.color.g, stop.color.b, stop.color.a, stop.offset);
                }
            }
            return gd;
        }

        Error TarpRenderer::drawPath(Path _path, const Mat3f & _transform)
        {
            detail::TarpRenderData & rd = ensureRenderData(_path);

            if (_path.fill().is<ColorRGBA>())
            {
                ColorRGBA & col = _path.fill().get<ColorRGBA>();
                tpStyleSetFillColor(m_tarp->style, col.r, col.g, col.b, col.a);
                tpStyleSetFillRule(m_tarp->style, _path.windingRule() == WindingRule::NonZero ? kTpFillRuleNonZero : kTpFillRuleEvenOdd);
            }
            else if (_path.fill().is<LinearGradient>())
            {
                detail::TarpGradientData & gd = updateTarpGradient(_path.fill().get<LinearGradient>());
                tpStyleSetFillGradient(m_tarp->style, gd.gradient);
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
                else if (_path.stroke().is<LinearGradient>())
                {
                    detail::TarpGradientData & gd = updateTarpGradient(_path.stroke().get<LinearGradient>());
                    tpStyleSetStrokeGradient(m_tarp->style, gd.gradient);
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

            // tpPathClear(rd.path);
            // recursivelyAddContours(m_tarp->tmpSegmentBuffer, _path, rd.path, nullptr);
            updateTarpPath(m_tarp->tmpSegmentBuffer, _path, rd.path, nullptr);

            // printf("%f", _transform[0][0]);

            tpTransform trans = tpTransformMake(_transform[0][0], _transform[1][0], _transform[2][0],
                                                _transform[0][1], _transform[1][1], _transform[2][1]);

            tpSetTransform(m_tarp->ctx, &trans);
            tpBool err = tpDrawPath(m_tarp->ctx, rd.path, m_tarp->style);

            if (err) return Error(ec::InvalidOperation, "Failed to draw tarp path", STICK_FILE, STICK_LINE);
            return Error();
        }

        Error TarpRenderer::beginClipping(Path _clippingPath, const Mat3f & _transform)
        {
            detail::TarpRenderData & rd = ensureRenderData(_clippingPath);

            updateTarpPath(m_tarp->tmpSegmentBuffer, _clippingPath, rd.path, nullptr);

            tpTransform trans = tpTransformMake(_transform[0][0], _transform[1][0], _transform[2][0],
                                                _transform[0][1], _transform[1][1], _transform[2][1]);

            tpSetTransform(m_tarp->ctx, &trans);
            tpBool err = tpBeginClipping(m_tarp->ctx, rd.path);
            if (err) return Error(ec::InvalidOperation, "Failed to draw tarp clip path", STICK_FILE, STICK_LINE);
            return Error();
        }

        Error TarpRenderer::endClipping()
        {
            tpBool err = tpEndClipping(m_tarp->ctx);
            if (err) return Error(ec::InvalidOperation, "Failed to draw tarp clip path", STICK_FILE, STICK_LINE);
            return Error();
        }

        Error TarpRenderer::prepareDrawing()
        {
            glViewport(m_viewport.min().x, m_viewport.min().y, m_viewport.width(), m_viewport.height());
            tpPrepareDrawing(m_tarp->ctx);

            return Error();
        }

        Error TarpRenderer::finishDrawing()
        {
            tpFinishDrawing(m_tarp->ctx);
            return Error();
        }
    }
}
