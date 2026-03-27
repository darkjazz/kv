/*
 *  osc.cpp
 *  lambda
 *
 *  Created by alo on 07/11/2011.
 *  
 *	This file is part of lambda.
 *
 *	lambda is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 
 *	lambda is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 
 *	You should have received a copy of the GNU General Public License
 *	along with lambda.  If not, see <http://www.gnu.org/licenses/>. 
 *
 */

#include "osc.h"

void OSCMessenger::setUpSender() {
    try {
        _sender.bind();
    }
    catch ( const osc::Exception &ex ) {
        CI_LOG_E( "Error binding: " << ex.what() << " val: " << ex.value() );
        quit();
    }
}

void OSCMessenger::sendAlive() {
    osc::Message msg("/lambda/world/alive");
    msg.append(_world->alive());
    _sender.send(msg);
}

void OSCMessenger::sendStates() {
    osc::Message msg("/lambda/world/states");
    for (int i = 0; i < _world->getQueryStatesSize(); i++) {
        msg.append(_world->getQueryStateAtIndex(i));
    }
    _sender.send(msg);
}

void OSCMessenger::sendFaderStates() {
    osc::Message msg("/lambda/world/faderstates");
    for (int i = 0; i < _world->getQueryStatesSize(); i++) {
        msg.append(_world->getQueryFaderStateItem(i));
    }
    _sender.send(msg);
}

void OSCMessenger::sendCoordinatesByState() {
    osc::Message msg("/lambda/world/coords");
    for (int i = 0; i < _world->getQueryStatesSize(); i++) {
        msg.append(_world->getQueryCoordAtIndex(i)); // Use append instead of addIntArg
    }
    _sender.send(msg);
}

