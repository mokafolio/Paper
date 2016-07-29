#include <Paper/OpenGL/GLRenderer.hpp>
#include <Paper/Private/PathFlattener.hpp>
#include <Paper/Private/StrokeTriangulator.hpp>
#include <Paper/Constants.hpp>
#include <Stick/Platform.hpp>
#include <Crunch/MatrixFunc.hpp>
#include <Crunch/StringConversion.hpp>

#if STICK_PLATFORM == STICK_PLATFORM_OSX
#include <OpenGL/OpenGL.h> //for CGL functions
//check if opengl 3+ is available
#if CGL_VERSION_1_3
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#endif //CGL_VERSION_1_3
#elif STICK_PLATFORM == STICK_PLATFORM_LINUX
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#undef None //??? some glx thing
#undef GL_GLEXT_PROTOTYPES
#undef WindingRule //????? where is this a macro on linux?
#endif

#ifdef STICK_DEBUG
#define ASSERT_NO_GL_ERROR(_func) do { _func; \
GLenum err = glGetError(); \
if(err != GL_NO_ERROR) \
{ \
switch(err) \
{ \
case GL_NO_ERROR: \
printf("%s line %i GL_NO_ERROR: No error has been recorded.\n", __FILE__, __LINE__);\
break; \
case GL_INVALID_ENUM: \
printf("%s line %i GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.\n", __FILE__, __LINE__);\
break; \
case GL_INVALID_VALUE: \
printf("%s line %i GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.\n", __FILE__, __LINE__);\
break; \
case GL_INVALID_OPERATION: \
printf("%s line %i GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.\n", __FILE__, __LINE__);\
break; \
case GL_INVALID_FRAMEBUFFER_OPERATION: \
printf("%s line %i GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.\n", __FILE__, __LINE__);\
break; \
case GL_OUT_OF_MEMORY: \
printf("%s line %i GL_OUT_OF_MEMORY: There is not enough memory left to executeLua the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.\n", __FILE__, __LINE__);\
break; \
} \
exit(EXIT_FAILURE); \
} \
} while(false)
#else
#define ASSERT_NO_GL_ERROR(_func) _func
#endif

namespace paper
{
    namespace opengl
    {
        using namespace stick;
        using namespace brick;
        using namespace crunch;

        //The shader programs used by the renderer
        static String vertexShaderCode =
            "#version 150 \n"
            "uniform mat4 transformProjection; \n"
            "uniform vec4 meshColor; \n"
            "in vec2 vertex; \n"
            "out vec4 icol;\n"
            "void main() \n"
            "{ \n"
            "gl_Position = transformProjection * vec4(vertex, 0.0, 1.0); \n"
            "icol = meshColor;\n"
            "} \n";

        static String fragmentShaderCode =
            "#version 150 \n"
            "in vec4 icol; \n"
            "out vec4 pixelColor; \n"
            "void main() \n"
            "{ \n"
            "pixelColor = icol; \n"
            "} \n";

        namespace detail
        {
            enum StencilPlane
            {
                //The mask values are important! FillRaster needs to have at least
                //2 bits. These need to be the lower bits in order to work with the
                //Increment, Decrement stencil operations
                //http://www.opengl.org/discussion_boards/showthread.php/149740-glStencilOp-s-GL_INCR-GL_DECR-behaviour-when-masked
                FillRasterStencilPlane = 0x1F, //binary mask    00011111
                ClipStencilPlaneOne = 1 << 5, //binary mask     00100000
                ClipStencilPlaneTwo = 1 << 6, //binary mask     01000000
                StrokeRasterStencilPlane = 1 << 7 //binary mask 10000000
            };
        }

