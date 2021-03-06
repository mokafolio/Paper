#ifndef PAPER_OPENGL_GLRENDERER_HPP
#define PAPER_OPENGL_GLRENDERER_HPP

#include <Paper/RenderInterface.hpp>
#include <Crunch/Matrix4.hpp>
#include <Crunch/StringConversion.hpp>

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

            void setProjection(const Mat4f & _projection) override;

            void reserveItems(stick::Size _count) override;

        protected:

            using PathArray = stick::DynamicArray<Path>;
            using PathGeometryArray = stick::DynamicArray<Vec2f>;
            struct TextureVertex
            {
                Vec2f vertex;
                Float textureCoord;
            };
            using TextureGeometryArray = stick::DynamicArray<TextureVertex>;

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
                        fill = _item.fill();
                    }
                    if (_item.hasStroke())
                    {
                        stroke = _item.stroke();
                        strokeWidth = _item.strokeWidth();
                        miterLimit = _item.miterLimit();
                        cap = _item.strokeCap();
                        join = _item.strokeJoin();
                        dashArray = _item.dashArray();
                        dashOffset = _item.dashOffset();
                    }
                }

                bool bHasFill;
                Paint fill;
                Paint stroke;
                Float strokeWidth;
                Float miterLimit;
                StrokeCap cap;
                StrokeJoin join;
                DashArray dashArray;
                Float dashOffset;
            };

            struct Texture
            {
                Texture();
                ~Texture();

                stick::UInt32 glTexture;
            };

            struct GradientCacheData
            {
                Texture texture;
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
                TextureGeometryArray textureVertices;
            };

            using RenderCache = brick::Component<ComponentName("RenderCache"), RenderCacheData>;
            using GradientCache = brick::Component<ComponentName("GradientCache"), GradientCacheData>;

            //these have to be implemented
            stick::Error drawPath(const Path & _path, const Mat3f * _transform) override;
            stick::Error beginClipping(const Path & _clippingPath, const Mat3f * _transform) override;
            stick::Error endClipping(const Path & _clippingPath, const Mat3f * _transform) override;

            stick::Error drawPathImpl(const Path & _path, bool _bIsClippingPath, const Mat3f * _transform);
            stick::Error generateClippingMask(const Path & _clippingPath, bool _bIsRebuilding, const Mat3f * _transform);

            //these can be implemented
            stick::Error prepareDrawing() override;
            stick::Error finishDrawing() override;

            RenderCacheData & updateRenderCache(const Path & _path, const PathStyle & _style, bool _bIsClipping);

            RenderCacheData & recursivelyDrawEvenOddPath(const Path & _path, const Mat4f * _tp,
                    const PathStyle & _style, bool _bIsClipping);

            RenderCacheData & recursivelyDrawNonZeroPath(const Path & _path, const Mat4f * _tp,
                    const PathStyle & _style, bool _bIsClipping);

            stick::Error recursivelyDrawStroke(const Path & _path, const Mat4f * _tp,
                                               const PathStyle & _style, stick::UInt32 _clippingPlaneToTestAgainst);

            stick::Error drawFillEvenOdd(const Path & _path,
                                         const crunch::Mat4f * _tp,
                                         stick::UInt32 _targetStencilBufferMask,
                                         stick::UInt32 _clippingPlaneToTestAgainst,
                                         const PathStyle & _style,
                                         bool _bIsClippingPath);

            stick::Error drawFillNonZero(const Path & _path,
                                         const crunch::Mat4f * _tp,
                                         stick::UInt32 _targetStencilBufferMask,
                                         stick::UInt32 _clippingPlaneToTestAgainst,
                                         const PathStyle & _style,
                                         bool _bIsClippingPath);


            stick::Error drawStroke(const Path & _path, const Mat4f * _tp, const PathStyle & _style, stick::UInt32 _clippingPlaneToTestAgainst);


            stick::Error drawFilling(const Path & _path,
                                     RenderCacheData & _cache,
                                     const crunch::Mat4f * _tp,
                                     const PathStyle & _style,
                                     bool _bStroke);


            struct StencilPlanes
            {
                stick::UInt32 targetStencilMask;
                stick::UInt32 clippingPlaneToTestAgainst;
            };
            StencilPlanes prepareStencilPlanes(bool _bIsClippingPath);

            stick::UInt32 m_program;
            stick::UInt32 m_programTexture;
            stick::UInt32 m_vao;
            stick::UInt32 m_vaoTexture;
            stick::UInt32 m_vbo;
            stick::UInt32 m_vboTexture;
            stick::UInt32 m_currentClipStencilPlane;
            stick::DynamicArray<Path> m_clippingMaskStack;
            bool m_bIsInitialized;
            bool m_bCanSwapStencilPlanesWhenEnding;
            bool m_bIsClipping;
            Vec2f m_viewport;
            Mat4f m_projection;
            bool m_bHasCustomProjection;
            Mat4f m_transform;
            Mat4f m_transformProjection;
            Vec2f m_transformScale;
            bool m_bTransformScaleChanged;
        };
    }
}

#endif //PAPER_OPENGL_GLRENDERER_HPP
