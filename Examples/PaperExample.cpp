
// we use GLFW to open a simple window
#include <GLFW/glfw3.h>
#include <Paper/Document.hpp>
#include <Paper/OpenGL/GLRenderer.hpp>
#include <Crunch/StringConversion.hpp>

using namespace paper;
using namespace brick;
using namespace stick;

int main(int _argc, const char * _args[])
{
    // initialize glfw
    if (!glfwInit())
        return EXIT_FAILURE;

    // and set some hints
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(800, 600, "Hello Triangle Example", NULL, NULL);
    if (window)
    {
        glfwMakeContextCurrent(window);

        Hub hub;
        Document doc = createDocument(hub);
        doc.setSize(800, 600);
        opengl::GLRenderer renderer(doc);

        Path circle = doc.createCircle(Vec2f(400, 300.0), 60.0);
        circle.setFill(ColorRGBA(1.0, 1.0, 0.0, 1.0));
        circle.setStrokeWidth(5.0);
        circle.setStroke(ColorRGBA(1.0, 0.0, 1.0, 1.0));
        circle.setStrokeJoin(StrokeJoin::Round);

        Path c2 = doc.createCircle(Vec2f(460, 300.0), 30.0);
        //c2.translateTransform(Vec2f(60, 0));
        circle.addChild(c2);

        circle.setStrokeScaling(false);
        circle.scale(0.5);

        Group grp = doc.createGroup();
        grp.addChild(circle);
        Path cl = circle.clone();
        cl.translate(100, 100);
        grp.addChild(cl);

        doc.saveSVG("test.svg");
        //circle.reverse();
        //circle.reverse();
        //circle.scaleTransform(2.0, 1.0);
        //doc.scaleTransform(0.75);

        /*Group grp = doc.createGroup("FICKEN");
        grp.addChild(circle);
        grp.rotateTransform(crunch::Constants<Float>::pi() * 0.25);*/


        
        // Path star = doc.createPath("Star");
        // star.addPoint(Vec2f(200, 100));
        // star.addPoint(Vec2f(180, 200));
        // star.addPoint(Vec2f(300, 150));
        // star.addPoint(Vec2f(100, 150));
        // star.addPoint(Vec2f(220, 200));
        // //star.addSegment(Vec2f(220, 200), Vec2f(50, -40), Vec2f(-30, 30));
        // star.closePath();
        // star.setFill(ColorRGBA(0.0f, 0.5f, 0.75f, 1.0f));
        // star.setStroke(ColorRGBA(1, 0, 1, 1));
        // star.setStrokeWidth(5.0);
        // star.setStrokeJoin(StrokeJoin::Round);
        // star.setStrokeCap(StrokeCap::Butt);
        // star.scaleTransform(Vec2f(2.0, 1.5));
        // star.rotateTransform(crunch::Constants<Float>::pi() * 0.25);
        // Float off = 0;
        // //star.setDashArray({12.5, 10});

        // Path c3 = doc.createCircle(Vec2f(270, 160.0), 5.0);
        // Path c4 = doc.createCircle(Vec2f(190, 230.0), 5.0);
        // c3.setFill(ColorRGBA(1,1,1,1));
        // c4.setFill(ColorRGBA(1,1,1,1));
        // star.smooth();
        //star.setWindingRule(WindingRule::NonZero);
        /* for(int i=0; i<12; i++)
         {
             Path star2 = star.clone();
             star2.translateTransform(Vec2f(-10, 10) * (i + 1));
         }*/

        /*Path cp = doc.createCircle(Vec2f(100, 300), 60.0);
        cp.setFill(ColorRGBA(1.0, 1.0, 0.0, 1.0));
        //cp.setWindingRule(WindingRule::NonZero);
        Path hole = cp.clone();
        hole.scaleTransform(0.5);
        //hole.setFill(ColorRGBA(1.0, 0.0, 0.0, 1.0));;
        cp.addChild(hole);
        cp.setStroke(ColorRGBA(1.0, 0.0, 1.0, 1.0));
        cp.setStrokeWidth(20.0);*/
        //cp.reverse();


        /*Path clippedRecta = doc.createRectangle(Vec2f(40.0, 240.0), Vec2f(160.0, 360.0));
        clippedRecta.setFill(ColorRGBA(1.0, 0.5, 1.0, 1.0));
        Group grpa = doc.createGroup("testa");
        Path c = doc.createCircle(Vec2f(100, 300), 50.0);
        c.setWindingRule(WindingRule::EvenOdd);
        grpa.addChild(c);
        grpa.setClipped(true);
        grpa.addChild(clippedRecta);

        Group grp = doc.createGroup("test");
        grp.addChild(cp);
        grp.setClipped(true);

        Path clippedRect = doc.createRectangle(Vec2f(40.0, 240.0), Vec2f(160.0, 360.0));
        clippedRect.setFill(ColorRGBA(0.0, 0.0, 1.0, 1.0));
        grp.addChild(clippedRect);

        clippedRect.setStroke(ColorRGBA(1.0, 0.0, 1.0, 1.0));
        clippedRect.setStrokeWidth(10.0);

        grp.addChild(grpa);*/

        //printf("%s\n", doc.exportSVG().ensure().cString());

        // the main loop
        while (!glfwWindowShouldClose(window))
        {
            //glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // if(star.contains(Vec2f(mouseX, mouseY)))
            //     printf("HIT\n");
            // else
            //     printf("NOT HIT\n");

            // get the window size from the glfw window
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            renderer.setViewport(width, height);

            //off += 0.1;
            //star.setDashOffset(off);
            //star.rotateTransform(0.01);

            auto bb = grp.bounds();
            Path bbp = doc.createRectangle(bb.min(), bb.max());
            bbp.setFill(ColorRGBA(1.0, 1.0, 1.0, 0.25));
            Error err = renderer.draw();
            bbp.remove();

            if (err)
            {
                printf("Error\n");
                return EXIT_FAILURE;
            }

            //swap the glfw windows buffer & poll the window events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    else
    {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    //clean up glfw
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}