        static Error compileShader(const String & _shaderCode, GLenum _shaderType, GLuint & _outHandle)
        {
            Error ret;
            GLenum glHandle = glCreateShader(_shaderType);
            const char * cstr = _shaderCode.cString();
            GLint len = (GLint)_shaderCode.length();
            ASSERT_NO_GL_ERROR(glShaderSource(glHandle, 1, &cstr, &len));
            ASSERT_NO_GL_ERROR(glCompileShader(glHandle));

            //check if the shader compiled
            GLint state;
            ASSERT_NO_GL_ERROR(glGetShaderiv(glHandle, GL_COMPILE_STATUS, &state));
            if (state == GL_FALSE)
            {
                GLint infologLength;
                ASSERT_NO_GL_ERROR(glGetShaderiv(glHandle, GL_INFO_LOG_LENGTH, &infologLength));

                char * str = (char *)malloc(infologLength);
                ASSERT_NO_GL_ERROR(glGetShaderInfoLog(glHandle, infologLength, &infologLength, str));

                ret = Error(ec::InvalidArgument, String::concat("Could not compile GLSL shader: ", str), STICK_FILE, STICK_LINE);
                glDeleteShader(glHandle);
                free(str);
            }
            else
            {
                _outHandle = glHandle;
            }
            return ret;
        }

        static Error createProgram(const String & _vertexShader, const String & _fragmentShader, GLuint & _outHandle)
        {
            GLuint vertexShader, fragmentShader;
            Error err = compileShader(_vertexShader, GL_VERTEX_SHADER, vertexShader);
            if (!err)
            {
                err = compileShader(_fragmentShader, GL_FRAGMENT_SHADER, fragmentShader);
            }
            if (err) return err;

            GLuint program = glCreateProgram();

            ASSERT_NO_GL_ERROR(glAttachShader(program, vertexShader));
            ASSERT_NO_GL_ERROR(glAttachShader(program, fragmentShader));

            //bind the attribute locations from the layout
            /*const BufferLayout::BufferElementArray & elements = _settings.vertexLayout.elements();
            auto sit = elements.begin();

            for (; sit != elements.end(); ++sit)
            {
                ASSERT_NO_GL_ERROR(glBindAttribLocation(program, (*sit).m_location, (*sit).m_name.cString()));
            }*/

            ASSERT_NO_GL_ERROR(glBindAttribLocation(program, 0, "vertex"));

            ASSERT_NO_GL_ERROR(glLinkProgram(program));

            //check if we had success
            GLint state;
            ASSERT_NO_GL_ERROR(glGetProgramiv(program, GL_LINK_STATUS, &state));

            if (state == GL_FALSE)
            {
                GLint infologLength;
                ASSERT_NO_GL_ERROR(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength));

                char * str = (char *)malloc(infologLength);
                ASSERT_NO_GL_ERROR(glGetProgramInfoLog(program, infologLength, &infologLength, str));

                err = Error(ec::InvalidArgument, String::concat("Error linking GLSL program: ", str), STICK_FILE, STICK_LINE);
                free(str);
            }

            ASSERT_NO_GL_ERROR(glDeleteShader(vertexShader));
            ASSERT_NO_GL_ERROR(glDeleteShader(fragmentShader));

            if (err)
            {
                glDeleteProgram(program);
            }
            else
            {
                _outHandle = program;
            }

            return err;
        }

        GLRenderer::GLRenderer() :
            m_program(0),
            m_vao(0),
            m_vbo(0),
            m_currentClipStencilPlane(detail::ClipStencilPlaneOne),
            m_bIsInitialized(false),
            m_bCanSwapStencilPlanesWhenEnding(true),
            m_bIsClipping(false)
        {
            m_projection = Mat4f::identity();
            m_transform = Mat4f::identity();
        }

        GLRenderer::GLRenderer(const Document & _doc) :
            RenderInterface(_doc),
            m_program(0),
            m_vao(0),
            m_vbo(0),
            m_currentClipStencilPlane(detail::ClipStencilPlaneOne),
            m_clippingMaskStack(_doc.allocator()),
            m_bIsInitialized(false),
            m_bCanSwapStencilPlanesWhenEnding(true),
            m_bIsClipping(false)
        {

        }

        GLRenderer::~GLRenderer()
        {
            if (m_bIsInitialized)
            {
                glDeleteProgram(m_program);
                glDeleteBuffers(1, &m_vbo);
                glDeleteVertexArrays(1, &m_vao);
            }
        }

