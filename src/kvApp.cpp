#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "osc.h"


using namespace ci;
using namespace ci::app;
using namespace std;


class kvApp : public App {
  public:
    void setup() override;
    void resize() override;
    void keyDown( KeyEvent event ) override;
    void update() override;
    void draw() override;
    void shutdown();
    
    OSCMessenger *oscMessenger;
    World *world;
    GraphicsRenderer *ogl;
    Rule *rule;
    Boids *boids;

    int _inport = 7000;
    int _outport = 57121;
    std::string _remoteHost = "127.0.0.1";
};

void kvApp::resize() {
    ogl->reshape();
}

void kvApp::setup()
{
    int _winSizeX = 1024;
    int _winSizeY = 768;
    int _frameRate = 30;
    int _windowMode = 0;
    int _fullScreen = 0;

    auto args = getCommandLineArgs();
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-screenx" && i + 1 < args.size()) {
            _winSizeX = std::stoi(args[i + 1]);
        } else if (args[i] == "-screeny" && i + 1 < args.size()) {
            _winSizeY = std::stoi(args[i + 1]);
        } else if (args[i] == "-fps" && i + 1 < args.size()) {
            _frameRate = std::stoi(args[i + 1]);
        } else if (args[i] == "-wmode" && i + 1 < args.size()) {
            _windowMode = std::stoi(args[i + 1]);
        } else if (args[i] == "-full" && i + 1 < args.size()) {
            _fullScreen = std::stoi(args[i + 1]);
        }
    }

    auto displays = Display::getDisplays();
    
    // Check if a secondary display is available
    DisplayRef targetDisplay;
    if (displays.size() > 1) {
        targetDisplay = displays[1]; // Select the second display
    } else {
        targetDisplay = Display::getMainDisplay(); // Fallback to main display
    }
    
    auto targetSize = targetDisplay->getSize();

    if (_fullScreen > 0) {
        setFullScreen(_fullScreen);
        _winSizeX = targetSize.x;
        _winSizeY = targetSize.y;
    }
    if (_windowMode == 0)
        getWindow()->setPos(targetDisplay->getBounds().x1, targetDisplay->getBounds().y1);
    else
        getWindow()->setPos(targetDisplay->getBounds().x2 - _winSizeX, targetDisplay->getBounds().y2 - _winSizeY);
    setWindowSize(_winSizeX, _winSizeY);
    
    setFrameRate(_frameRate);

    oscMessenger = new OSCMessenger(_remoteHost, _outport, _inport);
    world = new World();
    ogl = new GraphicsRenderer(world);
    oscMessenger->setOgl(ogl);
    oscMessenger->setWorld(world);
    
    ogl->setupOgl();
}

void kvApp::keyDown( KeyEvent event )
{
    if( event.getChar() == 'f' || event.getChar() == 'F' ) {
        setFullScreen( ! isFullScreen() );
        if (isFullScreen()) { hideCursor(); }
        else { showCursor(); }
    }
    if( event.getChar() == 'i' || event.getChar() == 'I' ) {
        
    }

}

void kvApp::update()
{
    ogl->update();
}

void kvApp::draw()
{
    
    ogl->startDraw();
        
    if (world->initialized()) {
        
        int x, y, z;
        world->prepareNext();
        
        for (x = 0; x < world->sizeX(); x++) {
            for (y = 0; y < world->sizeY(); y++) {
                for (z = 0; z < world->sizeZ(); z++) {
                    
                    if (world->ruleInitialized)
                        world->next(x, y, z);
                    if (world->somActivated)
                        world->nextSOM(x, y, z);

                    ogl->drawFragment(&world->cells[x][y][z]);
                }
            }
        }

        world->finalizeNext();
                
    }
            
    ogl->endDraw();
}

void kvApp::shutdown() {
    delete ogl;
    delete world;
    delete oscMessenger;
}

CINDER_APP( kvApp, RendererGl(RendererGl::Options().msaa( 16 )), [&]( App::Settings *settings ) {
    
//    auto displays = Display::getDisplays();
//    
//    // Check if a secondary display is available
//    DisplayRef targetDisplay;
//    if (displays.size() > 1) {
//        targetDisplay = displays[1]; // Select the second display
//    } else {
//        targetDisplay = Display::getMainDisplay(); // Fallback to main display
//    }
//    
//    auto targetSize = targetDisplay->getSize();
    
//    if (_fullScreen > 0) {
//        settings->setFullScreen();
//        _winSizeX = targetSize.x;
//        _winSizeY = targetSize.y;
//    }
//    if (_windowMode == 0)
//        settings->setWindowPos(targetDisplay->getBounds().x1, targetDisplay->getBounds().y1);
//    else
//        settings->setWindowPos(targetDisplay->getBounds().x2 - _winSizeX, targetDisplay->getBounds().y2 - _winSizeY);
//    settings->setWindowSize(_winSizeX, _winSizeY);
    
    settings->setBorderless(true);
//    settings->setFrameRate(_frameRate);
})