void OSCMessenger::setUpListener() {
    _listener.setListener( "/lambda/world/init",
    [&]( const osc::Message &msg ){
        _world->init(msg.getArgInt32(0), msg.getArgInt32(1), msg.getArgInt32(2), msg.getArgInt32(3));
    });
    _listener.setListener( "/lambda/world/interpl",
    [&]( const osc::Message &msg ){
        _world->setInterpolation((Interpolation)msg.getArgInt32(0), msg.getArgInt32(1));
    });
    _listener.setListener( "/lambda/world/somvector",
    [&]( const osc::Message &msg ){
        if (!_world->inputVectorUpdated() && !_world->newBMUFound() && _world->vectorSize() == msg.getNumArgs()) {
            vector<double> inputVector;
            for (int i = 0; i < _world->vectorSize(); i++) {
                inputVector.push_back(msg.getArgFloat(i));
            }
            _world->setInputVector(inputVector);
        }
    });
    _listener.setListener( "/lambda/world/rule/init",
    [&]( const osc::Message &msg ){
        _world->initRule((R)msg.getArgInt32(0));
        _world->mapStates();
    });
    _listener.setListener( "/lambda/world/rule/births",
    [&]( const osc::Message &msg ){
        int *b;
        b = new int[msg.getNumArgs()];
        for (int i = 0; i < msg.getNumArgs(); i++) {
            b[i] = msg.getArgInt32(i);
        }
        _world->rule()->setBirths(b);
        delete [] b;
    });
    _listener.setListener( "/lambda/world/rule/survivals",
    [&]( const osc::Message &msg ){
        int *s;
        s = new int[msg.getNumArgs()];
        for (int i = 0; i < msg.getNumArgs(); i++) {
            s[i] = msg.getArgInt32(i);
        }
        _world->rule()->setSurvivals(s);
        delete [] s;
    });
    _listener.setListener( "/lambda/world/rule/states",
    [&]( const osc::Message &msg ){
        _world->rule()->setStates(msg.getArgInt32(0));
    });
    _listener.setListener( "/lambda/world/rule/add",
    [&]( const osc::Message &msg ){
        _world->rule()->setAdd((double)msg.getArgFloat(0));
    });
    _listener.setListener( "/lambda/world/rule/weights",
    [&]( const osc::Message &msg ){
        double* w;
        w = new double[_world->rule()->nSize()];
        for (int i = 0; i < _world->rule()->nSize(); i++) {
            w[i] = (double)msg.getArgFloat(i);
        }
        _world->rule()->setWeights(w);
        delete [] w;
    });
    _listener.setListener( "/lambda/world/reset/rand",
    [&]( const osc::Message &msg ){
        bool include = msg.getArgInt32(7) == 1;
        _world->initRandInArea(
            msg.getArgInt32(0),
            msg.getArgInt32(1),
            msg.getArgInt32(2),
            msg.getArgInt32(3),
            msg.getArgInt32(4),
            msg.getArgInt32(5),
            _world->rule()->numStates() - 1,
            msg.getArgFloat(6),
            include
        );
    });
    _listener.setListener( "/lambda/world/reset/wirecube",
    [&]( const osc::Message &msg ){
        _world->initWireCube(
            msg.getArgInt32(0),
            msg.getArgInt32(1),
            msg.getArgInt32(2),
            msg.getArgInt32(3),
            msg.getArgInt32(4),
            msg.getArgInt32(5)
        );
    });
    _listener.setListener( "/lambda/world/query/states",
    [&]( const osc::Message &msg ){
        int* ind;
        ind = new int[msg.getNumArgs()];
        for (int i = 0; i < msg.getNumArgs(); i++) {
            ind[i] = msg.getArgInt32(i);
        }
        _world->setQueryIndices(ind, msg.getNumArgs());
        delete [] ind;
    });
    _listener.setListener( "/lambda/world/query/coords",
    [&]( const osc::Message &msg ){
        _world->setQueryStates(msg.getArgInt32(0), msg.getArgInt32(1));
    });
    _listener.setListener( "/lambda/world/query/alive",
    [&]( const osc::Message &msg ){
        _world->startQuery();
    });
    _listener.setListener( "/lambda/world/query/stop",
    [&]( const osc::Message &msg ){
        _world->stopQuery();
    });
    _listener.setListener( "/lambda/world/symmetry",
    [&]( const osc::Message &msg ){
        _world->symmetry = (Sym)msg.getArgInt32(0);
    });
    _listener.setListener( "/lambda/graphics/rotate",
    [&]( const osc::Message &msg ){
        _ogl->rotateXYZ = vec3(
            msg.getArgFloat(0),
            msg.getArgFloat(1),
            msg.getArgFloat(2)
        );
        _ogl->rotateAngle = msg.getArgFloat(3);
    });
    _listener.setListener( "/lambda/graphics/view",
    [&]( const osc::Message &msg ){
        _ogl->mEye = vec3(
            msg.getArgFloat(0),
            msg.getArgFloat(1),
            msg.getArgFloat(2)
        );
        _ogl->mCenter = vec3(
            msg.getArgFloat(3),
            msg.getArgFloat(4),
            msg.getArgFloat(5)
        );
    });
    _listener.setListener( "/lambda/graphics/boidcam",
    [&]( const osc::Message &msg ){
        _ogl->attachEyeToFirstBoid = msg.getArgInt32(0) == 1;
        _ogl->lookAtCentroid = msg.getArgInt32(1) == 1;
    });
    _listener.setListener( "/lambda/graphics/background",
    [&]( const osc::Message &msg ){
        _ogl->setBackground(
            msg.getArgFloat(0),
            msg.getArgFloat(1),
            msg.getArgFloat(2)
        );
    });
    _listener.setListener( "/lambda/graphics/pattern",
    [&]( const osc::Message &msg ){
        int patternId = msg.getArgInt32(0);
        Pattern* pattern = _ogl->getPattern(patternId);
        if (pattern) {
            pattern->setActive(msg.getArgInt32(1) == 1);
            pattern->setAlpha(msg.getArgFloat(2));
            pattern->setColorMap(msg.getArgInt32(3));
            pattern->setAlphaMap(msg.getArgInt32(4));
            pattern->setColor(Color(msg.getArgFloat(5), msg.getArgFloat(6), msg.getArgFloat(7)));
        }
    });
    _listener.setListener( "/lambda/graphics/pattern/audio",
    [&]( const osc::Message &msg ){
        int patternId = msg.getArgInt32(0);
        Pattern* pattern = _ogl->getPattern(patternId);
        if (pattern) {
            pattern->setAudioReactivity(msg.getArgFloat(1));
        }
    });

    // Audio input enable/disable
    _listener.setListener( "/lambda/audio/input/enable",
    [&]( const osc::Message &msg ){
        bool enable = msg.getArgInt32(0) == 1;
        _ogl->enableAudioInput(enable);
        console() << "Audio input " << (enable ? "enabled" : "disabled") << " via OSC" << std::endl;
    });

    // Audio output capture (requires loopback driver)
    _listener.setListener( "/lambda/audio/output/enable",
    [&]( const osc::Message &msg ){
        bool enable = msg.getArgInt32(0) == 1;
        if (enable) {
            _ogl->setupAudioInput(true);  // true = use output device
            _ogl->enableAudioInput(true);
        } else {
            _ogl->enableAudioInput(false);
        }
        console() << "Audio output capture " << (enable ? "enabled" : "disabled") << " via OSC" << std::endl;
    });

    // Select specific audio device by name
    _listener.setListener( "/lambda/audio/device",
    [&]( const osc::Message &msg ){
        std::string deviceName = msg.getArgString(0);
        _ogl->setupAudioFromDevice(deviceName);
        console() << "Setting audio device to: " << deviceName << std::endl;
    });

    // Audio input gain
    _listener.setListener( "/lambda/audio/gain",
    [&]( const osc::Message &msg ){
        float gain = msg.getArgFloat(0);
        _ogl->setAudioInputGain(gain);
        console() << "Audio input gain set to: " << gain << std::endl;
    });

    // Waveform display toggle
    _listener.setListener( "/lambda/audio/waveform",
    [&]( const osc::Message &msg ){
        bool enable = msg.getArgInt32(0) == 1;
        _ogl->mShowWaveform = enable;
        console() << "Waveform display " << (enable ? "enabled" : "disabled") << " via OSC" << std::endl;
    });

    // MFCC display toggle
    _listener.setListener( "/lambda/audio/mfcc",
    [&]( const osc::Message &msg ){
        bool enable = msg.getArgInt32(0) == 1;
        _ogl->mShowMFCC = enable;
        console() << "MFCC display " << (enable ? "enabled" : "disabled") << " via OSC" << std::endl;
    });

    // Waveform color (RGB floats 0.0-1.0)
    _listener.setListener( "/lambda/audio/waveform/color",
    [&]( const osc::Message &msg ){
        float r = msg.getArgFloat(0);
        float g = msg.getArgFloat(1);
        float b = msg.getArgFloat(2);
        _ogl->mWaveformColor = vec3(r, g, b);
        console() << "Waveform color set to RGB(" << r << ", " << g << ", " << b << ")" << std::endl;
    });

    // Waveform 3D ribbon mode toggle (int: 0=2D, 1=3D)
    _listener.setListener( "/lambda/audio/waveform/ribbon",
    [&]( const osc::Message &msg ){
        bool enable = msg.getArgInt32(0) == 1;
        _ogl->mWaveformRibbon3D = enable;
        console() << "Waveform ribbon 3D mode " << (enable ? "enabled" : "disabled") << " via OSC" << std::endl;
    });

    // Ribbon trail depth spacing (float, default 50.0)
    _listener.setListener( "/lambda/audio/waveform/ribbon/depth",
    [&]( const osc::Message &msg ){
        float depth = msg.getArgFloat(0);
        _ogl->mRibbonDepthSpacing = depth;
        console() << "Ribbon depth spacing set to " << depth << std::endl;
    });

    // Ribbon trail fade rate (float, default 0.08)
    _listener.setListener( "/lambda/audio/waveform/ribbon/fade",
    [&]( const osc::Message &msg ){
        float fade = msg.getArgFloat(0);
        _ogl->mRibbonFadeRate = fade;
        console() << "Ribbon fade rate set to " << fade << std::endl;
    });

    // Ribbon trail number of layers (int, default 12, max 32)
    _listener.setListener( "/lambda/audio/waveform/ribbon/layers",
    [&]( const osc::Message &msg ){
        int layers = msg.getArgInt32(0);
        layers = std::max(1, std::min(32, layers));  // Clamp to 1-32
        _ogl->mWaveformRibbonLayers = layers;
        console() << "Ribbon layers set to " << layers << std::endl;
    });

    // MFCC hue start (float 0.0-1.0)
    _listener.setListener( "/lambda/audio/mfcc/hue/start",
    [&]( const osc::Message &msg ){
        float hue = msg.getArgFloat(0);
        _ogl->mMFCCHueStart = hue;
        console() << "MFCC hue start set to " << hue << std::endl;
    });

    // MFCC hue range (float 0.0-1.0)
    _listener.setListener( "/lambda/audio/mfcc/hue/range",
    [&]( const osc::Message &msg ){
        float range = msg.getArgFloat(0);
        _ogl->mMFCCHueRange = range;
        console() << "MFCC hue range set to " << range << std::endl;
    });

    // Boid pattern system - matches old lambda app format
    _listener.setListener( "/lambda/graphics/boidpattern",
    [&]( const osc::Message &msg ){
        // Old format: patternId(int), active(int), mapIndex(int)
        // mapIndex was used for different cubemap image sets (0-4) - now ignored
        int patternId = msg.getArgInt32(0);
        bool active = msg.getArgInt32(1) == 1;
        int mapIndex = msg.getArgInt32(2);  // Read for compatibility but ignore

        CI_LOG_I("Received boidpattern OSC: patternId=" << patternId << " active=" << active << " mapIndex=" << mapIndex);

        // Update new boid pattern system
        BoidPattern* pattern = _ogl->getBoidPattern(patternId);
        if (pattern) {
            pattern->setActive(active);
            CI_LOG_I("Set pattern " << patternId << " to active=" << active);
        } else {
            CI_LOG_E("Pattern " << patternId << " not found!");
        }

        // Keep legacy system for backward compatibility
        if (patternId >= 0 && patternId < numBoidPatterns) {
            _ogl->boidPatternLib[patternId].active = active;
            _ogl->boidPatternLib[patternId].mapIndex = mapIndex;  // Kept for legacy compatibility
        }
    });
    _listener.setListener( "/lambda/boids/init",
    [&]( const osc::Message &msg ){
        if (_boids) {
            delete _boids;
        }
        _boids = new Boids(
            msg.getArgInt32(0),
            vec3(msg.getArgFloat(1), msg.getArgFloat(2), msg.getArgFloat(3)),
            msg.getArgFloat(4),
            msg.getArgFloat(5),
            msg.getArgFloat(6),
            msg.getArgFloat(7),
            msg.getArgFloat(8)
        );
        _ogl->boids = _boids;
    });
    _listener.setListener( "/lambda/boids/set",
    [&]( const osc::Message &msg ){
        _boids->speed = msg.getArgFloat(0);
        _boids->cohesion = msg.getArgFloat(1);
        _boids->alignment = msg.getArgFloat(2);
        _boids->separation = msg.getArgFloat(3);
        _boids->center = msg.getArgFloat(4);
        // Update base values when manually setting parameters
        _boids->baseCohesion = _boids->cohesion;
        _boids->baseSeparation = _boids->separation;
        _boids->baseAlignment = _boids->alignment;
    });
    _listener.setListener( "/lambda/boids/audio",
    [&]( const osc::Message &msg ){
        // Set audio reactivity for cohesion, separation, alignment
        // Values 0.0-1.0 determine how much audio affects each parameter
        _boids->audioReactivityCohesion = msg.getArgFloat(0);
        _boids->audioReactivitySeparation = msg.getArgFloat(1);
        _boids->audioReactivityAlignment = msg.getArgFloat(2);
    });
    _listener.setListener( "/lambda/boids/kill",
    [&]( const osc::Message &msg ){
        delete _boids;
        _boids = nullptr;
        _ogl->boids = nullptr;
    });
    // Additional boid pattern control messages
    _listener.setListener( "/lambda/boids/pattern/active",
    [&]( const osc::Message &msg ){
        int patternId = msg.getArgInt32(0);
        bool active = msg.getArgInt32(1) == 1;
        BoidPattern* pattern = _ogl->getBoidPattern(patternId);
        if (pattern) {
            pattern->setActive(active);
        }
    });
    _listener.setListener( "/lambda/boids/pattern/color",
    [&]( const osc::Message &msg ){
        // Read pattern ID - try as int first, fall back to float
        int patternId;
        try {
            patternId = msg.getArgInt32(0);
        } catch (...) {
            patternId = (int)msg.getArgFloat(0);
        }
        BoidPattern* pattern = _ogl->getBoidPattern(patternId);
        if (pattern) {
            pattern->setColor(Color(msg.getArgFloat(1), msg.getArgFloat(2), msg.getArgFloat(3)));
        }
    });
    _listener.setListener( "/lambda/boids/pattern/alpha",
    [&]( const osc::Message &msg ){
        int patternId = (int)msg.getArgFloat(0);  // Accept float and convert to int
        BoidPattern* pattern = _ogl->getBoidPattern(patternId);
        if (pattern) {
            pattern->setAlpha(msg.getArgFloat(1));
        }
    });
    // Code panel / live coding messages
    _listener.setListener( "/lambda/livecode/activate",
    [&]( const osc::Message &msg ){
        _ogl->codePanelActive = msg.getArgInt32(0) == 1;
    });
    _listener.setListener( "/lambda/livecode/map",
    [&]( const osc::Message &msg ){
        _ogl->codePanelMapped = msg.getArgInt32(0) == 1;
    });
    _listener.setListener( "/lambda/livecode/codeline",
    [&]( const osc::Message &msg ){
        _ogl->codePanel.addLine(msg.getArgString(0));
    });
    _listener.setListener( "/lambda/livecode/codetitle",
    [&]( const osc::Message &msg ){
        _ogl->codePanel.title = msg.getArgString(0);
    });
    _listener.setListener( "/lambda/livecode/fadeTime",
    [&]( const osc::Message &msg ){
        _ogl->codePanel.fadeTime = msg.getArgInt32(0);
    });
    _listener.setListener( "/lambda/livecode/codecolour",
    [&]( const osc::Message &msg ){
        _ogl->codePanel.setCodeColor(msg.getArgFloat(0), msg.getArgFloat(1), msg.getArgFloat(2));
    });
    _listener.setListener( "/lambda/livecode/codefont",
    [&]( const osc::Message &msg ){
        _ogl->codePanel.setCodeFont(msg.getArgString(0), msg.getArgInt32(1));
    });
    _listener.setListener( "/lambda/efx/enable",
    [&]( const osc::Message &msg ){
        std::string type = msg.getArgString(0);
        bool enabled = msg.getArgInt32(1) == 1;
        _ogl->setEffect(type, enabled);
    });
    _listener.setListener( "/lambda/efx/params",
    [&]( const osc::Message &msg ){
        std::vector<float> params;
        for (int i = 0; i < msg.getNumArgs(); i++) {
            // Try to get as float, fall back to int if needed
            try {
                if (msg.getArgType(i) == osc::ArgType::INTEGER_32) {
                    params.push_back(static_cast<float>(msg.getArgInt32(i)));
                } else {
                    params.push_back(msg.getArgFloat(i));
                }
            }
            catch (const std::exception& e) {
                console() << "Error parsing param " << i << ": " << e.what() << std::endl;
            }
        }
        _ogl->setEffectParams(params);
    });
    // Water simulation / caustics / cymatics
    _listener.setListener( "/lambda/water/drop",
    [&]( const osc::Message &msg ){
        // args: x y radius strength  (all normalised 0..1 except strength)
        float x        = msg.getArgFloat(0);
        float y        = msg.getArgFloat(1);
        float radius   = msg.getArgFloat(2);
        float strength = msg.getArgFloat(3);
        _ogl->addWaterDrop(x, y, radius, strength);
    });
    _listener.setListener( "/lambda/water/cymatics",
    [&]( const osc::Message &msg ){
        bool on = msg.getArgInt32(0) == 1;
        if (_ogl->mWaterSim) _ogl->mWaterSim->cymatics = on;
        console() << "Cymatics: " << (on ? "on" : "off") << std::endl;
    });
    _listener.setListener( "/lambda/water/damping",
    [&]( const osc::Message &msg ){
        if (_ogl->mWaterSim) _ogl->mWaterSim->damping = msg.getArgFloat(0);
    });
    _listener.setListener( "/lambda/water/speed",
    [&]( const osc::Message &msg ){
        if (_ogl->mWaterSim) _ogl->mWaterSim->waveSpeed = msg.getArgFloat(0);
    });

    // Standalone water scene (pool + caustics, evanw approach)
    _listener.setListener( "/lambda/liquid/show",
    [&]( const osc::Message &msg ){
        bool on = msg.getArgInt32(0) == 1;
        _ogl->mWaterScene.isVisible = on;
        if (on) {
            _ogl->mWaterScene.drawPool    = true;
            _ogl->mWaterScene.drawSurface = true;
        }
        console() << "WaterScene: " << (on ? "visible" : "hidden") << std::endl;
    });
    _listener.setListener( "/lambda/liquid/camera",
    [&]( const osc::Message &msg ){
        // args: eyeX eyeY eyeZ  (optional: targetX targetY targetZ)
        _ogl->mWaterScene.cameraEye = vec3(
            msg.getArgFloat(0), msg.getArgFloat(1), msg.getArgFloat(2));
        if (msg.getNumArgs() >= 6) {
            _ogl->mWaterScene.cameraTarget = vec3(
                msg.getArgFloat(3), msg.getArgFloat(4), msg.getArgFloat(5));
        }
    });
    _listener.setListener( "/lambda/liquid/drop",
    [&]( const osc::Message &msg ){
        // args: worldX worldZ radius strength
        float x = msg.getArgFloat(0);
        float z = msg.getArgFloat(1);
        float r = msg.getArgFloat(2);
        float s = msg.getArgFloat(3);
        console() << "liquid/drop x=" << x << " z=" << z << " r=" << r << " s=" << s << std::endl;
        _ogl->mWaterScene.addDrop(x, z, r, s);
    });
    _listener.setListener( "/lambda/liquid/cymatics",
    [&]( const osc::Message &msg ){
        _ogl->mWaterScene.cymatics = msg.getArgInt32(0) == 1;
    });
    _listener.setListener( "/lambda/liquid/damping",
    [&]( const osc::Message &msg ){
        _ogl->mWaterScene.damping = msg.getArgFloat(0);
    });
    _listener.setListener( "/lambda/liquid/speed",
    [&]( const osc::Message &msg ){
        _ogl->mWaterScene.waveSpeed = msg.getArgFloat(0);
    });
    _listener.setListener( "/lambda/liquid/caustic",
    [&]( const osc::Message &msg ){
        // args: strength scale
        _ogl->mWaterScene.causticStrength = msg.getArgFloat(0);
        if (msg.getNumArgs() > 1) _ogl->mWaterScene.causticScale = msg.getArgFloat(1);
    });
    _listener.setListener( "/lambda/liquid/surface",
    [&]( const osc::Message &msg ){
        // args: show(0|1) alpha heightScale
        _ogl->mWaterScene.drawSurface = msg.getArgInt32(0) == 1;
        if (msg.getNumArgs() > 1) _ogl->mWaterScene.waterAlpha  = msg.getArgFloat(1);
        if (msg.getNumArgs() > 2) _ogl->mWaterScene.heightScale = msg.getArgFloat(2);
    });
    _listener.setListener( "/lambda/liquid/color",
    [&]( const osc::Message &msg ){
        // args: r g b  (water surface color)
        _ogl->mWaterScene.waterColor = vec3(
            msg.getArgFloat(0), msg.getArgFloat(1), msg.getArgFloat(2));
    });
    _listener.setListener( "/lambda/liquid/wallcolor",
    [&]( const osc::Message &msg ){
        // args: r g b  (pool floor/wall base color)
        _ogl->mWaterScene.poolColor = vec3(
            msg.getArgFloat(0), msg.getArgFloat(1), msg.getArgFloat(2));
    });
    _listener.setListener( "/lambda/liquid/light",
    [&]( const osc::Message &msg ){
        // args: x y z  (world-space light direction for caustics; y should be negative)
        _ogl->mWaterScene.lightDir = vec3(
            msg.getArgFloat(0), msg.getArgFloat(1), msg.getArgFloat(2));
    });
    _listener.setListener( "/lambda/liquid/autoDrop",
    [&]( const osc::Message &msg ){
        _ogl->mWaterScene.autoDrop = msg.getArgInt32(0) == 1;
    });

    _listener.setListener( "/lambda/framerate",
    [&]( const osc::Message &msg ){
        setFrameRate(msg.getArgFloat(0));
    });
    _listener.setListener( "/lambda/quit",
    [&]( const osc::Message &msg ){
        _receivedQuit = true;
    });
    
    try {
        _listener.bind();
    }
    catch( const osc::Exception &ex ) {
        CI_LOG_E( "Error binding: " << ex.what() << " val: " << ex.value() );
        quit();
    }

    _listener.listen(
    []( asio::error_code error, protocol::endpoint endpoint ) -> bool {
        if( error ) {
            CI_LOG_E( "Error Listening: " << error.message() << " val: " << error.value() << " endpoint: " << endpoint );
            return false;
        }
        else
            return true;
    });

}