        GLRenderer::StencilPlanes GLRenderer::prepareStencilPlanes(bool _bIsClippingPath)
        {
            UInt32 targetStencilBufferMask = _bIsClippingPath ? m_currentClipStencilPlane : detail::FillRasterStencilPlane;
            UInt32 clippingStencilPlaneToTestAgainst = m_currentClipStencilPlane == detail::ClipStencilPlaneOne ? detail::ClipStencilPlaneTwo : detail::ClipStencilPlaneOne;
            return {targetStencilBufferMask, clippingStencilPlaneToTestAgainst};
        }

        void GLRenderer::setViewport(Float _widthInPixels, Float _heightInPixels)
        {
            m_viewport = Vec2f(_widthInPixels, _heightInPixels);
        }

        void GLRenderer::setTransform(const Mat3f & _transform)
        {
            m_transform = crunch::to3DTransform(_transform);;
        }

        GLRenderer::RenderCacheData & GLRenderer::updateRenderCache(const Path & _path, const PathStyle & _style, bool _bIsClipping)
        {
            Path p(_path);
            if (!p.hasComponent<RenderCache>())
            {
                p.set<RenderCache>(RenderCacheData());
            }

            RenderCacheData & cache = p.get<RenderCache>();
            if (_bIsClipping || _style.bHasFill || _style.strokeWidth > 0)
            {
                cache.transformProjection = m_projection * m_transform * crunch::to3DTransform(p.absoluteTransform());
                if (p.hasComponent<comps::FillGeometryDirtyFlag>())
                {
                    if (p.get<comps::FillGeometryDirtyFlag>())
                    {
                        cache.fillVertices.clear();
                        cache.fillVertices.reserve(256);
                        cache.joins.clear();
                        cache.joins.reserve(256);
                        paper::detail::PathFlattener::flatten(p, cache.fillVertices, &cache.joins, 0.15, 0.0, 32);
                        STICK_ASSERT(cache.joins.count() == cache.fillVertices.count());
                        p.set<comps::FillGeometryDirtyFlag>(false);
                    }
                }

                if (p.hasComponent<comps::BoundsGeometryDirtyFlag>())
                {
                    if (p.get<comps::BoundsGeometryDirtyFlag>())
                    {
                        Rect bounds = p.localBounds();
                        Vec2f min = bounds.min();
                        Vec2f max = bounds.max();
                        cache.boundsVertices.resize(4);
                        cache.boundsVertices[0] = min;
                        cache.boundsVertices[1] = min + Vec2f(0, bounds.height());
                        cache.boundsVertices[2] = min + Vec2f(bounds.width(), 0);
                        cache.boundsVertices[3] = max;

                        p.set<comps::BoundsGeometryDirtyFlag>(false);
                    }
                }
            }

            if (_bIsClipping)
                return cache;

            if (_style.strokeWidth > 0)
            {
                if (p.get<comps::StrokeGeometryDirtyFlag>())
                {
                    StrokeCap cap = _style.cap;
                    StrokeJoin join = _style.join;
                    bool bIsScalingStroke = _path.isScalingStroke();
                    Mat3f strokeMat = Item::strokeTransform(&_path.absoluteTransform(), _style.strokeWidth, bIsScalingStroke);
                    Float miterLimit = _style.miterLimit;
                    cache.strokeVertices.clear();

                    Vec2f trans, scale;
                    Float rot;
                    Vec2f * invScale;
                    if (!bIsScalingStroke)
                    {
                        Mat3f absTrans = _path.absoluteTransform();
                        crunch::decompose(absTrans, trans, rot, scale);
                        scale = Vec2f(1.0) / scale;
                        invScale = &scale;
                    }

                    paper::detail::StrokeTriangulator tri(strokeMat, crunch::inverse(strokeMat), join, cap,
                                                          miterLimit, _path.isClosed(), _style.dashArray, _style.dashOffset);
                    tri.triangulateStroke(cache.fillVertices, cache.joins, cache.strokeVertices, _path.isClockwise());

                    cache.strokeVertexDrawMode = (_style.dashArray.count() ? GL_TRIANGLES : GL_TRIANGLE_STRIP);

                    Rect approxStrokeBounds = p.localBounds();
                    Float strokeAdder = std::ceil(_style.strokeWidth * 0.5 * miterLimit) + 1.0;
                    approxStrokeBounds.min() -= strokeAdder;
                    approxStrokeBounds.max() += strokeAdder;

                    cache.strokeBoundsVertices.resize(4);
                    cache.strokeBoundsVertices[0] = approxStrokeBounds.min();
                    cache.strokeBoundsVertices[1] = approxStrokeBounds.min() + Vec2f(0, approxStrokeBounds.height());
                    cache.strokeBoundsVertices[2] = approxStrokeBounds.min() + Vec2f(approxStrokeBounds.width(), 0);
                    cache.strokeBoundsVertices[3] = approxStrokeBounds.max();

                    p.set<comps::StrokeGeometryDirtyFlag>(false);
                }
            }

            return cache;
        }

