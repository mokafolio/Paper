#ifndef PAPER_OPENGL_GLRENDERER_HPP
#define PAPER_OPENGL_GLRENDERER_HPP

#include <Paper/RenderInterface.hpp>
#include <Crunch/Matrix4.hpp>

namespace paper
{
    namespace opengl
    {
        class STICK_API GLRenderer : public RenderInterface
        {
        public:

            GLRenderer();

            GLRenderer(const Document & _doc);

            ~GLRenderer();

            void setViewport(Float _widthInPixels, Float _heightInPixels) override;

            void setTransform(const Mat3f & _transform) override;

        protected:

            using PathArray = stick::DynamicArray<Path>;
            using PathGeometryArray = stick::DynamicArray<Vec2f>;

            struct PathStyle
            {
                PathStyle()
                {
                    //no init for speed.
                }

                PathStyle(const Item & _item) :
                    strokeWidth(0),
                    bHasFill(false)
                {
                    bHasFill = _item.hasFill();
                    if (bHasFill)
                    {
                        fillColor = _item.fill();
                    }
                    if (_item.hasStroke())
                    {
                        strokeColor = _item.stroke();
                        strokeWidth = _item.strokeWidth();
                        miterLimit = _item.miterLimit();
                        cap = _item.strokeCap();
                        join = _item.strokeJoin();
                        dashArray = _item.dashArray();
                        dashOffset = _item.dashOffset();
                    }
                }

                bool bHasFill;
                ColorRGBA fillColor;
                ColorRGBA strokeColor;
                Float strokeWidth;
                Float miterLimit;
                StrokeCap cap;
                StrokeJoin join;
                DashArray dashArray;
                Float dashOffset;
            };

            struct RenderCacheData
            {
                PathGeometryArray fillVertices;
                stick::DynamicArray<bool> joins;
                PathGeometryArray boundsVertices;
                PathGeometryArray strokeVertices;
                PathGeometryArray strokeBoundsVertices;
                crunch::Mat4f transformProjection;
                stick::UInt32 strokeVertexDrawMode;
            };

            using RenderCache = brick::Component<ComponentName("RenderCache"), RenderCacheData>;

            //these have to be implemented
            stick::Error drawPath(const Path & _path) override;
            stick::Error beginClipping(const Path & _clippingPath) override;
            stick::Error endClipping(const Path & _clippingPath) override;

            stick::Error drawPathImpl(const Path & _path, bool _bIsClippingPath);
            stick::Error generateClippingMask(const Path & _clippingPath, bool _bIsRebuilding);

            //these can be implemented
            stick::Error prepareDrawing() override;
            stick::Error finishDrawing() override;

            RenderCacheData & updateRenderCache(const Path & _path, const PathStyle & _style, bool _bIsClipping);
            stick::Error drawFillEvenOdd(const PathArray & _paths,
                                         const PathGeometryArray & _boundsVertices,
                                         crunch::Mat4f * _boundsTransform,
                                         stick::UInt32 _targetStencilBufferMask,
                                         stick::UInt32 _clippingPlaneToTestAgainst,
                                         const PathStyle & _style,
                                         bool _bIsClippingPath);

            stick::Error drawFillNonZero(const PathArray & _paths,
                                         const PathGeometryArray & _boundsVertices,
                                         crunch::Mat4f * _boundsTransform,
                                         stick::UInt32 _targetStencilBufferMask,
                                         stick::UInt32 _clippingPlaneToTestAgainst,
                                         const PathStyle & _style,
                                         bool _bIsClippingPath);

            stick::Error drawStroke(const RenderCacheData & _geom, const PathStyle & _style, stick::UInt32 _clippingPlaneToTestAgainst);

            struct StencilPlanes
            {
                stick::UInt32 targetStencilMask;
                stick::UInt32 clippingPlaneToTestAgainst;
            };
            StencilPlanes prepareStencilPlanes(bool _bIsClippingPath);

            stick::UInt32 m_program;
            stick::UInt32 m_vao;
            stick::UInt32 m_vbo;
            stick::UInt32 m_currentClipStencilPlane;
            stick::DynamicArray<Path> m_clippingMaskStack;
            bool m_bIsInitialized;
            bool m_bCanSwapStencilPlanesWhenEnding;
            bool m_bIsClipping;
            crunch::Vec2f m_viewport;
            crunch::Mat4f m_projection;
            crunch::Mat4f m_transform;
        };
    }
}

#endif //PAPER_OPENGL_GLRENDERER_HPP
