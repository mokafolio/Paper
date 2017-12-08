// This example shows the very basic steps to create a Paper Document
// and draw it.

// we use GLFW to open a simple window
#include <GLFW/glfw3.h>

//include some paper headers.
#include <Paper/Document.hpp>
#include <Paper/CurveLocation.hpp>

//we want to use the OpenGL renderer, so we include it.
#include <Paper/OpenGL/GLRenderer.hpp>

#include <Crunch/Randomizer.hpp>
#include <Crunch/MatrixFunc.hpp>

#include <Stick/Thread.hpp>
#include <Stick/FileUtilities.hpp>


//we want to use the paper, brick, crunch & stick namespaces
using namespace paper; // paper namespace
using namespace brick; // brick namespace for entity / component things
using namespace crunch; // crunch namespace for math
using namespace stick; // stick namespace for core data structures/containers etc.

int main(int _argc, const char * _args[])
{
    // initialize glfw
    if (!glfwInit())
        return EXIT_FAILURE;

    // and set some hints to get the correct opengl versions/profiles
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //create the window
    GLFWwindow * window = glfwCreateWindow(800, 600, "Hello Paper Example", NULL, NULL);
    if (window)
    {
        glfwMakeContextCurrent(window);

        // the brick entity hub to use by the paper document.
        Hub hub;
        // create the document.
        Document doc = createDocument(hub);
        doc.setSize(800, 600);
        // create the opengl renderer for the document
        opengl::GLRenderer renderer(doc);


        // Randomizer r;

        // LinearGradient grad = doc.createLinearGradient(Vec2f(r.randomf(100, 700), r.randomf(100, 500)),
        //                       Vec2f(r.randomf(100, 700), r.randomf(100, 500)));

        // for (int i = 0; i < r.randomi(2, 10); ++i)
        // {
        //     grad.addStop(ColorRGBA(r.randomf(0, 1), r.randomf(0, 1), r.randomf(0, 1), 1.0), r.randomf(0, 1));
        // }

        // // Path circle = doc.createCircle(Vec2f(400, 300), 50);
        // // circle.setFill(grad);
        // // circle.setStroke(ColorRGBA(0.0, 0.0, 1.0, 1.0));
        // // circle.setStrokeWidth(10.0);
        // // circle.setFill("red");

        // Path circle = doc.createPath();
        // circle.setFill(grad);

        // for (int i = 0; i < 10; i++)
        // {
        //     circle.addPoint(Vec2f(r.randomf(100, 700), r.randomf(100, 500)));
        // }

        // circle.smooth();


        // Path c2 = doc.createCircle(Vec2f(100, 100), 100);
        // c2.setFill(grad);

        // // circle.rotate(Constants<Float32>::pi() * 0.25);


        // Vec2f dir = grad.destination() - grad.origin();
        // Vec2f ndir = normalize(dir);
        // Vec2f perp(-dir.y, dir.x);
        // Vec2f nperp(-ndir.y, ndir.x);
        // Float len = length(dir);
        // Line2f line(grad.origin(), dir);
        // LineSegment2f segment(grad.origin(), grad.destination());

        // auto angle = toDegrees(std::atan2(dir.y, dir.x));
        // auto side = line.side(circle.position());
        // printf("ANGLE %f SIDE %i\n", angle, side);
        // Rect bounds = circle.bounds();

        // auto center = grad.origin();
        // // auto d = dot(bounds.topLeft() - center, ndir) / len;
        // // auto d2 = dot(bounds.topLeft() - center, nperp);
        // // printf("DOT %f %f\n", d, d2);

        // // auto d3 = dot(bounds.bottomLeft() - center, ndir) / len;
        // // auto d4 = dot(bounds.bottomLeft() - center, nperp);

        // // printf("DOT2 %f %f\n", d3, d4);
        // Vec2f corners[4] =
        // {
        //     bounds.topLeft() - center, bounds.topRight() - center,
        //     bounds.bottomLeft() - center, bounds.bottomRight() - center
        // };
        // Float o, s;
        // Float left, right;
        // Float minOffset, maxOffset;
        // for (int i = 0; i < 4; ++i)
        // {
        //     o = dot(corners[i], ndir) / len;
        //     s = dot(corners[i], nperp);

        //     if (o < minOffset || i == 0) minOffset = o;
        //     if (o > maxOffset || i == 0) maxOffset = o;

        //     if (i == 0 || s < left) left = s;
        //     if (i == 0 || s > right) right = s;
        // }

        // Vec2f ac = center + nperp * left; ac += ndir * minOffset * len;
        // Vec2f bc = center + nperp * right; bc += ndir * minOffset * len;
        // Vec2f cc = center + nperp * left; cc += ndir * maxOffset * len;
        // Vec2f dc = center + nperp * right; dc += ndir * maxOffset * len;

        // Path cp = doc.createPath();
        // cp.setFill(ColorRGBA(1.0, 0.5, 0.1, 0.25));
        // cp.addPoint(ac);
        // cp.addPoint(bc);
        // cp.addPoint(dc);
        // cp.addPoint(cc);


        // LineSegment2f shifted = segment;
        // if (bounds.contains(grad.origin()) || bounds.contains(grad.destination()) ||
        //         intersect(bounds.leftSegment(), segment) ||
        //         intersect(bounds.topSegment(), segment) ||
        //         intersect(bounds.rightSegment(), segment) ||
        //         intersect(bounds.bottomSegment(), segment))
        // {
        //     auto dig = bounds.diagonal();
        //     auto ddir = segment.side(circle.position()) == -1 ? perp : -perp;
        //     shifted = LineSegment2f(shifted.positionOne() + ddir * dig, shifted.positionTwo() + ddir * dig);
        // }

        // Vec2f dir = grad.destination() - grad.origin();
        // Vec2f perp2(-dir.y, dir.x);
        // Vec2f center = grad.origin() + dir * len * 0.5;


        // Path rct = doc.createPath();
        // Float dig = std::max(bounds.diagonal(), len)  * 0.5;
        // Vec2f a = bounds.center() - dir * dig + perp * dig;
        // Vec2f b = bounds.center() + dir * dig + perp * dig;
        // Vec2f c = bounds.center() + dir * dig - perp * dig;
        // Vec2f d = bounds.center() - dir * dig - perp * dig;
        // rct.setFill(ColorRGBA(1.0, 1.0, 0.5, 0.5));
        // rct.addPoint(a);
        // rct.addPoint(b);
        // rct.addPoint(c);
        // rct.addPoint(d);

        // Path origin = doc.createCircle(grad.origin(), 3);
        // origin.setFill("green");

        // Path dest = doc.createCircle(grad.destination(), 3);
        // dest.setFill("blue");

        // for (auto & stop : grad.stops())
        // {
        //     printf("OFF %f\n", stop.offset);
        //     Path c = doc.createCircle(grad.origin() + dir * stop.offset , 3);
        //     c.setFill(stop.color);
        //     c.setStroke("black");
        // }

        // // Path brect = doc.createRectangle(bounds.min(), bounds.max());
        // // brect.setFill(ColorRGBA(0, 0, 0, 0.1));
        // // brect.rotate(toRadians(angle));

        // Path shiftedPath = doc.createPath();
        // shiftedPath.setStroke("gray");
        // shiftedPath.addPoint(shifted.positionOne());
        // shiftedPath.addPoint(shifted.positionTwo());

        // // for (auto & stop : grad.stops())
        // // {
        // //     Path c = doc.createCircle(shifted.positionOne() + dir * len * stop.offset , 3);
        // //     c.setFill(stop.color);
        // // }

        // printf("INVERSE %s\n", crunch::toString(inverse(Mat3f::identity())).cString());

        // Randomizer r;

        // Path p = doc.createCircle(Vec2f(100, 100), 30);
        // p.setFill("pink");

        // Path p2 = doc.createPath();
        // p2.setFill(ColorRGBA(1, 1, 0, 0.5));
        // Float off = 0;
        // Float len = p.length();
        // while(off <= len)
        // {
        //     CurveLocation loc = p.curveLocationAt(std::min(off, len));
        //     // Path n = doc.createPath();
        //     // n.addPoint(loc.position());
        //     // n.addPoint(loc.position() + loc.normal() * 20);
        //     // n.setStroke("green");

        //     // Path t = doc.createPath();
        //     // t.addPoint(loc.position());
        //     // t.addPoint(loc.position() + loc.tangent() * 20);
        //     // t.setStroke("red");

        //     p2.addPoint(loc.position() + loc.normal() * std::sin(off * 0.2) * 4);
        //     off += 1.0;
        // }
        // p2.closePath();

        // // Path p2 = p.clone();
        // // p2.setFill(ColorRGBA(1, 1, 0, 0.5));
        // // p2.flattenRegular(1);

        // printf("BEF %lu\n", p2.segmentCount());
        // p2.simplify(0.5);
        // printf("AFTER %lu\n", p2.segmentCount());

        // for(int i=0; i < p2.segmentCount(); ++i)
        // {
        //     Path c = doc.createCircle(p2.segment(i).position(), 1);
        //     c.setFill("blue");
        // }

        // Path p2 = doc.createPathFromSVGData("M299.235,142.199C298.796,141.766 298.738,141.533 298.705,141.231C298.673,140.93 298.666,140.561 298.74,140.257C298.814,139.953 298.969,139.715 299.137,139.482C299.305,139.249 299.487,139.021 299.685,138.801C299.883,138.581 300.098,138.369 300.324,138.162C300.55,137.955 300.787,137.754 301.035,137.554C301.283,137.355 301.542,137.158 301.814,136.962C302.085,136.767 302.368,136.573 302.664,136.383C302.959,136.193 303.267,136.008 303.587,135.829C303.907,135.651 304.239,135.48 304.581,135.318C304.923,135.157 305.274,135.005 305.634,134.863C305.993,134.721 306.359,134.59 306.73,134.468C307.101,134.347 307.477,134.237 307.856,134.137C308.235,134.038 308.616,133.949 308.998,133.872C309.38,133.795 309.763,133.729 310.145,133.674C310.527,133.618 310.909,133.574 311.289,133.539C311.669,133.505 312.048,133.481 312.424,133.465C312.8,133.45 313.173,133.444 313.544,133.446C313.914,133.448 314.282,133.458 314.646,133.475C315.009,133.492 315.37,133.515 315.726,133.543C316.083,133.571 316.436,133.604 316.785,133.641C317.135,133.677 317.481,133.717 317.824,133.758C318.167,133.799 318.507,133.841 318.846,133.883C319.184,133.926 319.52,133.968 319.855,134.01C320.19,134.052 320.523,134.094 320.856,134.138C321.188,134.183 321.52,134.23 321.851,134.284C322.182,134.338 322.513,134.4 322.841,134.475C323.17,134.549 323.498,134.637 323.82,134.746C324.143,134.855 324.461,134.984 324.768,135.141C325.076,135.298 325.373,135.482 325.652,135.687C325.931,135.893 326.193,136.121 326.416,136.398C326.64,136.674 326.826,137 326.836,137.296C326.845,137.592 326.678,137.859 326.48,138.083C326.281,138.307 326.052,138.487 325.811,138.666C325.57,138.845 325.318,139.022 325.063,139.202C324.809,139.382 324.552,139.565 324.29,139.749C324.028,139.932 323.762,140.118 323.491,140.301C323.22,140.485 322.944,140.667 322.665,140.845C322.385,141.024 322.101,141.2 321.815,141.371C321.528,141.542 321.24,141.709 320.949,141.872C320.659,142.035 320.367,142.193 320.074,142.348C319.782,142.502 319.488,142.652 319.194,142.798C318.9,142.944 318.605,143.086 318.309,143.226C318.014,143.365 317.718,143.503 317.422,143.636C317.125,143.77 316.827,143.9 316.526,144.024C316.226,144.149 315.922,144.267 315.616,144.378C315.309,144.489 314.998,144.591 314.684,144.687C314.369,144.782 314.051,144.87 313.729,144.951C313.408,145.032 313.082,145.108 312.755,145.177C312.427,145.247 312.096,145.311 311.765,145.377C311.435,145.442 311.104,145.509 310.776,145.585C310.448,145.661 310.124,145.745 309.804,145.842C309.484,145.938 309.17,146.047 308.857,146.154C308.544,146.262 308.232,146.37 307.916,146.473C307.601,146.577 307.282,146.677 306.95,146.687C306.619,146.697 306.276,146.617 305.942,146.489C305.607,146.36 305.28,146.183 304.951,146.001C304.623,145.82 304.291,145.633 303.966,145.447C303.64,145.262 303.319,145.077 302.976,144.869C302.632,144.661 302.265,144.431 301.984,144.266C301.703,144.101 301.509,144.001 301.002,143.634C300.495,143.267 299.675,142.633 299.235,142.199");
        Path p2 = doc.createPathFromSVGData("M330.49,193.83C329.789,194.066 329.647,194.068 329.395,194.019C329.143,193.97 328.781,193.871 328.542,193.66C328.303,193.449 328.187,193.128 328.112,192.79C328.037,192.453 328.004,192.1 328,191.734C327.996,191.367 328.022,190.986 328.085,190.595C328.147,190.203 328.246,189.8 328.386,189.392C328.526,188.984 328.706,188.57 328.932,188.164C329.158,187.759 329.43,187.361 329.745,186.99C330.06,186.62 330.418,186.276 330.805,185.976C331.192,185.676 331.607,185.421 332.026,185.213C332.445,185.005 332.867,184.845 333.271,184.721C333.674,184.597 334.06,184.508 334.417,184.436C334.774,184.363 335.103,184.306 335.407,184.25C335.712,184.195 335.992,184.142 336.258,184.084C336.524,184.027 336.776,183.965 337.022,183.896C337.269,183.828 337.509,183.752 337.756,183.667C338.003,183.582 338.255,183.488 338.524,183.386C338.793,183.284 339.08,183.174 339.389,183.056C339.698,182.938 340.03,182.812 340.381,182.674C340.732,182.537 341.102,182.387 341.474,182.219C341.845,182.052 342.219,181.867 342.565,181.674C342.911,181.48 343.231,181.277 343.493,181.1C343.755,180.923 343.959,180.772 344.088,180.732C344.217,180.693 344.27,180.765 344.292,180.941C344.313,181.117 344.303,181.398 344.293,181.707C344.282,182.017 344.273,182.355 344.268,182.69C344.262,183.024 344.261,183.355 344.273,183.681C344.285,184.008 344.31,184.33 344.361,184.653C344.411,184.977 344.487,185.301 344.563,185.63C344.639,185.96 344.716,186.295 344.756,186.635C344.796,186.974 344.799,187.318 344.761,187.661C344.724,188.004 344.644,188.346 344.532,188.694C344.42,189.042 344.275,189.397 344.098,189.746C343.92,190.095 343.709,190.439 343.478,190.77C343.246,191.101 342.992,191.418 342.718,191.685C342.443,191.952 342.147,192.168 341.848,192.336C341.549,192.505 341.248,192.626 340.947,192.709C340.645,192.793 340.343,192.839 340.03,192.852C339.717,192.865 339.393,192.845 339.036,192.805C338.679,192.766 338.289,192.706 337.866,192.647C337.443,192.587 336.987,192.528 336.462,192.516C335.937,192.504 335.343,192.538 334.874,192.569C334.405,192.6 334.06,192.628 333.257,192.877C332.453,193.125 331.192,193.594 330.49,193.83");
        p2.setFill("red");
        // p2.scale(10);
        p2.closePath();
        // p2.simplify(2.5);

        Path p = p2.clone();
        p.removeFill();
        p.setStroke("blue");
        printf("BEF %lu\n", p.segmentCount());
        p.simplify(2.5);
        printf("AFTER %lu\n", p.segmentCount());

        printf("CLOSED %i %i\n", p2.isClosed(), p.isClosed());

        for(int i=0; i < p.segmentCount(); ++i)
        {
            Path hi = doc.createPath();
            hi.setStroke("green");
            hi.addPoint(p.segment(i).position());
            hi.addPoint(p.segment(i).position() + p.segment(i).handleIn());
                       Path ho = doc.createPath();
            ho.setStroke("orange");
            ho.addPoint(p.segment(i).position());
            ho.addPoint(p.segment(i).position() + p.segment(i).handleOut());
            Path c = doc.createCircle(p.segment(i).position(), 2);
            c.setFill("yellow");
        }

        // the main loop
        while (!glfwWindowShouldClose(window))
        {
            // clear the background to black
            glClearColor(200, 2000, 2000, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            // get the window size from the glfw window
            // and set it as the viewport on the renderer
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            renderer.setViewport(width, height);
            renderer.setTransform(Mat3f::identity());

            auto err = renderer.draw();
            if (err)
            {
                printf("ERROR: %s\n", err.message().cString());
                return EXIT_FAILURE;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    else
    {
        glfwTerminate();
        printf("Could not open GLFW window :(\n");
        return EXIT_FAILURE;
    }

    // clean up glfw
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