        Error GLRenderer::drawPath(const Path & _path)
        {
            return drawPathImpl(_path, false);
        }

        Error GLRenderer::beginClipping(const Path & _clippingPath)
        {
            m_bIsClipping = true;
            return generateClippingMask(_clippingPath, false);
        }

        Error GLRenderer::drawPathImpl(const Path & _path, bool _bIsClippingPath)
        {
            Error ret;
            PathStyle style;

            if (!_bIsClippingPath)
                style = PathStyle(_path);

            const auto & children = _path.children();
            PathArray paths(1 + children.count());
            paths[0] = _path;
            for (Size i = 0; i < children.count(); ++i)
            {
                Path p(children[i]);
                updateRenderCache(p, style, _bIsClippingPath);
                paths[i + 1] = p;
            }

            auto & cache = updateRenderCache(_path, style, _bIsClippingPath);
            auto planes = prepareStencilPlanes(_bIsClippingPath);
            if (style.bHasFill || _bIsClippingPath)
            {
                if (static_cast<WindingRule>(_path.windingRule()) == WindingRule::EvenOdd)
                {
                    ret = drawFillEvenOdd(paths, cache.boundsVertices, children.count() ? &cache.transformProjection : nullptr,
                                          planes.targetStencilMask, planes.clippingPlaneToTestAgainst, style, _bIsClippingPath);
                }
                else
                {
                    ret = drawFillNonZero(paths, cache.boundsVertices, children.count() ? &cache.transformProjection : nullptr,
                                          planes.targetStencilMask, planes.clippingPlaneToTestAgainst, style, _bIsClippingPath);
                }
                if (ret) return ret;
            }

            if (_bIsClippingPath) return ret;

            if (style.strokeWidth > 0.0)
            {
                if (paths.count() > 1)
                {
                    for (const auto & p : paths)
                    {
                        ret = drawStroke(p.get<RenderCache>(), style, planes.clippingPlaneToTestAgainst);
                        if (ret) break;
                    }
                }
                else
                {
                    ret = drawStroke(cache, style, planes.clippingPlaneToTestAgainst);
                }
            }

            return ret;
        }

        Error GLRenderer::generateClippingMask(const Path & _clippingPath, bool _bIsRebuilding)
        {
            if (!_bIsRebuilding)
                m_clippingMaskStack.append(_clippingPath);

            ASSERT_NO_GL_ERROR(glStencilMask(m_currentClipStencilPlane));
            ASSERT_NO_GL_ERROR(glClearStencil(0));
            ASSERT_NO_GL_ERROR(glClear(GL_STENCIL_BUFFER_BIT));

            Error ret = drawPathImpl(_clippingPath, true);
            if (ret) return ret;
            m_currentClipStencilPlane = m_currentClipStencilPlane == detail::ClipStencilPlaneOne ? detail::ClipStencilPlaneTwo : detail::ClipStencilPlaneOne;
            return ret;
        }

        Error GLRenderer::endClipping(const Path & _clippingPath)
        {
            STICK_ASSERT(m_clippingMaskStack.count());
            STICK_ASSERT(_clippingPath == m_clippingMaskStack.last());
            m_clippingMaskStack.removeLast();

            if (m_clippingMaskStack.count())
            {
                //check if the last clip mask is still in one of the clipping planes
                if (m_bCanSwapStencilPlanesWhenEnding)
                {
                    m_currentClipStencilPlane = m_currentClipStencilPlane == detail::ClipStencilPlaneOne ? detail::ClipStencilPlaneTwo : detail::ClipStencilPlaneOne;
                    m_bCanSwapStencilPlanesWhenEnding = false;
                    return Error();
                }

                //otherwise rebuild it
                ASSERT_NO_GL_ERROR(glStencilMask(detail::ClipStencilPlaneOne | detail::ClipStencilPlaneTwo));
                ASSERT_NO_GL_ERROR(glClearStencil(255));
                ASSERT_NO_GL_ERROR(glClear(GL_STENCIL_BUFFER_BIT));

                for (Size i = 0; i <  m_clippingMaskStack.count(); ++i)
                {
                    Error err = generateClippingMask(m_clippingMaskStack[i], true);
                    if (err) return err;
                }

                m_bCanSwapStencilPlanesWhenEnding = true;
            }
            else
            {
                ASSERT_NO_GL_ERROR(glStencilMask(detail::ClipStencilPlaneOne | detail::ClipStencilPlaneTwo));
                ASSERT_NO_GL_ERROR(glClearStencil(255));
                ASSERT_NO_GL_ERROR(glClear(GL_STENCIL_BUFFER_BIT));
                m_bIsClipping = false;
            }

            return Error();
        }

        Error GLRenderer::drawFillEvenOdd(const PathArray & _paths,
                                          const PathGeometryArray & _boundsVertices,
                                          Mat4f * _boundsTransform,
                                          UInt32 _targetStencilBufferMask,
                                          UInt32 _clippingPlaneToTestAgainst,
                                          const PathStyle & _style,
                                          bool _bIsClippingPath)
        {
            ASSERT_NO_GL_ERROR(glColorMask(false, false, false, false));
            ASSERT_NO_GL_ERROR(glStencilFunc(m_bIsClipping ? GL_NOTEQUAL : GL_ALWAYS, 0, _clippingPlaneToTestAgainst));
            ASSERT_NO_GL_ERROR(glStencilMask(_targetStencilBufferMask));
            ASSERT_NO_GL_ERROR(glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT));

            for (Path p : _paths)
            {
                auto & cache = p.get<RenderCache>();
                ASSERT_NO_GL_ERROR(glUniformMatrix4fv(glGetUniformLocation(m_program, "transformProjection"), 1, false, cache.transformProjection.ptr()));
                ASSERT_NO_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f) * cache.fillVertices.count(), &cache.fillVertices[0].x, GL_DYNAMIC_DRAW));
                ASSERT_NO_GL_ERROR(glDrawArrays(GL_TRIANGLE_FAN, 0, cache.fillVertices.count()));
            }

            //if we are clipping, there is nothing else to do here
            if (_bIsClippingPath) return Error();

            ASSERT_NO_GL_ERROR(glStencilFunc(GL_EQUAL, 255, detail::FillRasterStencilPlane));
            ASSERT_NO_GL_ERROR(glStencilMask(detail::FillRasterStencilPlane));
            ASSERT_NO_GL_ERROR(glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO));
            ASSERT_NO_GL_ERROR(glColorMask(true, true, true, true));
            if (_boundsTransform)
                ASSERT_NO_GL_ERROR(glUniformMatrix4fv(glGetUniformLocation(m_program, "transformProjection"), 1, false, _boundsTransform->ptr()));
            ASSERT_NO_GL_ERROR(glUniform4fv(glGetUniformLocation(m_program, "meshColor"), 1, _style.fillColor.ptr()));
            ASSERT_NO_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f) * _boundsVertices.count(), &_boundsVertices[0].x, GL_DYNAMIC_DRAW));
            ASSERT_NO_GL_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

            return Error();
        }

        Error GLRenderer::drawFillNonZero(const PathArray & _paths,
                                          const PathGeometryArray & _boundsVertices,
                                          Mat4f * _boundsTransform,
                                          UInt32 _targetStencilBufferMask,
                                          UInt32 _clippingPlaneToTestAgainst,
                                          const PathStyle & _style,
                                          bool _bIsClippingPath)
        {
            ASSERT_NO_GL_ERROR(glColorMask(false, false, false, false));
            ASSERT_NO_GL_ERROR(glStencilFunc(m_bIsClipping ? GL_NOTEQUAL : GL_ALWAYS, 0, _clippingPlaneToTestAgainst));
            ASSERT_NO_GL_ERROR(glEnable(GL_CULL_FACE));

            for (Path p : _paths)
            {
                auto & cache = p.get<RenderCache>();

                //NonZero winding rule needs to use Increment and Decrement stencil operations.
                //we therefore render to the rasterize mask, even if this is a clipping mask, and transfer
                //the results to the clipping mask stencil plane afterwards
                ASSERT_NO_GL_ERROR(glStencilMask(detail::FillRasterStencilPlane));
                ASSERT_NO_GL_ERROR(glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP));
                ASSERT_NO_GL_ERROR(glCullFace(GL_BACK));
                ASSERT_NO_GL_ERROR(glFrontFace(GL_CCW));

                ASSERT_NO_GL_ERROR(glUniformMatrix4fv(glGetUniformLocation(m_program, "transformProjection"), 1, false, cache.transformProjection.ptr()));
                ASSERT_NO_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f) * cache.fillVertices.count(), &cache.fillVertices[0].x, GL_DYNAMIC_DRAW));
                ASSERT_NO_GL_ERROR(glDrawArrays(GL_TRIANGLE_FAN, 0, cache.fillVertices.count()));

                ASSERT_NO_GL_ERROR(glFrontFace(GL_CW));
                ASSERT_NO_GL_ERROR(glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP));

                ASSERT_NO_GL_ERROR(glDrawArrays(GL_TRIANGLE_FAN, 0, cache.fillVertices.count()));
            }

            ASSERT_NO_GL_ERROR(glDisable(GL_CULL_FACE));
            ASSERT_NO_GL_ERROR(glFrontFace(GL_CW));

            //TODO Add extra step to move the results to the clipping plane if this is a clipping mask
            if (_bIsClippingPath)
            {
                ASSERT_NO_GL_ERROR(glStencilMask(_targetStencilBufferMask));
                ASSERT_NO_GL_ERROR(glStencilFunc(GL_NOTEQUAL, 0, detail::FillRasterStencilPlane));
                ASSERT_NO_GL_ERROR(glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT));

                if (_boundsTransform)
                    ASSERT_NO_GL_ERROR(glUniformMatrix4fv(glGetUniformLocation(m_program, "transformProjection"), 1, false, _boundsTransform->ptr()));

                ASSERT_NO_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f) * _boundsVertices.count(), &_boundsVertices[0].x, GL_DYNAMIC_DRAW));
                ASSERT_NO_GL_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

                //draw the bounds one last time to zero out the tmp data created in the FillRasterStencilPlane
                ASSERT_NO_GL_ERROR(glStencilMask(detail::FillRasterStencilPlane));
                ASSERT_NO_GL_ERROR(glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO));
                ASSERT_NO_GL_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
                return Error();
            }

            //this is sort of meta O_o: 32 in binary is 00100000, which represents the two bits of OcclusionStencilPlane...not exactly sure why this works, needs more checking.
            ASSERT_NO_GL_ERROR(glStencilFunc(GL_NOTEQUAL, 0, detail::FillRasterStencilPlane));
            ASSERT_NO_GL_ERROR(glStencilMask(detail::FillRasterStencilPlane));
            ASSERT_NO_GL_ERROR(glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO));
            ASSERT_NO_GL_ERROR(glColorMask(true, true, true, true));
            ASSERT_NO_GL_ERROR(glUniform4fv(glGetUniformLocation(m_program, "meshColor"), 1, _style.fillColor.ptr()));
            if (_boundsTransform)
                ASSERT_NO_GL_ERROR(glUniformMatrix4fv(glGetUniformLocation(m_program, "transformProjection"), 1, false, _boundsTransform->ptr()));
            ASSERT_NO_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f) * _boundsVertices.count(), &_boundsVertices[0].x, GL_DYNAMIC_DRAW));
            ASSERT_NO_GL_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

            return Error();
        }

        Error GLRenderer::drawStroke(const RenderCacheData & _geom, const PathStyle & _style, stick::UInt32 _clippingPlaneToTestAgainst)
        {
            Error ret;

            ASSERT_NO_GL_ERROR(glColorMask(false, false, false, false));
            ASSERT_NO_GL_ERROR(glStencilMask(detail::StrokeRasterStencilPlane));
            ASSERT_NO_GL_ERROR(glStencilFunc(m_bIsClipping ? GL_NOTEQUAL : GL_ALWAYS, 0, _clippingPlaneToTestAgainst));
            ASSERT_NO_GL_ERROR(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
            ASSERT_NO_GL_ERROR(glUniformMatrix4fv(glGetUniformLocation(m_program, "transformProjection"), 1, false, _geom.transformProjection.ptr()));
            ASSERT_NO_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f) * _geom.strokeVertices.count(), &_geom.strokeVertices[0].x, GL_DYNAMIC_DRAW));
            ASSERT_NO_GL_ERROR(glDrawArrays(_geom.strokeVertexDrawMode, 0, _geom.strokeVertices.count()));

            ASSERT_NO_GL_ERROR(glColorMask(true, true, true, true));
            ASSERT_NO_GL_ERROR(glStencilFunc(GL_EQUAL, 0, detail::StrokeRasterStencilPlane));
            ASSERT_NO_GL_ERROR(glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT));
            ASSERT_NO_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f) * _geom.strokeBoundsVertices.count(), &_geom.strokeBoundsVertices[0].x, GL_DYNAMIC_DRAW));
            ASSERT_NO_GL_ERROR(glUniform4fv(glGetUniformLocation(m_program, "meshColor"), 1, _style.strokeColor.ptr()));
            ASSERT_NO_GL_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

            return ret;
        }

        Error GLRenderer::prepareDrawing()
        {
            if (!m_bIsInitialized)
            {
                Error err = createProgram(vertexShaderCode, fragmentShaderCode, m_program);
                if (err) return err;
                ASSERT_NO_GL_ERROR(glGenVertexArrays(1, &m_vao));
                ASSERT_NO_GL_ERROR(glGenBuffers(1, &m_vbo));
                ASSERT_NO_GL_ERROR(glBindVertexArray(m_vao));
                ASSERT_NO_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
                ASSERT_NO_GL_ERROR(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), ((char *)0)));
                ASSERT_NO_GL_ERROR(glEnableVertexAttribArray(0));
                m_bIsInitialized = true;
            }

            ASSERT_NO_GL_ERROR(glDisable(GL_DEPTH_TEST));
            ASSERT_NO_GL_ERROR(glDepthMask(false));
            ASSERT_NO_GL_ERROR(glEnable(GL_MULTISAMPLE));
            ASSERT_NO_GL_ERROR(glEnable(GL_BLEND));
            ASSERT_NO_GL_ERROR(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            ASSERT_NO_GL_ERROR(glEnable(GL_STENCIL_TEST));
            ASSERT_NO_GL_ERROR(glStencilMask(detail::FillRasterStencilPlane));
            ASSERT_NO_GL_ERROR(glClearStencil(0));
            ASSERT_NO_GL_ERROR(glClear(GL_STENCIL_BUFFER_BIT));
            ASSERT_NO_GL_ERROR(glStencilMask(detail::ClipStencilPlaneOne | detail::ClipStencilPlaneTwo | detail::StrokeRasterStencilPlane));
            ASSERT_NO_GL_ERROR(glClearStencil(255));
            ASSERT_NO_GL_ERROR(glClear(GL_STENCIL_BUFFER_BIT));

            ASSERT_NO_GL_ERROR(glUseProgram(m_program));
            ASSERT_NO_GL_ERROR(glBindVertexArray(m_vao));
            ASSERT_NO_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
            ASSERT_NO_GL_ERROR(glViewport(0, 0, m_viewport.x, m_viewport.y));
            m_projection = Mat4f::ortho(0, m_document.width(), m_document.height(), 0, -1, 1);
            return Error();
        }

        Error GLRenderer::finishDrawing()
        {
            ASSERT_NO_GL_ERROR(glUseProgram(0));
            ASSERT_NO_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
            ASSERT_NO_GL_ERROR(glBindVertexArray(0));

            ASSERT_NO_GL_ERROR(glDisable(GL_BLEND));
            ASSERT_NO_GL_ERROR(glDisable(GL_CULL_FACE));
            ASSERT_NO_GL_ERROR(glDisable(GL_STENCIL_TEST));

            return Error();
        }
    }
}
