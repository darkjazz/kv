/*
 *  ogl.cpp
 *  lambda
 *
 *  Created by alo on 22/04/2011.
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

#include "ogl.h"
#include "cinder/BSpline.h"
#include "cinder/audio/Context.h"
#include "cinder/audio/InputNode.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Device.h"
#include <algorithm>

void GraphicsRenderer::setupOgl () {

	_bgr = _bgg = _bgb = 0.0;

	gl::clear( Color( _bgr, _bgg, _bgb ) );

	mEye = vec3(0.0f, 0.0f, -30.0f);
	mCenter = vec3(0.0f, 0.0f, 0.0f);
	mUp = vec3(0.0f, 1.0f, 0.0f);

	mCam.lookAt(mEye, mCenter, mUp);

//	glEnable( GL_TEXTURE_2D );
//	glDisable( GL_TEXTURE_2D );

	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::enableAlphaBlending();

	// Load instance rendering shader
	try {
		auto vertPath = app::loadAsset("instance.vert");
		auto fragPath = app::loadAsset("instance.frag");
		console() << "Loading instance shaders from assets..." << std::endl;
		mInstanceShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Instance shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading instance shader: " << e.what() << std::endl;
	}

	// Load textured instance rendering shader
	try {
		auto vertPath = app::loadAsset("instance_textured.vert");
		auto fragPath = app::loadAsset("instance_textured.frag");
		console() << "Loading textured instance shaders from assets..." << std::endl;
		mInstanceTexturedShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Textured instance shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading textured instance shader: " << e.what() << std::endl;
	}

	// Load environment map shader
	try {
		auto vertPath = app::loadAsset("envmap_sphere.vert");
		auto fragPath = app::loadAsset("envmap_sphere.frag");
		console() << "Loading environment map shaders from assets..." << std::endl;
		mEnvMapShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Environment map shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading environment map shader: " << e.what() << std::endl;
	}

	// Load cubemap textures (fxic_* for Pattern05)
	try {
		console() << "Loading fxic_* cubemap textures..." << std::endl;

		// Load each face - order: +X, -X, +Y, -Y, +Z, -Z
		ImageSourceRef images[6];
		images[0] = loadImage(app::loadAsset("fxic_pos_x.png"));
		images[1] = loadImage(app::loadAsset("fxic_neg_x.png"));
		images[2] = loadImage(app::loadAsset("fxic_pos_y.png"));
		images[3] = loadImage(app::loadAsset("fxic_neg_y.png"));
		images[4] = loadImage(app::loadAsset("fxic_pos_z.png"));
		images[5] = loadImage(app::loadAsset("fxic_neg_z.png"));

		gl::TextureCubeMap::Format fmt;
		fmt.setMagFilter(GL_LINEAR);
		fmt.setMinFilter(GL_LINEAR);
		fmt.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		// Create cubemap from image array
		mCubeMap = gl::TextureCubeMap::create(images, fmt);
		console() << "fxic_* cubemap loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading fxic_* cubemap: " << e.what() << std::endl;
	}

	// Load second cubemap textures (fxp_* for Pattern13)
	try {
		console() << "Loading fxp_* cubemap textures..." << std::endl;

		// Load each face - order: +X, -X, +Y, -Y, +Z, -Z
		ImageSourceRef images[6];
		images[0] = loadImage(app::loadAsset("fxp_pos_x.png"));
		images[1] = loadImage(app::loadAsset("fxp_neg_x.png"));
		images[2] = loadImage(app::loadAsset("fxp_pos_y.png"));
		images[3] = loadImage(app::loadAsset("fxp_neg_y.png"));
		images[4] = loadImage(app::loadAsset("fxp_pos_z.png"));
		images[5] = loadImage(app::loadAsset("fxp_neg_z.png"));

		gl::TextureCubeMap::Format fmt;
		fmt.setMagFilter(GL_LINEAR);
		fmt.setMinFilter(GL_LINEAR);
		fmt.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		// Create cubemap from image array
		mCubeMap2 = gl::TextureCubeMap::create(images, fmt);
		console() << "fxp_* cubemap loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading fxp_* cubemap: " << e.what() << std::endl;
	}

	// Load line rendering shader
	try {
		auto vertPath = app::loadAsset("line.vert");
		auto fragPath = app::loadAsset("line.frag");
		console() << "Loading line shaders from assets..." << std::endl;
		mLineShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Line shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading line shader: " << e.what() << std::endl;
		// Try loading from Resources directory directly
		try {
			console() << "Trying to load shaders from source location..." << std::endl;
			auto vertSource = ci::DataSourcePath::create(getAppPath() / ".." / "Resources" / "line.vert");
			auto fragSource = ci::DataSourcePath::create(getAppPath() / ".." / "Resources" / "line.frag");
			mLineShader = gl::GlslProg::create(vertSource, fragSource);
			console() << "Line shader loaded from Resources" << std::endl;
		}
		catch (const std::exception& e2) {
			console() << "Failed to load line shader from Resources: " << e2.what() << std::endl;
		}
	}

	// Load spherical quad shader
	try {
		auto vertPath = app::loadAsset("spherical_quad.vert");
		auto fragPath = app::loadAsset("spherical_quad.frag");
		console() << "Loading spherical quad shaders from assets..." << std::endl;
		mSphericalQuadShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Spherical quad shader loaded successfully" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading spherical quad shader: " << e.what() << std::endl;
	}

	// Create cube mesh geometry with texture coordinates
	auto cubeGeom = geom::Cube().size(vec3(1.0f));
	mCubeMesh = gl::VboMesh::create(cubeGeom);

	// Check if mesh has texture coordinates by checking the geom source
	console() << "Cube geometry created - checking for texture coordinates..." << std::endl;
	if (cubeGeom.getAvailableAttribs().count(geom::Attrib::TEX_COORD_0) > 0) {
		console() << "  Cube mesh HAS texture coordinates!" << std::endl;
	} else {
		console() << "  WARNING: Cube mesh does NOT have texture coordinates!" << std::endl;
	}

	// Create sphere mesh geometry
	mSphereMesh = gl::VboMesh::create(geom::Sphere().subdivisions(16));

	// Create cylinder mesh geometry
	mCylinderMesh = gl::VboMesh::create(geom::Cylinder().subdivisionsAxis(16).subdivisionsHeight(1));

	// Create line mesh geometry (simple 2-vertex line)
	// First vertex has x=0 (start), second has x=1 (end)
	std::vector<float> lineVerts = {
		0.0f, 0.0f, 0.0f,  // First vertex (t=0)
		1.0f, 0.0f, 0.0f   // Second vertex (t=1)
	};
	auto lineGeom = geom::BufferLayout();
	lineGeom.append(geom::Attrib::POSITION, 3, sizeof(float) * 3, 0);
	auto lineVbo = gl::Vbo::create(GL_ARRAY_BUFFER, lineVerts.size() * sizeof(float), lineVerts.data(), GL_STATIC_DRAW);
	mLineMesh = gl::VboMesh::create(2, GL_LINES, { {lineGeom, lineVbo} });

	// Create FBOs for audio visualization textures
	try {
		gl::Fbo::Format format;
		format.setColorTextureFormat(gl::Texture::Format().internalFormat(GL_RGBA8));
		format.depthBuffer();  // Add depth buffer for proper 3D rendering

		// Waveform FBO (1920x400 - wide aspect for waveform display)
		mWaveformFbo = gl::Fbo::create(1920, 400, format);
		console() << "Waveform FBO created: 1920x400" << std::endl;

		// MFCC FBO (800x400 - narrower aspect for bar chart)
		mMFCCFbo = gl::Fbo::create(800, 400, format);
		console() << "MFCC FBO created: 800x400" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error creating audio visualization FBOs: " << e.what() << std::endl;
	}

	// Initialize post-processing FBOs (including audio visualization FBOs)
	setupPostProcessing();

}

void GraphicsRenderer::reshape() {

	mCam.setPerspective(45.0, getWindowAspectRatio(), 0.1f, 2000.0f);
	gl::setMatrices( mCam );

	// Recreate FBO on window resize if any effect is active
	if (mCurrentEffect != EFFECT_NONE && mFbo) {
		setupPostProcessing();
	}

}

void GraphicsRenderer::setupPostProcessing() {
	console() << "Setting up post-processing effects..." << std::endl;

	// Create FBO for rendering scene to texture
	try {
		gl::Fbo::Format format;
		format.setSamples(4);  // 4x MSAA
		format.setColorTextureFormat(gl::Texture::Format().internalFormat(GL_RGBA8));
		mFbo = gl::Fbo::create(getWindowWidth(), getWindowHeight(), format);
		console() << "FBO created: " << getWindowWidth() << "x" << getWindowHeight() << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error creating FBO: " << e.what() << std::endl;
		mCurrentEffect = EFFECT_NONE;
		return;
	}

	// Load all effect shaders
	try {
		auto vertPath = app::loadAsset("blur.vert");
		auto fragPath = app::loadAsset("blur.frag");
		mBlurShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Blur shader loaded" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading blur shader: " << e.what() << std::endl;
	}

	try {
		auto vertPath = app::loadAsset("blur.vert");  // Reuse same vertex shader
		auto fragPath = app::loadAsset("radial.frag");
		mRadialShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Radial shader loaded" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading radial shader: " << e.what() << std::endl;
	}

	try {
		auto vertPath = app::loadAsset("blur.vert");  // Reuse same vertex shader
		auto fragPath = app::loadAsset("motion.frag");
		mMotionShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Motion shader loaded" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading motion shader: " << e.what() << std::endl;
	}

	try {
		auto vertPath = app::loadAsset("blur.vert");  // Reuse same vertex shader
		auto fragPath = app::loadAsset("glitch.frag");
		mGlitchShader = gl::GlslProg::create(vertPath, fragPath);
		console() << "Glitch shader loaded" << std::endl;
	}
	catch (const std::exception& e) {
		console() << "Error loading glitch shader: " << e.what() << std::endl;
	}

	// Create fullscreen quad for post-processing (shader will be set dynamically)
	auto rect = geom::Rect(Rectf(-1, -1, 1, 1));
	if (mBlurShader) {
		mFullscreenQuad = gl::Batch::create(rect, mBlurShader);
	}

	console() << "Post-processing setup complete" << std::endl;
}

void GraphicsRenderer::setEffect(const std::string& type, bool enabled) {
	if (!enabled) {
		mCurrentEffect = EFFECT_NONE;
		console() << "Effect disabled" << std::endl;
		return;
	}

	// Map string to effect type
	if (type == "blur") {
		mCurrentEffect = EFFECT_BLUR;
		// Default blur params: [amount]
		mEffectParams = {0.5f};
		console() << "Blur effect enabled (amount: 0.5)" << std::endl;
	}
	else if (type == "radial") {
		mCurrentEffect = EFFECT_RADIAL;
		// Default radial params: [centerX, centerY, amount, samples]
		mEffectParams = {0.5f, 0.5f, 0.0f, 12.0f};
		console() << "Radial blur effect enabled (center: 0.5,0.5 amount: 0.0 samples: 12)" << std::endl;
	}
	else if (type == "motion") {
		mCurrentEffect = EFFECT_MOTION;
		// Default motion params: [angle, amount, samples]
		mEffectParams = {0.0f, 0.0f, 12.0f};
		console() << "Motion blur effect enabled (angle: 0 amount: 0.0 samples: 12)" << std::endl;
	}
	else if (type == "glitch") {
		mCurrentEffect = EFFECT_GLITCH;
		// Default glitch params: [amount, time, rgbOffset, blockiness]
		mEffectParams = {0.5f, 0.0f, 1.0f, 1.0f};
		console() << "Glitch effect enabled (amount: 0.5 rgbOffset: 1.0 blockiness: 1.0)" << std::endl;
	}
	else {
		console() << "Unknown effect type: " << type << std::endl;
		mCurrentEffect = EFFECT_NONE;
	}
}

void GraphicsRenderer::setEffectParams(const std::vector<float>& params) {
	mEffectParams = params;
	console() << "Effect params updated: " << params.size() << " values" << std::endl;
}

void GraphicsRenderer::applyEffect() {
	// Unbind FBO and render to screen
	mFbo->unbindFramebuffer();

	// Clear screen with background color
	gl::clear(Color(_bgr, _bgg, _bgb));

	// Disable depth test for fullscreen quad
	gl::ScopedDepth scopedDepth(false);

	// Use identity matrices for fullscreen quad in NDC (-1 to 1)
	gl::ScopedMatrices scopedMatrices;
	gl::setMatrices(CameraOrtho(-1, 1, -1, 1, -1, 1));

	// Select shader based on current effect
	gl::GlslProgRef shader;
	switch (mCurrentEffect) {
		case EFFECT_BLUR:
			shader = mBlurShader;
			break;
		case EFFECT_RADIAL:
			shader = mRadialShader;
			break;
		case EFFECT_MOTION:
			shader = mMotionShader;
			break;
		case EFFECT_GLITCH:
			shader = mGlitchShader;
			break;
		default:
			return;  // No effect
	}

	if (!shader) return;

	// Bind shader and set uniforms
	gl::ScopedGlslProg scopedShader(shader);
	shader->uniform("uTexture", 0);

	// Set effect-specific uniforms
	switch (mCurrentEffect) {
		case EFFECT_BLUR: {
			// Blur shader needs texel size for kernel sampling
			shader->uniform("uTexelSize", vec2(1.0f / getWindowWidth(), 1.0f / getWindowHeight()));
			// params[0] = amount
			float amount = mEffectParams.size() > 0 ? mEffectParams[0] : 0.5f;
			shader->uniform("uBlurAmount", amount);
			break;
		}
		case EFFECT_RADIAL: {
			// params[0] = centerX, params[1] = centerY, params[2] = amount, params[3] = samples
			vec4 params(
				mEffectParams.size() > 0 ? mEffectParams[0] : 0.5f,
				mEffectParams.size() > 1 ? mEffectParams[1] : 0.5f,
				mEffectParams.size() > 2 ? mEffectParams[2] : 0.0f,
				mEffectParams.size() > 3 ? mEffectParams[3] : 12.0f
			);
			shader->uniform("uParams", params);

			// Debug output (only every 60 frames to avoid spam)
			static int frameCounter = 0;
			if (frameCounter++ % 60 == 0) {
				console() << "Radial params: center(" << params.x << "," << params.y
				          << ") amount=" << params.z << " samples=" << params.w << std::endl;
			}
			break;
		}
		case EFFECT_MOTION: {
			// params[0] = angle, params[1] = amount, params[2] = samples
			vec4 params(
				mEffectParams.size() > 0 ? mEffectParams[0] : 0.0f,
				mEffectParams.size() > 1 ? mEffectParams[1] : 0.0f,
				mEffectParams.size() > 2 ? mEffectParams[2] : 12.0f,
				0.0f
			);
			shader->uniform("uParams", params);

			// Debug output (only every 60 frames to avoid spam)
			static int frameCounter = 0;
			if (frameCounter++ % 60 == 0) {
				console() << "Motion params: angle=" << params.x
				          << " amount=" << params.y << " samples=" << params.z << std::endl;
			}
			break;
		}
		case EFFECT_GLITCH: {
			// params[0] = amount, params[1] = time (auto-updated), params[2] = rgbOffset, params[3] = blockiness
			// Update time parameter automatically
			if (mEffectParams.size() > 1) {
				mEffectParams[1] = static_cast<float>(app::getElapsedSeconds());
			}
			vec4 params(
				mEffectParams.size() > 0 ? mEffectParams[0] : 0.5f,
				mEffectParams.size() > 1 ? mEffectParams[1] : 0.0f,
				mEffectParams.size() > 2 ? mEffectParams[2] : 1.0f,
				mEffectParams.size() > 3 ? mEffectParams[3] : 1.0f
			);
			shader->uniform("uParams", params);
			break;
		}
		default:
			break;
	}

	// Bind FBO texture
	gl::ScopedTextureBind scopedTexture(mFbo->getColorTexture(), 0);

	// Create batch on-the-fly with current shader (batch caches its shader, so we recreate it)
	auto rect = geom::Rect(Rectf(-1, -1, 1, 1)).texCoords(vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1));
	auto batch = gl::Batch::create(rect, shader);
	batch->draw();
}

void GraphicsRenderer::updateAudioFeatures() {
	// Extract audio features directly from MFCC input vector
	if (!ptrWorld || !ptrWorld->initialized()) {
		mAudioAmplitude = 0.0f;
		mAudioLowBand = 0.0f;
		mAudioMidBand = 0.0f;
		mAudioHighBand = 0.0f;
		return;
	}

	// Get the current MFCC input vector (sent from SuperCollider)
	const std::vector<double>& mfcc = ptrWorld->getInputVector();

	if (mfcc.empty()) {
		return;
	}

	// MFCC coefficients represent different frequency bands
	// MFCC 0 = overall energy (DC component)
	// MFCC 1-4 = low frequencies (bass)
	// MFCC 5-9 = mid frequencies
	// MFCC 10+ = high frequencies (treble)

	int numCoeffs = mfcc.size();

	// Extract total energy from MFCC[0] or sum of all coefficients
	float totalEnergy = 0.0f;
	for (int i = 0; i < numCoeffs; i++) {
		totalEnergy += std::abs(mfcc[i]);
	}
	totalEnergy /= numCoeffs;

	// Low frequency energy (MFCC 0-4)
	float lowEnergy = 0.0f;
	int lowEnd = std::min(5, numCoeffs);
	for (int i = 0; i < lowEnd; i++) {
		lowEnergy += std::abs(mfcc[i]);
	}
	if (lowEnd > 0) lowEnergy /= lowEnd;

	// Mid frequency energy (MFCC 5-9)
	float midEnergy = 0.0f;
	int midStart = 5;
	int midEnd = std::min(10, numCoeffs);
	if (midStart < numCoeffs) {
		for (int i = midStart; i < midEnd; i++) {
			midEnergy += std::abs(mfcc[i]);
		}
		midEnergy /= (midEnd - midStart);
	}

	// High frequency energy (MFCC 10+)
	float highEnergy = 0.0f;
	int highStart = 10;
	if (highStart < numCoeffs) {
		for (int i = highStart; i < numCoeffs; i++) {
			highEnergy += std::abs(mfcc[i]);
		}
		highEnergy /= (numCoeffs - highStart);
	}

	// Debug output every 60 frames - show RAW values and first few MFCCs
	static int debugCounter = 0;
	if (++debugCounter >= 60) {
		console() << "MFCC RAW: Amp=" << totalEnergy
		          << " Low=" << lowEnergy
		          << " Mid=" << midEnergy
		          << " High=" << highEnergy << " | ";

		// Show first 5 MFCC values to see if they're varying
		console() << "MFCCs[0-4]: ";
		for (int i = 0; i < std::min(5, numCoeffs); i++) {
			console() << mfcc[i] << " ";
		}
		console() << std::endl;
		debugCounter = 0;
	}

	// Track min/max for adaptive normalization (to expand dynamic range)
	static float minAmp = 1.0f, maxAmp = 0.0f;
	static float minLow = 1.0f, maxLow = 0.0f;
	static float minMid = 1.0f, maxMid = 0.0f;
	static float minHigh = 1.0f, maxHigh = 0.0f;

	// Update running min/max (with slow decay to adapt to changes)
	float decay = 0.999f;  // Very slow decay
	minAmp = std::min(minAmp * decay + totalEnergy * (1.0f - decay), totalEnergy);
	maxAmp = std::max(maxAmp * decay + totalEnergy * (1.0f - decay), totalEnergy);
	minLow = std::min(minLow * decay + lowEnergy * (1.0f - decay), lowEnergy);
	maxLow = std::max(maxLow * decay + lowEnergy * (1.0f - decay), lowEnergy);
	minMid = std::min(minMid * decay + midEnergy * (1.0f - decay), midEnergy);
	maxMid = std::max(maxMid * decay + midEnergy * (1.0f - decay), midEnergy);
	minHigh = std::min(minHigh * decay + highEnergy * (1.0f - decay), highEnergy);
	maxHigh = std::max(maxHigh * decay + highEnergy * (1.0f - decay), highEnergy);

	// Normalize to 0-1 range using adaptive min/max
	auto normalize = [](float val, float min, float max) {
		float range = max - min;
		if (range < 0.01f) return 0.5f;  // Avoid division by zero
		return std::min(1.0f, std::max(0.0f, (val - min) / range));
	};

	float normAmp = normalize(totalEnergy, minAmp, maxAmp);
	float normLow = normalize(lowEnergy, minLow, maxLow);
	float normMid = normalize(midEnergy, minMid, maxMid);
	float normHigh = normalize(highEnergy, minHigh, maxHigh);

	// Smooth the NORMALIZED values
	float smoothing = 0.3f;  // Lower smoothing for more responsiveness
	mAudioAmplitude = mAudioAmplitude * smoothing + normAmp * (1.0f - smoothing);
	mAudioLowBand = mAudioLowBand * smoothing + normLow * (1.0f - smoothing);
	mAudioMidBand = mAudioMidBand * smoothing + normMid * (1.0f - smoothing);
	mAudioHighBand = mAudioHighBand * smoothing + normHigh * (1.0f - smoothing);
}

void GraphicsRenderer::setupAudioInput(bool useOutput) {
	auto ctx = audio::Context::master();
	mUseOutputDevice = useOutput;

	// Get appropriate device (input or output)
	audio::DeviceRef device;
	if (useOutput) {
		device = audio::Device::getDefaultOutput();
		console() << "Attempting to use output device (requires loopback driver like BlackHole)" << std::endl;
	} else {
		device = audio::Device::getDefaultInput();
	}

	if (!device) {
		console() << "No default audio " << (useOutput ? "output" : "input") << " device found" << std::endl;
		return;
	}

	console() << "Using audio " << (useOutput ? "output" : "input") << " device: " << device->getName() << std::endl;

	// Create input node with selected device
	auto format = audio::Node::Format().autoEnable();
	mAudioInput = ctx->createInputDeviceNode(device, format);

	// Create FFT node for spectral analysis
	auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
	mMonitorSpectralNode = ctx->makeNode(new audio::MonitorSpectralNode(monitorFormat));

	// Connect input to FFT
	mAudioInput >> mMonitorSpectralNode;

	// Enable the input (but don't start processing yet)
	mAudioInput->enable();
	ctx->enable();

	console() << "Audio setup complete" << std::endl;
}

void GraphicsRenderer::setupAudioFromDevice(const std::string& deviceName) {
	auto ctx = audio::Context::master();

	// List all available devices
	console() << "Available audio devices:" << std::endl;
	auto devices = audio::Device::getDevices();
	audio::DeviceRef targetDevice = nullptr;

	for (const auto& dev : devices) {
		console() << "  - " << dev->getName() << " (Input: " << dev->getNumInputChannels()
		          << ", Output: " << dev->getNumOutputChannels() << ")" << std::endl;

		// Case-insensitive partial match
		std::string devNameLower = dev->getName();
		std::string targetNameLower = deviceName;
		std::transform(devNameLower.begin(), devNameLower.end(), devNameLower.begin(), ::tolower);
		std::transform(targetNameLower.begin(), targetNameLower.end(), targetNameLower.begin(), ::tolower);

		if (devNameLower.find(targetNameLower) != std::string::npos && dev->getNumInputChannels() > 0) {
			targetDevice = dev;
		}
	}

	if (!targetDevice) {
		console() << "Device '" << deviceName << "' not found or has no input channels" << std::endl;
		return;
	}

	console() << "Using audio device: " << targetDevice->getName() << std::endl;

	// Create input node with selected device
	auto format = audio::Node::Format().autoEnable();
	mAudioInput = ctx->createInputDeviceNode(targetDevice, format);

	// Create FFT node for spectral analysis
	// Use window size matching our waveform buffer for better time-domain capture
	auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(mWaveformBufferSize);
	mMonitorSpectralNode = ctx->makeNode(new audio::MonitorSpectralNode(monitorFormat));

	// Connect input directly to FFT
	// We'll capture time-domain samples from the spectral node's buffer
	mAudioInput >> mMonitorSpectralNode;

	// Enable the input
	mAudioInput->enable();
	ctx->enable();

	mAudioInputEnabled = true;
	console() << "Audio from device setup complete" << std::endl;
}

void GraphicsRenderer::enableAudioInput(bool enable) {
	mAudioInputEnabled = enable;

	if (enable && !mAudioInput) {
		setupAudioInput();
	}

	console() << "Audio input " << (enable ? "enabled" : "disabled") << std::endl;
}

void GraphicsRenderer::updateAudioFromInput() {
	if (!mAudioInputEnabled || !mMonitorSpectralNode) {
		return;
	}

	// Check if input is actually enabled and running
	static int statusCounter = 0;
	if (++statusCounter >= 120) {  // Every 2 seconds
		if (mAudioInput) {
			console() << "Audio input status: enabled=" << mAudioInput->isEnabled()
			          << " initialized=" << mAudioInput->isInitialized()
			          << " num channels=" << mAudioInput->getNumChannels() << std::endl;
		}
		statusCounter = 0;
	}

	// Get magnitude spectrum from FFT
	mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();

	if (mMagSpectrum.empty()) {
		return;
	}

	// Calculate total energy
	float totalEnergy = 0.0f;
	for (float mag : mMagSpectrum) {
		totalEnergy += mag;
	}
	totalEnergy /= mMagSpectrum.size();

	// Extract frequency bands
	// Assuming 44100 Hz sample rate and 2048 FFT size
	// Each bin = 44100 / 2048 ≈ 21.5 Hz
	int numBins = mMagSpectrum.size();

	// Low band: 0-200 Hz (bins 0-9)
	int lowEnd = std::min(10, numBins);
	float lowEnergy = 0.0f;
	for (int i = 0; i < lowEnd; i++) {
		lowEnergy += mMagSpectrum[i];
	}
	lowEnergy /= lowEnd;

	// Mid band: 200-2000 Hz (bins 10-93)
	int midStart = 10;
	int midEnd = std::min(94, numBins);
	float midEnergy = 0.0f;
	for (int i = midStart; i < midEnd; i++) {
		midEnergy += mMagSpectrum[i];
	}
	midEnergy /= (midEnd - midStart);

	// High band: 2000Hz+ (bins 94+)
	int highStart = 94;
	float highEnergy = 0.0f;
	if (highStart < numBins) {
		for (int i = highStart; i < numBins; i++) {
			highEnergy += mMagSpectrum[i];
		}
		highEnergy /= (numBins - highStart);
	}

	// Smooth the values (exponential moving average)
	float smoothing = 0.8f;
	mAudioAmplitude = mAudioAmplitude * smoothing + totalEnergy * (1.0f - smoothing);
	mAudioLowBand = mAudioLowBand * smoothing + lowEnergy * (1.0f - smoothing);
	mAudioMidBand = mAudioMidBand * smoothing + midEnergy * (1.0f - smoothing);
	mAudioHighBand = mAudioHighBand * smoothing + highEnergy * (1.0f - smoothing);

	// Debug output every 60 frames (~1 second) - show raw values
	static int debugCounter = 0;
	if (++debugCounter >= 60) {
		console() << "Audio RAW: Amp=" << mAudioAmplitude
		          << " Low=" << mAudioLowBand
		          << " Mid=" << mAudioMidBand
		          << " High=" << mAudioHighBand << std::endl;
		debugCounter = 0;
	}

	// Normalize to 0-1 range with adaptive scaling
	// Use much lower scale - FFT magnitudes are typically very small
	float scale = 100.0f;
	mAudioAmplitude = std::min(1.0f, std::max(0.0f, mAudioAmplitude * scale));
	mAudioLowBand = std::min(1.0f, std::max(0.0f, mAudioLowBand * scale));
	mAudioMidBand = std::min(1.0f, std::max(0.0f, mAudioMidBand * scale));
	mAudioHighBand = std::min(1.0f, std::max(0.0f, mAudioHighBand * scale));

	// Capture time-domain waveform from spectral node's input buffer
	// The MonitorSpectralNode stores the time-domain samples before FFT
	const audio::Buffer& timeBuffer = mMonitorSpectralNode->getBuffer();
	size_t numFrames = timeBuffer.getNumFrames();

	if (numFrames > 0) {
		const float* channelData = timeBuffer.getChannel(0);  // Get first channel
		size_t copySize = std::min(numFrames, (size_t)mWaveformBufferSize);

		// Copy the most recent samples
		for (size_t i = 0; i < copySize; i++) {
			mWaveformBuffer[i] = channelData[i];
		}

		// Debug output every 60 frames
		static int waveformDebugCounter = 0;
		if (++waveformDebugCounter >= 60) {
			float maxSample = 0.0f;
			for (size_t i = 0; i < copySize; i++) {
				float absSample = std::abs(mWaveformBuffer[i]);
				if (absSample > maxSample) maxSample = absSample;
			}
			console() << "Waveform: " << copySize << " samples, max amplitude: "
			          << maxSample << " (range: -1 to +1)" << std::endl;
			waveformDebugCounter = 0;
		}
	}
}

void GraphicsRenderer::computeMFCCs() {
	if (mMagSpectrum.empty()) {
		static bool warnedOnce = false;
		if (!warnedOnce) {
			console() << "computeMFCCs: mMagSpectrum is empty!" << std::endl;
			warnedOnce = true;
		}
		return;
	}

	// Simplified MFCC computation from FFT magnitudes
	// Real MFCCs require: FFT -> Mel filterbank -> log -> DCT
	// This is a simplified version using energy bands as proxy

	int numBins = mMagSpectrum.size();
	int numMFCC = mMFCCCoeffs.size();

	// Debug output every 60 frames
	static int mfccDebugCounter = 0;
	if (++mfccDebugCounter >= 60) {
		console() << "computeMFCCs: numBins=" << numBins << " numMFCC=" << numMFCC << std::endl;
		mfccDebugCounter = 0;
	}

	// Create mel-scale filter banks (simplified)
	std::vector<float> melEnergies(numMFCC, 0.0f);

	for (int m = 0; m < numMFCC; m++) {
		float melStart = m * numBins / (float)numMFCC;
		float melEnd = (m + 1) * numBins / (float)numMFCC;

		int startBin = (int)melStart;
		int endBin = std::min((int)melEnd, numBins);

		float energy = 0.0f;
		for (int k = startBin; k < endBin; k++) {
			energy += mMagSpectrum[k];
		}

		melEnergies[m] = log(energy + 1e-10f);  // Log energy
	}

	// Simple DCT (Discrete Cosine Transform) approximation
	for (int i = 0; i < numMFCC; i++) {
		float sum = 0.0f;
		for (int m = 0; m < numMFCC; m++) {
			sum += melEnergies[m] * cos(M_PI * i * (m + 0.5f) / numMFCC);
		}
		mMFCCCoeffs[i] = sum * 0.1f;  // Scale for display
	}
}

void GraphicsRenderer::drawWaveform() {
	if (!mShowWaveform || !mAudioInputEnabled || mWaveformBuffer.empty()) {
		return;
	}

	// Disable depth testing for 2D overlay
	gl::ScopedDepth scopedDepth(false);

	// Draw waveform in 2D overlay - full width, large height
	gl::ScopedMatrices scopedMatrices;
	gl::setMatricesWindow(getWindowSize());

	float waveWidth = getWindowWidth();  // Full screen width
	float waveHeight = 300.0f;  // Height
	float xStart = 0.0f;  // Full width from left edge
	float yStart = getWindowHeight() / 2.0f - waveHeight / 2.0f;  // Center vertically

	// Center line for reference
	gl::ScopedColor colorScope;
	gl::color(0.3f, 0.3f, 0.3f, 0.5f);
	float centerY = yStart + waveHeight / 2.0f;
	gl::drawLine(vec2(xStart, centerY), vec2(xStart + waveWidth, centerY));

	// Waveform - use configured color
	gl::color(mWaveformColor.x, mWaveformColor.y, mWaveformColor.z, 1.0f);
	glLineWidth(2.0f);  // Thicker line for visibility
	gl::begin(GL_LINE_STRIP);

	float yScale = waveHeight / 2.0f * 1.8f;  // 180% of half height for doubled amplitude

	// Draw waveform from linear buffer
	for (int i = 0; i < mWaveformBufferSize; i++) {
		float x = xStart + (i / (float)mWaveformBufferSize) * waveWidth;
		float y = centerY - mWaveformBuffer[i] * yScale;
		gl::vertex(vec2(x, y));
	}

	gl::end();
	glLineWidth(1.0f);  // Reset line width
}

void GraphicsRenderer::drawMFCC() {
	if (!mShowMFCC || !mAudioInputEnabled || mMFCCCoeffs.empty()) {
		return;
	}

	// Disable depth testing for 2D overlay
	gl::ScopedDepth scopedDepth(false);

	// Draw MFCC as horizontal bars - full width
	gl::ScopedMatrices scopedMatrices;
	gl::setMatricesWindow(getWindowSize());

	float chartWidth = getWindowWidth();
	float chartHeight = 400.0f;
	float xStart = 0.0f;
	float yStart = getWindowHeight() / 2.0f - chartHeight / 2.0f;

	gl::ScopedColor colorScope;

	int numCoeffs = mMFCCCoeffs.size();  // Should be 13
	float lineSpacing = chartHeight / (float)numCoeffs;

	// Draw 13 horizontal bars (one per MFCC coefficient)
	float centerX = chartWidth / 2.0f;

	for (int coeffIdx = 0; coeffIdx < numCoeffs; coeffIdx++) {
		float centerY = yStart + (coeffIdx + 0.5f) * lineSpacing;

		// Get raw value
		float value = std::abs(mMFCCCoeffs[coeffIdx]);

		// Width: INVERTED - higher value = shorter width
		float normalizedValue = 1.0f - std::min(value * 10.0f, 1.0f);  // Invert and scale
		float halfWidth = normalizedValue * (chartWidth / 2.0f);

		// Height: varies with value (higher value = taller rectangle)
		float heightScale = std::min(value * 20.0f, 1.0f);  // Scale for height
		float rectHeight = heightScale * lineSpacing * 0.8f;  // Max 80% of spacing

		// Alpha: varies with value (higher value = more opaque)
		float alpha = 0.3f + (heightScale * 0.7f);  // Range from 0.3 to 1.0

		// Color gradient: configurable hue range
		float hue = mMFCCHueStart + (coeffIdx / (float)numCoeffs) * mMFCCHueRange;
		gl::color(ColorAf(CM_HSV, hue, 0.9f, 1.0f, alpha));

		// Draw centered rectangle
		Rectf rect(centerX - halfWidth, centerY - rectHeight / 2.0f,
		           centerX + halfWidth, centerY + rectHeight / 2.0f);
		gl::drawSolidRect(rect);
	}
}

void GraphicsRenderer::createWaveformTexture() {
	static bool loggedOnce = false;
	if (!loggedOnce) {
		console() << "createWaveformTexture called: mWaveformFbo=" << (mWaveformFbo ? "valid" : "null") << std::endl;
		loggedOnce = true;
	}

	if (!mWaveformFbo) {
		static bool loggedNoFbo = false;
		if (!loggedNoFbo) {
			console() << "createWaveformTexture: No FBO available!" << std::endl;
			loggedNoFbo = true;
		}
		return;
	}

	// Save current state
	gl::ScopedFramebuffer fboScope(mWaveformFbo);
	gl::ScopedViewport viewportScope(ivec2(0), mWaveformFbo->getSize());
	gl::ScopedMatrices matrixScope;

	// Set up orthographic projection for FBO
	gl::setMatricesWindow(mWaveformFbo->getSize());

	// Clear FBO with transparent black
	gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));

	// Draw waveform to FBO (no margins - use full FBO size)
	float waveWidth = (float)mWaveformFbo->getWidth();
	float waveHeight = (float)mWaveformFbo->getHeight();

	// Background - semi-transparent black
	gl::ScopedColor colorScope;
	gl::color(0.0f, 0.0f, 0.0f, 0.6f);
	gl::drawSolidRect(Rectf(0, 0, waveWidth, waveHeight));

	// Center line
	gl::color(0.2f, 0.2f, 0.2f, 0.8f);
	float centerY = waveHeight / 2.0f;
	gl::drawLine(vec2(0, centerY), vec2(waveWidth, centerY));

	// Waveform - bright green
	gl::color(0.0f, 1.0f, 0.0f, 1.0f);
	glLineWidth(2.0f);
	gl::begin(GL_LINE_STRIP);

	float yScale = waveHeight / 2.0f * 0.9f;

	for (int i = 0; i < mWaveformBufferSize; i++) {
		float x = (i / (float)mWaveformBufferSize) * waveWidth;
		float y = centerY - mWaveformBuffer[i] * yScale;
		gl::vertex(vec2(x, y));
	}

	gl::end();
	glLineWidth(1.0f);

	// Border
	gl::color(0.2f, 0.8f, 0.2f, 0.8f);
	gl::drawStrokedRect(Rectf(0, 0, waveWidth, waveHeight));

	// Store texture reference for mapping
	mWaveformTexture = mWaveformFbo->getColorTexture();
}

void GraphicsRenderer::createMFCCTexture() {
	if (!mMFCCFbo) {
		return;
	}

	// Save current state
	gl::ScopedFramebuffer fboScope(mMFCCFbo);
	gl::ScopedViewport viewportScope(ivec2(0), mMFCCFbo->getSize());
	gl::ScopedMatrices matrixScope;

	// Set up orthographic projection for FBO
	gl::setMatricesWindow(mMFCCFbo->getSize());

	// Clear FBO with transparent black
	gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));

	// Draw MFCC to FBO (use full FBO size)
	float chartWidth = (float)mMFCCFbo->getWidth();
	float chartHeight = (float)mMFCCFbo->getHeight();

	// Background
	gl::ScopedColor colorScope;
	gl::color(0.0f, 0.0f, 0.0f, 0.7f);
	gl::drawSolidRect(Rectf(0, 0, chartWidth, chartHeight));

	// Border
	gl::color(0.7f, 0.3f, 0.7f, 1.0f);
	gl::drawStrokedRect(Rectf(0, 0, chartWidth, chartHeight));

	// MFCC bars
	int numCoeffs = mMFCCCoeffs.size();
	float barWidth = chartWidth / (float)numCoeffs;
	float maxVal = *std::max_element(mMFCCCoeffs.begin(), mMFCCCoeffs.end());
	maxVal = std::max(maxVal, 0.01f);

	for (int i = 0; i < numCoeffs; i++) {
		float normalized = std::abs(mMFCCCoeffs[i]) / maxVal;
		normalized = std::min(normalized, 1.0f);

		float barHeight = normalized * chartHeight * 0.9f;
		float x = i * barWidth;
		float y = chartHeight - barHeight;

		// Color gradient based on coefficient index
		float hue = i / (float)numCoeffs;
		gl::color(ColorAf(CM_HSV, hue, 0.8f, 0.9f));
		gl::drawSolidRect(Rectf(x + 1, y, x + barWidth - 1, chartHeight));
	}

	// Store texture reference for mapping
	mMFCCTexture = mMFCCFbo->getColorTexture();
}

void GraphicsRenderer::update() {
	// FPS monitoring - update every second
	float currentTime = static_cast<float>(app::getElapsedSeconds());
	mFrameCount++;

	if (currentTime - mFpsLastTime >= 1.0f) {
		mCurrentFps = mFrameCount / (currentTime - mFpsLastTime);

		// Get window size for display debugging
		ivec2 windowSize = getWindowSize();

		console() << "FPS: " << mCurrentFps
		          << " | Alive: " << ptrWorld->alive()
		          << " | Resolution: " << windowSize.x << "x" << windowSize.y
		          << " | Audio: " << mAudioAmplitude << std::endl;
		mFrameCount = 0;
		mFpsLastTime = currentTime;
	}

	// Update audio features from either real audio input or SOM BMU
	if (mAudioInputEnabled) {
		updateAudioFromInput();
		computeMFCCs();  // Compute MFCCs from FFT data
	} else {
		updateAudioFeatures();
	}

	// Update pattern animations
	float dt = currentTime - mLastTime;
	mLastTime = currentTime;

	for (auto& pattern : mPatterns) {
		if (pattern) {
			pattern->update(currentTime, dt);
		}
	}

	// Update boids with audio reactivity
	if (boids) {
		// Modulate boid parameters based on audio features
		// Cohesion reacts to low frequencies (bass)
		if (boids->audioReactivityCohesion > 0.0f) {
			boids->cohesion = boids->baseCohesion * (1.0 + mAudioLowBand * boids->audioReactivityCohesion);
		}

		// Separation reacts to high frequencies (treble/percussion)
		if (boids->audioReactivitySeparation > 0.0f) {
			boids->separation = boids->baseSeparation * (1.0 + mAudioHighBand * boids->audioReactivitySeparation);
		}

		// Alignment reacts to mid frequencies (melody)
		if (boids->audioReactivityAlignment > 0.0f) {
			boids->alignment = boids->baseAlignment * (1.0 + mAudioMidBand * boids->audioReactivityAlignment);
		}

		boids->update();
	}

	if (ptrWorld->initialized()) {

		// Use window width for all dimensions to maintain cubic proportions
		fragSizeX = (double)(getWindowWidth() / ptrWorld->sizeX()) * 0.1;
		fragSizeY = (double)(getWindowWidth() / ptrWorld->sizeY()) * 0.1;
		fragSizeZ = (double)(getWindowWidth() / ptrWorld->sizeZ()) * 0.1;

		hx = fragSizeX * ptrWorld->sizeX() * 0.5;
		hy = fragSizeY * ptrWorld->sizeY() * 0.5;
		hz = fragSizeZ * ptrWorld->sizeZ() * 0.5;
	}

    mRotation = glm::rotate(mRotation, glm::radians(rotateAngle), rotateXYZ);

	// Update camera based on boid attachment flags
	if (boids && boids->numBoids() > 0) {
		vec3 boidDimensions = boids->dimensions();
		vec3 boidOffset = boidDimensions * 0.5f;

		if (attachEyeToFirstBoid) {
			Boid* firstBoid = boids->getBoidAtIndex(0);
			vec3 boidPos = firstBoid->pos - boidOffset;

			// Offset camera behind the boid along its velocity direction
			vec3 velocity = glm::normalize(firstBoid->vec);
			float offsetDistance = 20.0f;  // Distance behind boid
			mEye = boidPos - velocity * offsetDistance + vec3(0, 5.0f, 0);  // Slightly above too
		}

		if (lookAtCentroid) {
			vec3 centroid = boids->centroid();
			mCenter = centroid - boidOffset;
		}
	}

	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );

    gl::enableAdditiveBlending();
    glEnable( GL_MULTISAMPLE_ARB );
    glHint (GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

    gl::clear( Color( _bgr, _bgg, _bgb ) );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

}

void GraphicsRenderer::clearInstanceData() {
	mCubePositions.clear();
	mCubeColors.clear();
	mCubeScales.clear();
	mCubeTextures.clear();

	mSpherePositions.clear();
	mSphereColors.clear();
	mSphereRadii.clear();
	mSpherePatternIds.clear();

	mCylinderPositions.clear();
	mCylinderColors.clear();
	mCylinderScales.clear();
	mCylinderRotations.clear();

	mLineStarts.clear();
	mLineEnds.clear();
	mLineColors.clear();
	mLineWidths.clear();

	mPlaneInstances.clear();
	mPolygonInstances.clear();
	mSphericalQuads.clear();

	mPointPositions.clear();
	mPointColors.clear();
	mPointSizes.clear();

	mTriangleVertices.clear();
	mTriangleColors.clear();
}

void GraphicsRenderer::addCubeInstance(const vec3& position, const ColorA& color, const vec3& scale, const gl::TextureRef& texture) {
	mCubePositions.push_back(position);
	mCubeColors.push_back(color);
	mCubeScales.push_back(scale);
	mCubeTextures.push_back(texture);
}

void GraphicsRenderer::addSphereInstance(const vec3& position, const ColorA& color, float radius, int patternId) {
	mSpherePositions.push_back(position);
	mSphereColors.push_back(color);
	mSphereRadii.push_back(radius);
	mSpherePatternIds.push_back(patternId);
}

void GraphicsRenderer::addCylinderInstance(const vec3& position, const ColorA& color, const vec3& scale, const quat& rotation) {
	mCylinderPositions.push_back(position);
	mCylinderColors.push_back(color);
	mCylinderScales.push_back(scale);
	mCylinderRotations.push_back(rotation);
}

void GraphicsRenderer::addLineInstance(const vec3& start, const vec3& end, const ColorA& color, float width) {
	mLineStarts.push_back(start);
	mLineEnds.push_back(end);
	mLineColors.push_back(color);
	mLineWidths.push_back(width);
}

void GraphicsRenderer::addSphericalQuad(float theta, float phi, float rho, const ivec2& gridPos, const ColorA cornerColors[4], float cellState) {
	SphericalQuadData quad;
	quad.theta = theta;
	quad.phi = phi;
	quad.rho = rho;
	quad.gridPos = gridPos;
	quad.cellState = cellState;
	for (int i = 0; i < 4; i++) {
		quad.cornerColors[i] = cornerColors[i];
	}
	mSphericalQuads.push_back(quad);
}

void GraphicsRenderer::addPlaneInstance(const vec3& position, const vec3& size, const ColorA& color, int planeType, bool wireframe) {
	PlaneInstanceData plane;
	plane.position = position;
	plane.size = size;
	plane.color = color;
	plane.planeType = planeType;
	plane.wireframe = wireframe;
	mPlaneInstances.push_back(plane);
}

void GraphicsRenderer::addPolygonInstance(const vec3 vertices[4], const ColorA colors[4]) {
	PolygonInstanceData polygon;
	for (int i = 0; i < 4; i++) {
		polygon.vertices[i] = vertices[i];
		polygon.colors[i] = colors[i];
	}
	mPolygonInstances.push_back(polygon);
}

void GraphicsRenderer::addPointInstance(const vec3& position, const ColorA& color, float size) {
	mPointPositions.push_back(position);
	mPointColors.push_back(color);
	mPointSizes.push_back(size);
}

void GraphicsRenderer::startDraw() {
    gl::ScopedModelMatrix scopedModelMatrix;
    glEnable(GL_LINE_SMOOTH);
    mGrid = gl::VertBatch::create( GL_LINES );
    mGrid->begin( GL_LINES );

	// Bind FBO if any effect is enabled
	if (mCurrentEffect != EFFECT_NONE) {
		// Create FBO if it doesn't exist yet
		if (!mFbo) {
			setupPostProcessing();
		}
		if (mFbo) {
			mFbo->bindFramebuffer();
			// Clear with current background color
			gl::clear(Color(_bgr, _bgg, _bgb));
		}
	}

	// Clear instance data for this frame
	clearInstanceData();

	// Pre-reserve capacity to avoid reallocations during frame
	// Worst case: 17^3 cells = 4913, with Pattern00/23 = ~24 instances per cell
	// Reserve conservatively for ~10k instances
	static bool reserved = false;
	if (!reserved) {
		mCubePositions.reserve(10000);
		mCubeColors.reserve(10000);
		mCubeScales.reserve(10000);
		mCubeTextures.reserve(10000);

		mSpherePositions.reserve(10000);
		mSphereColors.reserve(10000);
		mSphereRadii.reserve(10000);
		mSpherePatternIds.reserve(10000);

		mLineStarts.reserve(20000);
		mLineEnds.reserve(20000);
		mLineColors.reserve(20000);
		mLineWidths.reserve(20000);

		mPlaneInstances.reserve(10000);
		mTriangleVertices.reserve(30000);
		mTriangleColors.reserve(30000);

		reserved = true;
		console() << "Pre-allocated instance buffers for stable performance" << std::endl;
	}
}

void GraphicsRenderer::drawCubeInstances() {
	if (mCubePositions.empty() || !mCubeMesh) {
		return;
	}

	// Check if any textures are actually valid
	bool hasValidTextures = false;
	for (const auto& tex : mCubeTextures) {
		if (tex) {
			hasValidTextures = true;
			break;
		}
	}

	static int frameCount = 0;
	if (frameCount % 60 == 0) {  // Log every 60 frames
		console() << "Drawing " << mCubePositions.size() << " cubes, hasValidTextures=" << hasValidTextures << std::endl;
	}
	frameCount++;

	if (hasValidTextures) {
		static bool loggedOnce = false;
		if (!loggedOnce) {
			console() << "Has valid textures, attempting textured instanced rendering..." << std::endl;
			console() << "mInstanceTexturedShader valid: " << (mInstanceTexturedShader ? "YES" : "NO") << std::endl;
			loggedOnce = true;
		}

		if (mInstanceTexturedShader) {
			// Use instanced rendering with texture shader
			static bool loggedOnce2 = false;
			if (!loggedOnce2) {
				console() << "Drawing " << mCubePositions.size() << " textured cubes INSTANCED with fu_00.png" << std::endl;
				// Find first valid texture to check size
				for (const auto& tex : mCubeTextures) {
					if (tex) {
						console() << "Texture size: " << tex->getWidth() << "x" << tex->getHeight() << std::endl;
						break;
					}
				}
				loggedOnce2 = true;
			}

			// Clear any previous GL errors
			while (glGetError() != GL_NO_ERROR);

			// Create interleaved instance data
			struct InstanceData {
				vec3 position;
				ColorA color;
				vec3 scale;
			};

			std::vector<InstanceData> instanceData;
			instanceData.reserve(mCubePositions.size());

			for (size_t i = 0; i < mCubePositions.size(); ++i) {
				InstanceData data;
				data.position = mCubePositions[i];
				data.color = ColorA(1.0f, 1.0f, 1.0f, 1.0f);  // White for full texture brightness
				data.scale = mCubeScales[i];
				instanceData.push_back(data);
			}

			// Create or update instance VBO
			if (!mCubeInstanceVbo || mCubeInstanceVbo->getSize() < instanceData.size() * sizeof(InstanceData)) {
				mCubeInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceData, GL_DYNAMIC_DRAW);
			} else {
				mCubeInstanceVbo->bufferData(instanceData.size() * sizeof(InstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
			}

			// Set up instance attributes
			geom::BufferLayout instanceLayout;
			instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(InstanceData), offsetof(InstanceData, position), 1);
			instanceLayout.append(geom::Attrib::CUSTOM_1, 4, sizeof(InstanceData), offsetof(InstanceData, color), 1);
			instanceLayout.append(geom::Attrib::CUSTOM_2, 3, sizeof(InstanceData), offsetof(InstanceData, scale), 1);

			// Get the VBO mesh's VBOs and layouts
			std::vector<std::pair<geom::BufferLayout, gl::VboRef>> vertexArrayBuffers = mCubeMesh->getVertexArrayLayoutVbos();

			static bool loggedOnce3 = false;
			if (!loggedOnce3) {
				console() << "Original cube mesh has " << vertexArrayBuffers.size() << " VBO buffers" << std::endl;
				loggedOnce3 = true;
			}

			// Append instance data VBO
			vertexArrayBuffers.push_back(std::make_pair(instanceLayout, mCubeInstanceVbo));

			// Create new mesh with instance data
			auto instancedMesh = gl::VboMesh::create(
				mCubeMesh->getNumVertices(),
				mCubeMesh->getGlPrimitive(),
				vertexArrayBuffers,
				mCubeMesh->getNumIndices(),
				mCubeMesh->getIndexDataType(),
				mCubeMesh->getIndexVbo()
			);

			// Bind texture and draw instanced
			if (!mCubeTextures.empty() && mCubeTextures[0]) {
				try {
					// Find first valid texture
					gl::TextureRef validTex;
					for (const auto& tex : mCubeTextures) {
						if (tex) {
							validTex = tex;
							break;
						}
					}

					if (!validTex) {
						console() << "ERROR: No valid texture found!" << std::endl;
						return;
					}

					// Build attribute mapping - only custom attributes, let standard ones auto-bind
					gl::VboMesh::AttribGlslMap attributeMapping = {
						{ geom::Attrib::CUSTOM_0, "ciCustom0" },
						{ geom::Attrib::CUSTOM_1, "ciCustom1" },
						{ geom::Attrib::CUSTOM_2, "ciCustom2" }
					};

					auto batch = gl::Batch::create(instancedMesh, mInstanceTexturedShader, attributeMapping);

					static bool loggedTexInfo = false;
					if (!loggedTexInfo) {
						console() << "Texture bound to unit 0, ID=" << validTex->getId()
						          << " format=" << validTex->getInternalFormat() << std::endl;
						console() << "Setting uniform uTexture=0 via batch shader" << std::endl;
						loggedTexInfo = true;
					}

					// Set the texture uniform on the batch's shader BEFORE drawing
					batch->getGlslProg()->uniform("uTexture", 0);

					// Check pre-draw
					GLenum preErr = glGetError();
					if (preErr != GL_NO_ERROR) {
						console() << "OpenGL error BEFORE draw: 0x" << std::hex << preErr << std::dec << std::endl;
					}

					// Bind texture and draw
					gl::ScopedTextureBind texBind(validTex, 0);
					batch->drawInstanced(static_cast<GLsizei>(mCubePositions.size()));

					// Check for GL errors after draw
					GLenum err = glGetError();
					if (err != GL_NO_ERROR) {
						static bool reportedError = false;
						if (!reportedError) {
							console() << "OpenGL error after textured cube draw: 0x" << std::hex << err << std::dec << std::endl;
							reportedError = true;
						}
					} else {
						static bool reportedSuccess = false;
						if (!reportedSuccess) {
							console() << "Textured cubes drawn successfully with no GL errors!" << std::endl;
							reportedSuccess = true;
						}
					}
				}
				catch (const std::exception& e) {
					console() << "Exception during textured cube rendering: " << e.what() << std::endl;
				}
			}
		}
	} else {
		// Use instanced rendering for non-textured cubes
		if (!mInstanceShader) {
			console() << "ERROR: Instance shader not loaded!" << std::endl;
			return;
		}

		// Create interleaved instance data
		struct InstanceData {
			vec3 position;
			ColorA color;
			vec3 scale;
		};

		std::vector<InstanceData> instanceData;
		instanceData.reserve(mCubePositions.size());

		for (size_t i = 0; i < mCubePositions.size(); ++i) {
			InstanceData data;
			data.position = mCubePositions[i];
			data.color = mCubeColors[i];
			data.scale = mCubeScales[i];
			instanceData.push_back(data);
		}

		// Create or update instance VBO
		if (!mCubeInstanceVbo || mCubeInstanceVbo->getSize() < instanceData.size() * sizeof(InstanceData)) {
			mCubeInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceData, GL_DYNAMIC_DRAW);
		} else {
			mCubeInstanceVbo->bufferData(instanceData.size() * sizeof(InstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
		}

		// Set up instance attributes
		geom::BufferLayout instanceLayout;
		instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(InstanceData), offsetof(InstanceData, position), 1);
		instanceLayout.append(geom::Attrib::CUSTOM_1, 4, sizeof(InstanceData), offsetof(InstanceData, color), 1);
		instanceLayout.append(geom::Attrib::CUSTOM_2, 3, sizeof(InstanceData), offsetof(InstanceData, scale), 1);

		// Get the VBO mesh's VBOs and layouts
		std::vector<std::pair<geom::BufferLayout, gl::VboRef>> vertexArrayBuffers = mCubeMesh->getVertexArrayLayoutVbos();

		// Append instance data VBO
		vertexArrayBuffers.push_back(std::make_pair(instanceLayout, mCubeInstanceVbo));

		// Create new mesh with instance data
		auto instancedMesh = gl::VboMesh::create(
			mCubeMesh->getNumVertices(),
			mCubeMesh->getGlPrimitive(),
			vertexArrayBuffers,
			mCubeMesh->getNumIndices(),
			mCubeMesh->getIndexDataType(),
			mCubeMesh->getIndexVbo()
		);

		// Build attribute mapping for custom attributes
		gl::VboMesh::AttribGlslMap attributeMapping = {
			{ geom::Attrib::CUSTOM_0, "ciCustom0" },
			{ geom::Attrib::CUSTOM_1, "ciCustom1" },
			{ geom::Attrib::CUSTOM_2, "ciCustom2" }
		};

		// Create batch and draw with instancing
		auto batch = gl::Batch::create(instancedMesh, mInstanceShader, attributeMapping);
		batch->drawInstanced(static_cast<GLsizei>(mCubePositions.size()));
	}
}

void GraphicsRenderer::drawSphereInstances() {
	static bool loggedSphereDrawing = false;
	if (!loggedSphereDrawing && !mSpherePositions.empty()) {
		console() << "drawSphereInstances: " << mSpherePositions.size() << " spheres, mSphereMesh=" << (mSphereMesh ? "yes" : "no") << std::endl;
		loggedSphereDrawing = true;
	}

	if (mSpherePositions.empty() || !mSphereMesh) {
		return;
	}

	// Check if any spheres are from Pattern05 or Pattern13 (environment mapped)
	bool hasPattern05 = false;
	bool hasPattern13 = false;
	for (int id : mSpherePatternIds) {
		if (id == 5) hasPattern05 = true;
		if (id == 13) hasPattern13 = true;
	}

	// Determine which cubemap to use
	bool useEnvMap = (hasPattern05 || hasPattern13) && mEnvMapShader;
	gl::TextureCubeMapRef activeCubeMap;
	if (hasPattern13 && mCubeMap2) {
		activeCubeMap = mCubeMap2;  // Pattern13 uses fxp_* cubemap
	} else if (hasPattern05 && mCubeMap) {
		activeCubeMap = mCubeMap;   // Pattern05 uses fxic_* cubemap
	}

	useEnvMap = useEnvMap && activeCubeMap;
	auto shader = useEnvMap ? mEnvMapShader : mInstanceShader;

	if (!shader) {
		return;
	}

	// Create interleaved instance data
	struct InstanceData {
		vec3 position;
		ColorA color;
		vec3 scale;
	};

	std::vector<InstanceData> instanceData;
	instanceData.reserve(mSpherePositions.size());

	for (size_t i = 0; i < mSpherePositions.size(); ++i) {
		InstanceData data;
		data.position = mSpherePositions[i];
		data.color = mSphereColors[i];
		// Use radius for uniform scale
		data.scale = vec3(mSphereRadii[i]);
		instanceData.push_back(data);
	}

	// Create or update instance VBO
	if (!mSphereInstanceVbo || mSphereInstanceVbo->getSize() < instanceData.size() * sizeof(InstanceData)) {
		mSphereInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceData, GL_DYNAMIC_DRAW);
	} else {
		mSphereInstanceVbo->bufferData(instanceData.size() * sizeof(InstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
	}

	// Set up instance attributes
	geom::BufferLayout instanceLayout;
	instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(InstanceData), offsetof(InstanceData, position), 1);  // iPosition
	instanceLayout.append(geom::Attrib::CUSTOM_1, 4, sizeof(InstanceData), offsetof(InstanceData, color), 1);     // iColor
	instanceLayout.append(geom::Attrib::CUSTOM_2, 3, sizeof(InstanceData), offsetof(InstanceData, scale), 1);     // iScale

	// Get the VBO mesh's VBOs and layouts
	std::vector<std::pair<geom::BufferLayout, gl::VboRef>> vertexArrayBuffers = mSphereMesh->getVertexArrayLayoutVbos();

	// Append instance data VBO
	vertexArrayBuffers.push_back(std::make_pair(instanceLayout, mSphereInstanceVbo));

	// Create new mesh with instance data
	auto instancedMesh = gl::VboMesh::create(
		mSphereMesh->getNumVertices(),
		mSphereMesh->getGlPrimitive(),
		vertexArrayBuffers,
		mSphereMesh->getNumIndices(),
		mSphereMesh->getIndexDataType(),
		mSphereMesh->getIndexVbo()
	);

	// Build attribute mapping for custom attributes
	gl::VboMesh::AttribGlslMap attributeMapping = {
		{ geom::Attrib::CUSTOM_0, "ciCustom0" },
		{ geom::Attrib::CUSTOM_1, "ciCustom1" },
		{ geom::Attrib::CUSTOM_2, "ciCustom2" }
	};

	// Create batch and draw with instancing
	auto batch = gl::Batch::create(instancedMesh, shader, attributeMapping);

	if (useEnvMap) {
		// Set environment map uniforms
		batch->getGlslProg()->uniform("uLightPos", vec3(0.0f, 100.0f, -100.0f));

		// Use different lighting for Pattern13 vs Pattern05
		if (hasPattern13) {
			// Pattern13: Brighter base color for fxp_* cubemap
			batch->getGlslProg()->uniform("uBaseColor", vec4(0.8f, 0.8f, 0.8f, 1.0f));
			batch->getGlslProg()->uniform("uMixRatio", 0.7f);  // More reflection
		} else {
			// Pattern05: Original darker settings for fxic_* cubemap
			batch->getGlslProg()->uniform("uBaseColor", vec4(0.3f, 0.3f, 0.3f, 0.7f));
			batch->getGlslProg()->uniform("uMixRatio", 0.5f);
		}

		batch->getGlslProg()->uniform("uEnvMap", 0);

		// Bind active cubemap and draw
		gl::ScopedTextureBind texBind(activeCubeMap, 0);
		batch->drawInstanced(static_cast<GLsizei>(mSpherePositions.size()));
	} else {
		static bool loggedBatchDraw = false;
		if (!loggedBatchDraw) {
			console() << "Drawing " << mSpherePositions.size() << " sphere instances with standard shader" << std::endl;
			loggedBatchDraw = true;
		}
		batch->drawInstanced(static_cast<GLsizei>(mSpherePositions.size()));
	}
}

void GraphicsRenderer::drawCylinderInstances() {
	if (mCylinderPositions.empty() || !mCylinderMesh) {
		return;
	}

	// Use instanced rendering with instance shader
	// The shader will handle position, scale, and color
	// For rotation, we'll use a simplified approach: convert quat to 3x3 matrix

	if (!mInstanceShader) {
		return;
	}

	// Create interleaved instance data with rotation matrix
	struct CylinderInstanceData {
		vec3 position;
		ColorA color;
		vec3 scale;
		vec4 rotation;  // Quaternion (x, y, z, w)
	};

	std::vector<CylinderInstanceData> instanceData;
	instanceData.reserve(mCylinderPositions.size());

	for (size_t i = 0; i < mCylinderPositions.size(); ++i) {
		CylinderInstanceData data;
		data.position = mCylinderPositions[i];
		data.color = mCylinderColors[i];
		data.scale = mCylinderScales[i];
		// Store quaternion as vec4
		data.rotation = vec4(mCylinderRotations[i].x, mCylinderRotations[i].y,
		                     mCylinderRotations[i].z, mCylinderRotations[i].w);
		instanceData.push_back(data);
	}

	// Create or update instance VBO
	if (!mCylinderInstanceVbo || mCylinderInstanceVbo->getSize() < instanceData.size() * sizeof(CylinderInstanceData)) {
		mCylinderInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceData, GL_DYNAMIC_DRAW);
	} else {
		mCylinderInstanceVbo->bufferData(instanceData.size() * sizeof(CylinderInstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
	}

	// Set up instance attributes
	geom::BufferLayout instanceLayout;
	instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(CylinderInstanceData), offsetof(CylinderInstanceData, position), 1);
	instanceLayout.append(geom::Attrib::CUSTOM_1, 4, sizeof(CylinderInstanceData), offsetof(CylinderInstanceData, color), 1);
	instanceLayout.append(geom::Attrib::CUSTOM_2, 3, sizeof(CylinderInstanceData), offsetof(CylinderInstanceData, scale), 1);
	instanceLayout.append(geom::Attrib::CUSTOM_3, 4, sizeof(CylinderInstanceData), offsetof(CylinderInstanceData, rotation), 1);

	// Get the VBO mesh's VBOs and layouts
	std::vector<std::pair<geom::BufferLayout, gl::VboRef>> vertexArrayBuffers = mCylinderMesh->getVertexArrayLayoutVbos();

	// Append instance data VBO
	vertexArrayBuffers.push_back(std::make_pair(instanceLayout, mCylinderInstanceVbo));

	// Create new mesh with instance data
	auto instancedMesh = gl::VboMesh::create(
		mCylinderMesh->getNumVertices(),
		mCylinderMesh->getGlPrimitive(),
		vertexArrayBuffers,
		mCylinderMesh->getNumIndices(),
		mCylinderMesh->getIndexDataType(),
		mCylinderMesh->getIndexVbo()
	);

	// Build attribute mapping for custom attributes
	gl::VboMesh::AttribGlslMap attributeMapping = {
		{ geom::Attrib::CUSTOM_0, "ciCustom0" },
		{ geom::Attrib::CUSTOM_1, "ciCustom1" },
		{ geom::Attrib::CUSTOM_2, "ciCustom2" },
		{ geom::Attrib::CUSTOM_3, "ciCustom3" }
	};

	// Create batch and draw with instancing
	// Note: The instance shader doesn't support quaternion rotation yet,
	// so rotations may not work correctly. This is a performance trade-off.
	auto batch = gl::Batch::create(instancedMesh, mInstanceShader, attributeMapping);
	batch->drawInstanced(static_cast<GLsizei>(mCylinderPositions.size()));
}

void GraphicsRenderer::drawLineInstances() {
	if (mLineStarts.empty()) {
		return;
	}

	if (!mLineShader) {
		console() << "ERROR: Line shader not loaded!" << std::endl;
		return;
	}

	if (!mLineMesh) {
		console() << "ERROR: Line mesh not created!" << std::endl;
		return;
	}


	// Create interleaved instance data
	struct LineInstanceData {
		vec3 start;
		vec3 end;
		ColorA color;
		float width;
	};

	std::vector<LineInstanceData> instanceData;
	instanceData.reserve(mLineStarts.size());

	for (size_t i = 0; i < mLineStarts.size(); ++i) {
		LineInstanceData data;
		data.start = mLineStarts[i];
		data.end = mLineEnds[i];
		data.color = mLineColors[i];
		data.width = mLineWidths[i];
		instanceData.push_back(data);
	}

	// Create or update instance VBO
	if (!mLineInstanceVbo || mLineInstanceVbo->getSize() < instanceData.size() * sizeof(LineInstanceData)) {
		mLineInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceData, GL_DYNAMIC_DRAW);
	} else {
		mLineInstanceVbo->bufferData(instanceData.size() * sizeof(LineInstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
	}

	// Set up instance attributes
	geom::BufferLayout instanceLayout;
	instanceLayout.append(geom::Attrib::CUSTOM_0, 3, sizeof(LineInstanceData), offsetof(LineInstanceData, start), 1);  // iStart
	instanceLayout.append(geom::Attrib::CUSTOM_1, 3, sizeof(LineInstanceData), offsetof(LineInstanceData, end), 1);    // iEnd
	instanceLayout.append(geom::Attrib::CUSTOM_2, 4, sizeof(LineInstanceData), offsetof(LineInstanceData, color), 1);  // iColor
	instanceLayout.append(geom::Attrib::CUSTOM_3, 1, sizeof(LineInstanceData), offsetof(LineInstanceData, width), 1);  // iWidth

	// Get the VBO mesh's VBOs and layouts
	std::vector<std::pair<geom::BufferLayout, gl::VboRef>> vertexArrayBuffers = mLineMesh->getVertexArrayLayoutVbos();

	// Append instance data VBO
	vertexArrayBuffers.push_back(std::make_pair(instanceLayout, mLineInstanceVbo));

	// Create new mesh with instance data
	auto instancedMesh = gl::VboMesh::create(
		mLineMesh->getNumVertices(),
		mLineMesh->getGlPrimitive(),
		vertexArrayBuffers,
		mLineMesh->getNumIndices(),
		mLineMesh->getIndexDataType(),
		mLineMesh->getIndexVbo()
	);

	// Create batch and draw with instancing
	try {
		// Build attribute mapping for custom attributes
		gl::VboMesh::AttribGlslMap attributeMapping = {
			{ geom::Attrib::CUSTOM_0, "ciCustom0" },
			{ geom::Attrib::CUSTOM_1, "ciCustom1" },
			{ geom::Attrib::CUSTOM_2, "ciCustom2" },
			{ geom::Attrib::CUSTOM_3, "ciCustom3" }
		};

		// Clear any previous errors
		while (glGetError() != GL_NO_ERROR);

		glEnable(GL_LINE_SMOOTH);

		auto batch = gl::Batch::create(instancedMesh, mLineShader, attributeMapping);
		batch->drawInstanced(static_cast<GLsizei>(mLineStarts.size()));

		glDisable(GL_LINE_SMOOTH);

		// Check for GL errors
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			static int errorCount = 0;
			if (errorCount < 5) {  // Only print first 5 errors to avoid spam
				console() << "OpenGL error after drawLineInstances: 0x" << std::hex << err << std::dec << std::endl;
				errorCount++;
			}
		}
	}
	catch (const std::exception& e) {
		console() << "Exception in drawLineInstances: " << e.what() << std::endl;
	}
}

void GraphicsRenderer::drawSphericalQuads() {
	if (mSphericalQuads.empty()) return;

	// Use custom shader if available, fall back to Lambert
	auto shader = mSphericalQuadShader ? mSphericalQuadShader : gl::getStockShader(gl::ShaderDef().color().lambert());
	gl::ScopedGlslProg scopedShader(shader);

	// Build a single mesh containing all spherical quads
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<ColorA> colors;

	positions.reserve(mSphericalQuads.size() * 4);
	normals.reserve(mSphericalQuads.size() * 4);
	colors.reserve(mSphericalQuads.size() * 4);

	for (const auto& quad : mSphericalQuads) {
		float theta = quad.theta;
		float phi = quad.phi;
		float rho = quad.rho;
		float cellState = quad.cellState;

		// Scale quad size based on cell state
		// Higher state = larger quads, lower state = smaller quads
		float stateScale = 0.3f + cellState * 0.7f;  // Range from 0.3 to 1.0

		// Calculate angular spans - vary based on cell state
		float thetaStep = (2.0f * M_PI / ptrWorld->sizeX()) * stateScale;
		float phiStep = (2.0f * M_PI / ptrWorld->sizeY()) * stateScale;

		// Center the quad around the cell's angular position
		float thetaOffset = thetaStep * 0.5f;
		float phiOffset = phiStep * 0.5f;

		// Calculate the 4 corners of the quad in spherical coordinates
		vec3 corners[4];

		// Corner 0: (x+offset, y+offset)
		corners[0].x = rho * cos(theta + thetaOffset) * cos(phi + phiOffset);
		corners[0].y = rho * sin(theta + thetaOffset) * cos(phi + phiOffset);
		corners[0].z = rho * sin(phi + phiOffset);

		// Corner 1: (x-offset, y+offset)
		corners[1].x = rho * cos(theta - thetaOffset) * cos(phi + phiOffset);
		corners[1].y = rho * sin(theta - thetaOffset) * cos(phi + phiOffset);
		corners[1].z = rho * sin(phi + phiOffset);

		// Corner 2: (x-offset, y-offset)
		corners[2].x = rho * cos(theta - thetaOffset) * cos(phi - phiOffset);
		corners[2].y = rho * sin(theta - thetaOffset) * cos(phi - phiOffset);
		corners[2].z = rho * sin(phi - phiOffset);

		// Corner 3: (x+offset, y-offset)
		corners[3].x = rho * cos(theta + thetaOffset) * cos(phi - phiOffset);
		corners[3].y = rho * sin(theta + thetaOffset) * cos(phi - phiOffset);
		corners[3].z = rho * sin(phi - phiOffset);

		// Add quad as two triangles (0-1-2 and 0-2-3)
		// Triangle 1: corners 0, 1, 2
		for (int i : {0, 1, 2}) {
			positions.push_back(corners[i]);
			normals.push_back(glm::normalize(corners[i]));  // Normal points outward from sphere center
			colors.push_back(quad.cornerColors[i]);
		}

		// Triangle 2: corners 0, 2, 3
		for (int i : {0, 2, 3}) {
			positions.push_back(corners[i]);
			normals.push_back(glm::normalize(corners[i]));
			colors.push_back(quad.cornerColors[i]);
		}
	}

	// Create VboMesh from the collected vertices
	if (!positions.empty()) {
		auto vboMesh = gl::VboMesh::create(positions.size(), GL_TRIANGLES, {
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::NORMAL, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, colors.size() * sizeof(ColorA), colors.data(), GL_STATIC_DRAW) }
		});

		// Set shader uniforms if using custom shader
		if (mSphericalQuadShader) {
			shader->uniform("uTime", static_cast<float>(app::getElapsedSeconds()));
			shader->uniform("uLightPos", mLightLoc);
		}

		gl::draw(vboMesh);
	}
}

void GraphicsRenderer::drawPlaneInstances() {
	if (mPlaneInstances.empty()) return;

	// Separate filled planes from wireframe planes
	std::vector<vec3> filledPositions;
	std::vector<vec3> filledNormals;
	std::vector<ColorA> filledColors;
	std::vector<vec3> wirePositions;
	std::vector<ColorA> wireColors;

	// Collect vertices for all planes
	for (const auto& plane : mPlaneInstances) {
		vec3 v0, v1, v2, v3;
		vec3 normal;

		// Calculate corners and normal based on plane type
		switch (plane.planeType) {
			case 0: // XY plane (Z fixed) - normal points toward camera (-Z)
				v0 = plane.position;
				v1 = plane.position + vec3(plane.size.x, 0, 0);
				v2 = plane.position + vec3(plane.size.x, plane.size.y, 0);
				v3 = plane.position + vec3(0, plane.size.y, 0);
				normal = vec3(0, 0, -1);
				break;
			case 1: // YZ plane (X fixed) - normal points toward camera (-X)
				v0 = plane.position;
				v1 = plane.position + vec3(0, plane.size.y, 0);
				v2 = plane.position + vec3(0, plane.size.y, plane.size.z);
				v3 = plane.position + vec3(0, 0, plane.size.z);
				normal = vec3(-1, 0, 0);
				break;
			case 2: // XZ plane (Y fixed) - normal points toward camera (-Y)
				v0 = plane.position;
				v1 = plane.position + vec3(plane.size.x, 0, 0);
				v2 = plane.position + vec3(plane.size.x, 0, plane.size.z);
				v3 = plane.position + vec3(0, 0, plane.size.z);
				normal = vec3(0, -1, 0);
				break;
		}

		if (plane.wireframe) {
			// Add as line segments (4 edges)
			wirePositions.push_back(v0); wirePositions.push_back(v1);
			wireColors.push_back(plane.color); wireColors.push_back(plane.color);

			wirePositions.push_back(v1); wirePositions.push_back(v2);
			wireColors.push_back(plane.color); wireColors.push_back(plane.color);

			wirePositions.push_back(v2); wirePositions.push_back(v3);
			wireColors.push_back(plane.color); wireColors.push_back(plane.color);

			wirePositions.push_back(v3); wirePositions.push_back(v0);
			wireColors.push_back(plane.color); wireColors.push_back(plane.color);
		} else {
			// Add as two triangles (v0-v1-v2 and v0-v2-v3) with normals
			// Triangle 1
			filledPositions.push_back(v0);
			filledPositions.push_back(v1);
			filledPositions.push_back(v2);
			filledNormals.push_back(normal);
			filledNormals.push_back(normal);
			filledNormals.push_back(normal);
			filledColors.push_back(plane.color);
			filledColors.push_back(plane.color);
			filledColors.push_back(plane.color);

			// Triangle 2
			filledPositions.push_back(v0);
			filledPositions.push_back(v2);
			filledPositions.push_back(v3);
			filledNormals.push_back(normal);
			filledNormals.push_back(normal);
			filledNormals.push_back(normal);
			filledColors.push_back(plane.color);
			filledColors.push_back(plane.color);
			filledColors.push_back(plane.color);
		}
	}

	// Draw filled planes with additive blending (like original pattern05)
	if (!filledPositions.empty()) {
		// Boost colors significantly for visibility
		for (auto& color : filledColors) {
			color.r = std::min(color.r * 4.0f, 1.0f);
			color.g = std::min(color.g * 4.0f, 1.0f);
			color.b = std::min(color.b * 4.0f, 1.0f);
		}

		auto filledMesh = gl::VboMesh::create(filledPositions.size(), GL_TRIANGLES, {
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, filledPositions.size() * sizeof(vec3), filledPositions.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, filledColors.size() * sizeof(ColorA), filledColors.data(), GL_STATIC_DRAW) }
		});

		// Disable backface culling so planes are visible from both sides
		gl::disable(GL_CULL_FACE);

		// Use additive blending like the original pattern05 for brightness
		gl::ScopedBlendAdditive scopedBlend;

		// Use flat color shader (no lighting) - original pattern05 had no lighting
		auto shader = gl::getStockShader(gl::ShaderDef().color());
		gl::ScopedGlslProg scopedShader(shader);
		gl::draw(filledMesh);

		gl::enable(GL_CULL_FACE);
	}

	// Draw wireframe planes with additive blending
	if (!wirePositions.empty()) {
		// Boost wireframe colors significantly for visibility
		for (auto& color : wireColors) {
			color.r = std::min(color.r * 4.0f, 1.0f);
			color.g = std::min(color.g * 4.0f, 1.0f);
			color.b = std::min(color.b * 4.0f, 1.0f);
		}

		auto wireMesh = gl::VboMesh::create(wirePositions.size(), GL_LINES, {
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, wirePositions.size() * sizeof(vec3), wirePositions.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, wireColors.size() * sizeof(ColorA), wireColors.data(), GL_STATIC_DRAW) }
		});

		// Use additive blending like the original pattern05
		gl::ScopedBlendAdditive scopedBlend;

		auto shader = gl::getStockShader(gl::ShaderDef().color());
		gl::ScopedGlslProg scopedShader(shader);
		gl::enable(GL_LINE_SMOOTH);
		glLineWidth(2.0f);  // Make lines thicker for visibility
		gl::draw(wireMesh);
		glLineWidth(1.0f);  // Reset to default
		gl::disable(GL_LINE_SMOOTH);
	}
}

void GraphicsRenderer::drawPolygonInstances() {
	if (mPolygonInstances.empty()) return;

	// Build mesh with all polygons
	std::vector<vec3> positions;
	std::vector<ColorA> colors;

	positions.reserve(mPolygonInstances.size() * 6);  // 2 triangles per polygon = 6 vertices
	colors.reserve(mPolygonInstances.size() * 6);

	for (const auto& polygon : mPolygonInstances) {
		// Split quad into 2 triangles: (0,1,2) and (0,2,3)
		// Triangle 1
		positions.push_back(polygon.vertices[0]);
		positions.push_back(polygon.vertices[1]);
		positions.push_back(polygon.vertices[2]);
		colors.push_back(polygon.colors[0]);
		colors.push_back(polygon.colors[1]);
		colors.push_back(polygon.colors[2]);

		// Triangle 2
		positions.push_back(polygon.vertices[0]);
		positions.push_back(polygon.vertices[2]);
		positions.push_back(polygon.vertices[3]);
		colors.push_back(polygon.colors[0]);
		colors.push_back(polygon.colors[2]);
		colors.push_back(polygon.colors[3]);
	}

	if (!positions.empty()) {
		auto polygonMesh = gl::VboMesh::create(positions.size(), GL_TRIANGLES, {
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, colors.size() * sizeof(ColorA), colors.data(), GL_STATIC_DRAW) }
		});

		// Use additive blending and polygon smoothing like the original
		gl::ScopedBlendAdditive scopedBlend;
		glEnable(GL_POLYGON_SMOOTH);

		auto shader = gl::getStockShader(gl::ShaderDef().color());
		gl::ScopedGlslProg scopedShader(shader);
		gl::draw(polygonMesh);

		glDisable(GL_POLYGON_SMOOTH);
	}
}

void GraphicsRenderer::drawPointInstances() {
	if (mPointPositions.empty()) return;

	// Build mesh with all points
	std::vector<vec3> positions;
	std::vector<ColorA> colors;

	positions.reserve(mPointPositions.size());
	colors.reserve(mPointPositions.size());

	for (size_t i = 0; i < mPointPositions.size(); i++) {
		positions.push_back(mPointPositions[i]);
		colors.push_back(mPointColors[i]);
	}

	if (!positions.empty()) {
		auto pointMesh = gl::VboMesh::create(positions.size(), GL_POINTS, {
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW) },
			{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
			  gl::Vbo::create(GL_ARRAY_BUFFER, colors.size() * sizeof(ColorA), colors.data(), GL_STATIC_DRAW) }
		});

		// Use additive blending for glow effect
		gl::ScopedBlendAdditive scopedBlend;

		// Set point size
		glPointSize(3.0f);  // Base point size

		auto shader = gl::getStockShader(gl::ShaderDef().color());
		gl::ScopedGlslProg scopedShader(shader);
		gl::draw(pointMesh);

		glPointSize(1.0f);  // Reset to default
	}
}

void GraphicsRenderer::addTriangleInstance(const vec3& v0, const vec3& v1, const vec3& v2,
                                           const ColorA& c0, const ColorA& c1, const ColorA& c2) {
	mTriangleVertices.push_back(v0);
	mTriangleVertices.push_back(v1);
	mTriangleVertices.push_back(v2);
	mTriangleColors.push_back(c0);
	mTriangleColors.push_back(c1);
	mTriangleColors.push_back(c2);
}

void GraphicsRenderer::drawTriangleInstances() {
	if (mTriangleVertices.empty()) return;

	// Build VBO mesh with all triangles
	auto triangleMesh = gl::VboMesh::create(mTriangleVertices.size(), GL_TRIANGLES, {
		{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::POSITION, 3, 0, 0)}),
		  gl::Vbo::create(GL_ARRAY_BUFFER, mTriangleVertices.size() * sizeof(vec3), mTriangleVertices.data(), GL_STATIC_DRAW) },
		{ geom::BufferLayout({geom::AttribInfo(geom::Attrib::COLOR, 4, 0, 0)}),
		  gl::Vbo::create(GL_ARRAY_BUFFER, mTriangleColors.size() * sizeof(ColorA), mTriangleColors.data(), GL_STATIC_DRAW) }
	});

	gl::enableAlphaBlending();
	auto shader = gl::getStockShader(gl::ShaderDef().color());
	gl::ScopedGlslProg scopedShader(shader);
	gl::draw(triangleMesh);
}

void GraphicsRenderer::endDraw() {
	// Add boid instances to buffers before drawing
	drawBoids();

	// Draw all collected instances
	drawCubeInstances();
	drawSphereInstances();
	drawCylinderInstances();
	drawLineInstances();
	drawSphericalQuads();
	drawPlaneInstances();
	drawPolygonInstances();
	drawPointInstances();
	drawTriangleInstances();

	// Draw legacy lines (will be deprecated)
    mGrid->end();
    mGrid->draw();

	// Draw code panel if active
	if (codePanelActive) {
		if (codePanelMapped)
			mapCodePanel();
		else
			drawCodePanel();
	}

	// Draw audio visualizations (2D overlays)
	drawWaveform();
	drawMFCC();

	// Apply post-processing effects if enabled
	if (mCurrentEffect != EFFECT_NONE && mFbo) {
		applyEffect();
	}

	counter++;
}

void GraphicsRenderer::drawCodePanel() {
	gl::pushMatrices();
	gl::setMatricesWindow(getWindowSize());

	codePanel.update(vec2(getWindowWidth(), getWindowHeight()));

	gl::popMatrices();
}

void GraphicsRenderer::mapCodePanel() {
	// Update texture and opacity counter
	codePanel.bind();

	if (!codePanel.texture) {
		codePanel.unbind();
		return;
	}

	gl::enableAlphaBlending();
	gl::color(1.0f, 1.0f, 1.0f, codePanel.opacity);

	float size = hx * 2;
	Rectf rect = Rectf(-size/2, -size/2, size/2, size/2);

	// Draw 6 faces of cube with texture mapped
	gl::pushMatrices();

	// Front face (Z+)
	gl::pushMatrices();
	gl::translate(0.0f, 0.0f, size/2);
	gl::draw(codePanel.texture, rect);
	gl::popMatrices();

	// Back face (Z-)
	gl::pushMatrices();
	gl::translate(0.0f, 0.0f, -size/2);
	gl::rotate(glm::radians(180.0f), 0.0f, 1.0f, 0.0f);
	gl::draw(codePanel.texture, rect);
	gl::popMatrices();

	// Right face (X+)
	gl::pushMatrices();
	gl::translate(size/2, 0.0f, 0.0f);
	gl::rotate(glm::radians(90.0f), 0.0f, 1.0f, 0.0f);
	gl::draw(codePanel.texture, rect);
	gl::popMatrices();

	// Left face (X-)
	gl::pushMatrices();
	gl::translate(-size/2, 0.0f, 0.0f);
	gl::rotate(glm::radians(-90.0f), 0.0f, 1.0f, 0.0f);
	gl::draw(codePanel.texture, rect);
	gl::popMatrices();

	// Top face (Y+)
	gl::pushMatrices();
	gl::translate(0.0f, size/2, 0.0f);
	gl::rotate(glm::radians(-90.0f), 1.0f, 0.0f, 0.0f);
	gl::draw(codePanel.texture, rect);
	gl::popMatrices();

	// Bottom face (Y-)
	gl::pushMatrices();
	gl::translate(0.0f, -size/2, 0.0f);
	gl::rotate(glm::radians(90.0f), 1.0f, 0.0f, 0.0f);
	gl::draw(codePanel.texture, rect);
	gl::popMatrices();

	gl::popMatrices();

	codePanel.unbind();
}

void GraphicsRenderer::drawBoids() {
	// Early exit if no boids
	if (!boids || boids->numBoids() == 0) return;

	// Debug: log boid info once
	static bool logged = false;
	if (!logged) {
		console() << "drawBoids: " << boids->numBoids() << " boids, dimensions: "
		          << boids->dimensions().x << "," << boids->dimensions().y << "," << boids->dimensions().z << std::endl;
		if (boids->numBoids() > 0) {
			Boid* b = boids->getBoidAtIndex(0);
			console() << "First boid pos: " << b->pos.x << "," << b->pos.y << "," << b->pos.z << std::endl;
		}
		console() << "Boid patterns: " << mBoidPatterns.size() << std::endl;
		for (int i = 0; i < mBoidPatterns.size(); i++) {
			console() << "  Pattern " << i << ": " << mBoidPatterns[i]->getName()
			          << " active=" << mBoidPatterns[i]->isActive() << std::endl;
		}
		logged = true;
	}

	// Center boids around origin (similar to cellular patterns)
	vec3 boidDimensions = boids->dimensions();
	vec3 boidOffset = boidDimensions * 0.5f;

	// Iterate through each boid pattern
	int activePatternCount = 0;
	int spheresAdded = 0;
	for (const auto& pattern : mBoidPatterns) {
		if (!pattern->isActive()) continue;
		activePatternCount++;

		int patternId = pattern->getId();

		static bool loggedPatternMode = false;
		if (!loggedPatternMode) {
			console() << "Pattern " << patternId << " (" << pattern->getName() << ") is active" << std::endl;
			loggedPatternMode = true;
		}

		// Iterate through all boids
		for (int i = 0; i < boids->numBoids(); i++) {
			Boid* boid = boids->getBoidAtIndex(i);

			// Get render configuration for this boid
			BoidRenderConfig config = pattern->getRenderConfig(boid, i, boids, this);

			// Center boid position around origin
			vec3 centeredPos = boid->pos - boidOffset;

			static bool loggedMode = false;
			if (!loggedMode && i == 0) {
				console() << "Pattern " << patternId << " mode: " << (int)config.mode
				          << " (0=ENVMAP, 1=TRAILS, 2=CONNECTIONS, 3=SPLINES)" << std::endl;
				loggedMode = true;
			}

			// Render based on mode
			switch (config.mode) {
				case BoidRenderMode::ENVMAP: {
					// Environment mapped sphere - mark for env map rendering
					// Pattern ID 5 and 13 are used to trigger env map shader in drawSphereInstances
					int envMapPatternId = config.useEnvMap ? 5 : -1;
					addSphereInstance(centeredPos, config.color, config.size, envMapPatternId);
					spheresAdded++;
					break;
				}

				case BoidRenderMode::TRAILS: {
					// Main boid sphere
					addSphereInstance(centeredPos, config.color, config.size, -1);

					// Trail particles along velocity vector (backwards)
					// Check if velocity is non-zero before normalizing to prevent crash
					float velMag = glm::length(boid->vec);
					if (velMag > 0.001f) {
						vec3 velocityDir = glm::normalize(boid->vec);
						for (int t = 1; t <= config.trailLength; t++) {
							float trailFactor = (float)t / (float)config.trailLength;
							vec3 trailPos = centeredPos - velocityDir * trailFactor * 3.0f;

							// Fade trail particles
							ColorA trailColor = config.color;
							trailColor.a *= (1.0f - trailFactor * 0.8f);

							// Shrink trail particles
							float trailSize = config.size * (1.0f - trailFactor * 0.6f);

							addSphereInstance(trailPos, trailColor, trailSize, -1);
						}
					}
					break;
				}

				case BoidRenderMode::CONNECTIONS: {
					// Draw boid as small sphere
					static bool loggedConnection = false;
					if (!loggedConnection && i == 0) {
						console() << "CONNECTIONS: size=" << config.size << " color="
						          << config.color.r << "," << config.color.g << "," << config.color.b << "," << config.color.a
						          << " radius=" << config.connectionRadius << std::endl;
						loggedConnection = true;
					}
					// Use pattern ID 13 for fxp_* cubemap, 5 for fxic_* cubemap
					int envMapPatternId = config.useEnvMap ? (config.useEnvMapPattern13 ? 13 : 5) : -1;
					addSphereInstance(centeredPos, config.color, config.size, envMapPatternId);

					// Draw lines to nearby boids
					for (int j = i + 1; j < boids->numBoids(); j++) {
						Boid* otherBoid = boids->getBoidAtIndex(j);
						float dist = boid->distance(otherBoid);

						if (dist < config.connectionRadius) {
							// Calculate line color based on distance (closer = brighter)
							float distFactor = 1.0f - (dist / config.connectionRadius);
							// Use same color as nodes, modulated by distance
							ColorA lineColor = config.color;
							lineColor.r *= distFactor;
							lineColor.g *= distFactor;
							lineColor.b *= distFactor;
							lineColor.a *= distFactor;

							vec3 otherCenteredPos = otherBoid->pos - boidOffset;
							addLineInstance(centeredPos, otherCenteredPos, lineColor, config.lineWidth);
						}
					}
					break;
				}

				case BoidRenderMode::SPLINES: {
					// Spline rendering is handled below after collecting all boid positions
					break;
				}

				case BoidRenderMode::CUSTOM: {
					// Custom rendering can be added here for future patterns
					break;
				}
			}
		}
	}

	// Handle SPLINES mode - draw B-spline curves through all boid positions
	for (const auto& pattern : mBoidPatterns) {
		if (!pattern->isActive()) continue;

		// Check if this is a splines pattern (pattern ID 3)
		if (pattern->getId() == 3 && boids->numBoids() >= 4) {
			// Collect all boid positions
			vector<vec3> points;
			for (int i = 0; i < boids->numBoids(); i++) {
				Boid* boid = boids->getBoidAtIndex(i);
				points.push_back(boid->pos - boidOffset);
			}

			// Get color from pattern
			BoidRenderConfig config = pattern->getRenderConfig(boids->getBoidAtIndex(0), 0, boids, this);

			// Create 3 B-splines with different degrees (like old lambda app)
			BSpline3f splineX(points, 3, true, false);   // Degree 3, closed loop
			BSpline3f splineY(points, 5, true, false);   // Degree 5, closed loop
			BSpline3f splineZ(points, 7, true, false);   // Degree 7, closed loop (was 11, reduced for stability)

			// Sample splines and create line segments
			const int numSamples = 200;
			const float step = 1.0f / numSamples;

			// Spline X - thin lines
			for (int t = 0; t < numSamples; t++) {
				float t0 = t * step;
				float t1 = (t + 1) * step;
				vec3 p0 = splineX.getPosition(t0);
				vec3 p1 = splineX.getPosition(t1);
				addLineInstance(p0, p1, config.color, 1.0f);
			}

			// Spline Y - medium lines (brighter)
			ColorA colorY = config.color;
			colorY.r = glm::min(colorY.r * 1.2f, 1.0f);
			colorY.g = glm::min(colorY.g * 1.2f, 1.0f);
			colorY.b = glm::min(colorY.b * 1.2f, 1.0f);
			for (int t = 0; t < numSamples; t++) {
				float t0 = t * step;
				float t1 = (t + 1) * step;
				vec3 p0 = splineY.getPosition(t0);
				vec3 p1 = splineY.getPosition(t1);
				addLineInstance(p0, p1, colorY, 3.0f);
			}

			// Spline Z - thick lines (brightest)
			ColorA colorZ = config.color;
			colorZ.r = glm::min(colorZ.r * 1.5f, 1.0f);
			colorZ.g = glm::min(colorZ.g * 1.5f, 1.0f);
			colorZ.b = glm::min(colorZ.b * 1.5f, 1.0f);
			for (int t = 0; t < numSamples; t++) {
				float t0 = t * step;
				float t1 = (t + 1) * step;
				vec3 p0 = splineZ.getPosition(t0);
				vec3 p1 = splineZ.getPosition(t1);
				addLineInstance(p0, p1, colorZ, 5.0f);
			}
		}
	}

	static bool loggedRender = false;
	if (!loggedRender && activePatternCount > 0) {
		console() << "Active patterns: " << activePatternCount << ", spheres added: " << spheresAdded << std::endl;
		console() << "Total sphere instances: " << mSpherePositions.size() << std::endl;
		if (spheresAdded > 0) {
			console() << "First sphere: pos=" << mSpherePositions[0].x << "," << mSpherePositions[0].y << "," << mSpherePositions[0].z
			          << " color=" << mSphereColors[0].r << "," << mSphereColors[0].g << "," << mSphereColors[0].b << "," << mSphereColors[0].a
			          << " radius=" << mSphereRadii[0] << std::endl;
		}
		loggedRender = true;
	}
}

void GraphicsRenderer::drawFragment(Cell* cell) {
	int x, y, z;
	currentCell = cell;
	x = currentCell->x;
	y = currentCell->y;
	z = currentCell->z;

	state = currentCell->phase;

	// Early exit for dead cells - check actual state, not interpolated phase
	// phase can be > 0 during interpolation even when cell is dead (states[index] = 0)
	double actualState = currentCell->states[ptrWorld->index()];
	if (actualState <= 0.0) {
		return;
	}

	// New pattern system: iterate through all patterns
	for (const auto& pattern : mPatterns) {
		if (!pattern->getActive()) continue;

		// Check if this pattern applies to this cell
		if (!pattern->isActive(x, y, z, cell, ptrWorld)) continue;

		// Get render configuration
		RenderConfig config = pattern->getRenderConfig(x, y, z, cell, ptrWorld, this);
		int patternId = pattern->getId();

		// Compute final position
		vec3 position;
		position.x = (float)x * fragSizeX + (fragSizeX * 0.5f) - hx;
		position.y = (float)y * fragSizeY + (fragSizeY * 0.5f) - hy;
		position.z = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - hz;

		position += config.offset;

		// Compute final scale
		vec3 finalScale = config.scale * config.uniformScale;
		finalScale.x *= fragSizeX;
		finalScale.y *= fragSizeY;
		finalScale.z *= fragSizeZ;

		// Dispatch based on render mode
		switch (config.mode) {
			case RenderMode::CUBES: {
				// Pattern11 needs special Y position handling
				if (patternId == 11) {
					float yCompress = config.customFloats.at("yCompress");  // 0.5
					float yOffset = config.customFloats.at("yOffset");      // 0.25

					// Recalculate Y position: yB = y * (fragSizeX * 0.5) + (fragSizeX * 0.25)
					position.y = (float)y * fragSizeY * yCompress + fragSizeY * yOffset - hy;
				}
				addCubeInstance(position, config.color, finalScale, config.texture);
				break;
			}

			case RenderMode::SPHERES:
				addSphereInstance(position, config.color, config.uniformScale * fragSizeX, patternId);
				break;

			case RenderMode::CYLINDERS:
				addCylinderInstance(position, config.color, finalScale, config.rotation);
				break;

			case RenderMode::SPHERICAL_QUAD:
				addSphericalQuad(config.sphericalTheta, config.sphericalPhi, config.sphericalRho,
				                 config.sphericalGridPos, config.sphericalCornerColors, config.sphericalCellState);
				break;

			case RenderMode::LINES: {
				// For boundary wireframes, compute line segments for the cell's boundary edges
				float xL_local = (float)x * fragSizeX - hx;
				float yB_local = (float)y * fragSizeY - hx;
				float zF_local = (float)z * fragSizeZ - hx;
				float xR = xL_local + fragSizeX * config.scale.x * config.uniformScale * 4.0f;
				float yT = yB_local + fragSizeY * config.scale.y * config.uniformScale * 4.0f;
				float zB = zF_local + fragSizeZ * config.scale.z * config.uniformScale * 4.0f;

				// Draw wireframe box edges for this cell
				// Bottom face
				addLineInstance(vec3(xL_local, yB_local, zF_local), vec3(xR, yB_local, zF_local), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yB_local, zF_local), vec3(xR, yT, zF_local), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yT, zF_local), vec3(xL_local, yT, zF_local), config.color, config.lineWidth);
				addLineInstance(vec3(xL_local, yT, zF_local), vec3(xL_local, yB_local, zF_local), config.color, config.lineWidth);

				// Top face
				addLineInstance(vec3(xL_local, yB_local, zB), vec3(xR, yB_local, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yB_local, zB), vec3(xR, yT, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yT, zB), vec3(xL_local, yT, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xL_local, yT, zB), vec3(xL_local, yB_local, zB), config.color, config.lineWidth);

				// Vertical edges
				addLineInstance(vec3(xL_local, yB_local, zF_local), vec3(xL_local, yB_local, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yB_local, zF_local), vec3(xR, yB_local, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xR, yT, zF_local), vec3(xR, yT, zB), config.color, config.lineWidth);
				addLineInstance(vec3(xL_local, yT, zF_local), vec3(xL_local, yT, zB), config.color, config.lineWidth);
				break;
			}

			case RenderMode::PLANES: {
				int patternId = pattern->getId();

				if (patternId == 0) {
					// Pattern00: Nested boundary rectangles with inverse alpha
					float unmap = config.customFloats.at("unmap");

					bool onXNeg = config.customFloats.at("onXNeg") > 0.5f;
					bool onYNeg = config.customFloats.at("onYNeg") > 0.5f;
					bool onZNeg = config.customFloats.at("onZNeg") > 0.5f;
					bool onXPos = config.customFloats.at("onXPos") > 0.5f;
					bool onYPos = config.customFloats.at("onYPos") > 0.5f;
					bool onZPos = config.customFloats.at("onZPos") > 0.5f;

					// Draw 4 nested rectangles at sizes: 0.25, 0.5, 0.75, 1.0
					// with alpha = 1/size (larger rects have smaller alpha)
					float sizes[] = {0.25f, 0.5f, 0.75f, 1.0f};

					for (int i = 0; i < 4; i++) {
						float size = sizes[i];
						float alpha = 1.0f / size;  // Inverse relationship

						// Clamp alpha to reasonable range
						alpha = std::min(alpha, 1.0f);

						// Calculate rectangle dimensions for this size
						float xL_nest = (float)x * fragSizeX + (fragSizeX * 0.5f) - (fragSizeX * unmap * size) - hx;
						float yB_nest = (float)y * fragSizeY + (fragSizeY * 0.5f) - (fragSizeY * unmap * size) - hx;
						float zF_nest = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - (fragSizeZ * unmap * size) - hx;

						float xW_nest = fragSizeX * unmap * size * 2.0f;
						float yH_nest = fragSizeY * unmap * size * 2.0f;
						float zD_nest = fragSizeZ * unmap * size * 2.0f;

						// Create color with scaled alpha
						ColorA rectColor = config.color;
						rectColor.a *= alpha;

						// First (smallest) rect is filled, others are wireframe
						bool isWireframe = (i > 0);

						// Draw on each boundary
						if (onXNeg) {
							addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), rectColor, 1, isWireframe);
						}
						if (onYNeg) {
							addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), rectColor, 2, isWireframe);
						}
						if (onZNeg) {
							addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), rectColor, 0, isWireframe);
						}
						if (onXPos) {
							addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), rectColor, 1, isWireframe);
						}
						if (onYPos) {
							addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), rectColor, 2, isWireframe);
						}
						if (onZPos) {
							addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), rectColor, 0, isWireframe);
						}
					}
				} else if (patternId == 8) {
					// Pattern08: Center planes with nested rectangles
					float unmap = config.customFloats.at("unmap");
					bool onXPlane = config.customFloats.at("onXPlane") > 0.5f;
					bool onYPlane = config.customFloats.at("onYPlane") > 0.5f;
					bool onZPlane = config.customFloats.at("onZPlane") > 0.5f;

				// Draw initial stroked/filled rectangles
				float xL_base = (float)x * fragSizeX + (fragSizeX * 0.5f) - (fragSizeX * unmap) - hx;
				float yB_base = (float)y * fragSizeY + (fragSizeY * 0.5f) - (fragSizeY * unmap) - hy;
				float zF_base = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - (fragSizeZ * unmap) - hz;

				float xW_base = fragSizeX * unmap * 2.0f;
				float yH_base = fragSizeY * unmap * 2.0f;
				float zD_base = fragSizeZ * unmap * 2.0f;

				// First set: initial rectangles (stroked for X/Z, filled for Y)
				if (onXPlane) {
					// YZ plane - stroked
					addPlaneInstance(vec3(xL_base, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 1, true);
				}
				if (onYPlane) {
					// XZ plane - filled
					addPlaneInstance(vec3(xL_base, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 2, false);
				}
				if (onZPlane) {
					// XY plane - stroked
					addPlaneInstance(vec3(xL_base, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 0, true);
				}

				// Draw nested rectangles at different scales
				float sizes[] = {1.0f, 0.5f, 0.25f, 0.125f};
				ColorA currentColor = config.color;

				for (int i = 0; i < 4; i++) {
					// Center each nested rectangle properly
					float scale = unmap * (2.0f - sizes[i]);
					float xL_nest = (float)x * fragSizeX + (fragSizeX * 0.5f) - (fragSizeX * scale) - hx;
					float yB_nest = (float)y * fragSizeY + (fragSizeY * 0.5f) - (fragSizeY * scale) - hy;
					float zF_nest = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - (fragSizeZ * scale) - hz;

					float xW_nest = fragSizeX * scale * 2.0f;
					float yH_nest = fragSizeY * scale * 2.0f;
					float zD_nest = fragSizeZ * scale * 2.0f;

					// Fade alpha for each nested level
					currentColor.a *= 0.87f;

					// Second set: nested rectangles (filled for X, stroked for Y, filled for Z)
					if (onXPlane) {
						// YZ plane - filled
						addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), currentColor, 1, false);
					}
					if (onYPlane) {
						// XZ plane - stroked
						addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), currentColor, 2, true);
					}
					if (onZPlane) {
						// XY plane - filled
						addPlaneInstance(vec3(xL_nest, yB_nest, zF_nest), vec3(xW_nest, yH_nest, zD_nest), currentColor, 0, false);
					}
				}
				} else if (patternId == 10) {
					// Pattern10: Boundary rectangles with state-based fill/stroke
					float unmap = config.customFloats.at("unmap");
					float mapState = config.customFloats.at("mapState");
					bool shouldFill = config.customFloats.at("shouldFill") > 0.5f;

					bool onXNeg = config.customFloats.at("onXNeg") > 0.5f;
					bool onYNeg = config.customFloats.at("onYNeg") > 0.5f;
					bool onZNeg = config.customFloats.at("onZNeg") > 0.5f;
					bool onXPos = config.customFloats.at("onXPos") > 0.5f;
					bool onYPos = config.customFloats.at("onYPos") > 0.5f;
					bool onZPos = config.customFloats.at("onZPos") > 0.5f;

					// Calculate rectangle dimensions
					float xL_base = (float)x * fragSizeX + (fragSizeX * 0.5f) - (fragSizeX * unmap) - hx;
					float yB_base = (float)y * fragSizeY + (fragSizeY * 0.5f) - (fragSizeY * unmap) - hx;
					float zF_base = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - (fragSizeZ * unmap) - hx;

					float xW_base = fragSizeX * unmap * 2.0f;
					float yH_base = fragSizeY * unmap * 2.0f;
					float zD_base = fragSizeZ * unmap * 2.0f;

					// Draw on each boundary
					if (onXNeg) {
						addPlaneInstance(vec3(xL_base, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 1, !shouldFill);
					}
					if (onYNeg) {
						addPlaneInstance(vec3(xL_base, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 2, !shouldFill);
					}
					if (onZNeg) {
						addPlaneInstance(vec3(xL_base, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 0, !shouldFill);
					}

					// For positive boundaries, adjust position based on unmap
					if (onXPos) {
						float xL_pos = xL_base + (xW_base * unmap);
						addPlaneInstance(vec3(xL_pos, yB_base, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 1, !shouldFill);
					}
					if (onYPos) {
						float yB_pos = yB_base + (yH_base * unmap);
						addPlaneInstance(vec3(xL_base, yB_pos, zF_base), vec3(xW_base, yH_base, zD_base), config.color, 2, !shouldFill);
					}
					if (onZPos) {
						float zF_pos = zF_base + (zD_base * unmap);
						addPlaneInstance(vec3(xL_base, yB_base, zF_pos), vec3(xW_base, yH_base, zD_base), config.color, 0, !shouldFill);
					}
				}
				break;
			}

			case RenderMode::POLYGON: {
				// Pattern09: Smooth polygons connecting neighbors
				// Vertices are already in world space from the pattern
				addPolygonInstance(config.polygonVertices, config.polygonColors);
				break;
			}

			case RenderMode::CUSTOM: {
				// Pattern03: Spherical neighbor connections with dynamic motion
				if (patternId == 3) {
					float state = config.customFloats.at("state");
					float unmap = config.customFloats.at("unmap");
					int cellX = static_cast<int>(config.customFloats.at("x"));
					int cellY = static_cast<int>(config.customFloats.at("y"));
					int cellZ = static_cast<int>(config.customFloats.at("z"));

					// Get neighbor 3
					Cell* otherCell = ptrWorld->rule()->getNeighbor(currentCell, 3);
					float otherState = otherCell->phase;

					// Animation phase based on counter (wraps around maxphase=28)
					float maxPhase = 28.0f;
					float animPhase = (2.0f * M_PI / maxPhase) * (fmod(counter, maxPhase) / maxPhase);

					// Calculate spherical coordinates for current cell
					float thetaA = ((2.0f * M_PI) / ptrWorld->sizeX() * cellX) + animPhase;
					float phiA = ((2.0f * M_PI) / ptrWorld->sizeY() * cellY) + animPhase;
					float rhoA = cellZ * (fragSizeX * 0.5f) + (fragSizeX * unmap);

					// Convert to Cartesian
					float xL = rhoA * cos(thetaA) * cos(phiA);
					float yB = rhoA * sin(thetaA) * cos(phiA);
					float zF = rhoA * sin(phiA);

					// Calculate spherical coordinates for neighbor cell
					float thetaB = ((2.0f * M_PI) / ptrWorld->sizeX() * otherCell->x) + animPhase;
					float phiB = ((2.0f * M_PI) / ptrWorld->sizeY() * otherCell->y) + animPhase;
					float rhoB = cellZ * (fragSizeX * 0.5f) + (fragSizeX * otherState);

					// Convert to Cartesian
					float xW = rhoB * cos(thetaB) * cos(phiB);
					float yH = rhoB * sin(thetaB) * cos(phiB);
					float zD = rhoB * sin(phiB);

					// Draw points at both positions
					addPointInstance(vec3(xL, yB, zF), config.color, 4.0f);
					addPointInstance(vec3(xW, yH, zD), config.color, 4.0f);

					// Draw line connecting them
					addLineInstance(vec3(xL, yB, zF), vec3(xW, yH, zD), config.color, 1.0f);
				}
				// Pattern12: Composite sphere + cube + wireframe cube
				else if (patternId == 12) {
					float mapState = config.customFloats.at("mapState");

					// Calculate geometry size and position
					float xW = fragSizeX * mapState;
					float yH = fragSizeY * mapState;
					float zD = fragSizeZ * mapState;

					vec3 cubeScale(xW, yH, zD);

					// 1. Draw sphere (radius = xW * 0.5, which is size * mapState * 0.5)
					addSphereInstance(position, config.color, xW * 0.5f, patternId);

					// 2. Draw filled cube with brightened color
					ColorA brightenedColor = config.color;
					brightenedColor.r = std::min(brightenedColor.r + 0.2f, 1.0f);
					brightenedColor.g = std::min(brightenedColor.g + 0.2f, 1.0f);
					brightenedColor.b = std::min(brightenedColor.b + 0.2f, 1.0f);
					addCubeInstance(position, brightenedColor, cubeScale, nullptr);

					// 3. Draw wireframe cube with full alpha
					// Use lines to draw wireframe cube
					ColorA wireframeColor = config.color;
					wireframeColor.a = 1.0f;  // Full alpha

					// Calculate cube corners
					float halfW = xW * 0.5f;
					float halfH = yH * 0.5f;
					float halfD = zD * 0.5f;

					vec3 v000 = position + vec3(-halfW, -halfH, -halfD);
					vec3 v001 = position + vec3(-halfW, -halfH,  halfD);
					vec3 v010 = position + vec3(-halfW,  halfH, -halfD);
					vec3 v011 = position + vec3(-halfW,  halfH,  halfD);
					vec3 v100 = position + vec3( halfW, -halfH, -halfD);
					vec3 v101 = position + vec3( halfW, -halfH,  halfD);
					vec3 v110 = position + vec3( halfW,  halfH, -halfD);
					vec3 v111 = position + vec3( halfW,  halfH,  halfD);

					// Draw 12 edges of the wireframe cube
					// Bottom face (y = -halfH)
					addLineInstance(v000, v100, wireframeColor, 1.0f);
					addLineInstance(v100, v101, wireframeColor, 1.0f);
					addLineInstance(v101, v001, wireframeColor, 1.0f);
					addLineInstance(v001, v000, wireframeColor, 1.0f);

					// Top face (y = halfH)
					addLineInstance(v010, v110, wireframeColor, 1.0f);
					addLineInstance(v110, v111, wireframeColor, 1.0f);
					addLineInstance(v111, v011, wireframeColor, 1.0f);
					addLineInstance(v011, v010, wireframeColor, 1.0f);

					// Vertical edges
					addLineInstance(v000, v010, wireframeColor, 1.0f);
					addLineInstance(v100, v110, wireframeColor, 1.0f);
					addLineInstance(v101, v111, wireframeColor, 1.0f);
					addLineInstance(v001, v011, wireframeColor, 1.0f);
				}
				else if (patternId == 13) {
					// Pattern13: Network visualization with cube-mapped spheres and line connections
					float mapState = config.customFloats.at("mapState");
					float offsetX = config.customFloats.at("offsetX");
					float offsetY = config.customFloats.at("offsetY");
					float offsetZ = config.customFloats.at("offsetZ");
					float cellState = config.customFloats.at("cellState");

					// Apply circular motion offset
					vec3 spherePos = position + vec3(
						fragSizeX * offsetX,
						fragSizeY * offsetY,
						fragSizeZ * offsetZ
					);

					// Calculate sphere size - state == 1.0 uses fixed size, otherwise uses mapState
					float sphereRadius;
					if (cellState == 1.0f) {
						sphereRadius = fragSizeX * 0.47f;
					} else {
						sphereRadius = mapState * fragSizeX * 0.47f;
					}

					// 1. Draw cube-mapped sphere (uses Pattern13's ID for env mapping)
					addSphereInstance(spherePos, config.color, sphereRadius, patternId);

					// 2. Draw thin lines connecting to neighbors (much faster than cylinders!)
					// Iterate through all 26 neighbors
					for (int i = 0; i < 26; i++) {
						Cell* neighbor = ptrWorld->rule()->getNeighbor(currentCell, i);

						// Only connect to neighbors with low phase (1.0-10.0)
						if (neighbor->phase > 0.0f && neighbor->phase < 10.0f) {
							// Calculate neighbor position with same circular motion
							float neighborMaxState = ptrWorld->rule()->numStates() - 1;
							float neighborState = neighbor->states[ptrWorld->index()];
							float neighborMapState = (neighborMaxState - neighborState) * (1.0f / neighborMaxState);
							float neighborAngle = (1.0f - neighborMapState) * 2.0f * M_PI;

							vec3 neighborPos;
							neighborPos.x = (float)neighbor->x * fragSizeX + (fragSizeX * 0.5f) - hx;
							neighborPos.y = (float)neighbor->y * fragSizeY + (fragSizeY * 0.5f) - hy;
							neighborPos.z = (float)neighbor->z * fragSizeZ + (fragSizeZ * 0.5f) - hz;

							neighborPos += vec3(
								fragSizeX * sin(neighborAngle),
								fragSizeY * cos(neighborAngle),
								fragSizeZ * sin(neighborAngle)
							);

							// Calculate connection color (fades based on neighbor phase)
							float phaseFade = 1.0f - ((neighbor->phase - 1.0f) / 11.0f) * 0.7f;  // linlin(phase, 1, 12, 0.0, 0.7)
							ColorA connectionColor = config.color;
							connectionColor.r *= phaseFade;
							connectionColor.g *= phaseFade;
							connectionColor.b *= phaseFade;

							// Draw thin line connection (instanced rendering - very fast!)
							addLineInstance(spherePos, neighborPos, connectionColor, 1.0f);
						}
					}
				}
				else if (patternId == 15) {
					// Pattern15: Random points with BMU glow
					float cstate = config.customFloats.at("cstate");
					float x = config.customFloats.at("x");
					float y = config.customFloats.at("y");
					float z = config.customFloats.at("z");
					bool isBMU = (config.customFloats.at("isBMU") > 0.5f);

					// Vary number of points based on cell state
					// State 1.0 (lowest) = 13 points, higher states = fewer points down to 5
					// Using linear interpolation: pts = 13 - (cstate - 1) * 8 / (maxState - 1)
					// For typical maxState of 10: state 1 = 13 pts, state 10 = 5 pts
					float maxState = ptrWorld->rule()->numStates() - 1;
					float normalizedState = (cstate - 1.0f) / std::max(1.0f, maxState - 1.0f);
					int pts = static_cast<int>(13.0f - normalizedState * 8.0f);
					pts = std::max(5, std::min(13, pts));  // Clamp between 5 and 13

					for (int i = 1; i <= pts; i++) {
						// Random position within cell bounds
						float randX = x * fragSizeX + randFloat() * fragSizeX - hx;
						float randY = y * fragSizeY + randFloat() * fragSizeY - hy;
						float randZ = z * fragSizeZ + randFloat() * fragSizeZ - hx;

						vec3 pointPos(randX, randY, randZ);

						// Color with enhanced BMU glow
						ColorA pointColor = config.color;
						if (isBMU) {
							// Much brighter glow for BMU cell (8x instead of 3x)
							// Use additive blending so values can exceed 1.0
							pointColor.r = pointColor.r * 8.0f;
							pointColor.g = pointColor.g * 8.0f;
							pointColor.b = pointColor.b * 8.0f;
							pointColor.a = 1.0f;  // Full alpha for maximum glow
						}

						addPointInstance(pointPos, pointColor, 1.0f);
					}
				}
				else if (patternId == 16) {
					// Pattern16: Conditional plane rectangles based on neighbor states
					float unmap = config.customFloats.at("unmap");

					// Calculate rectangle dimensions
					float rectSize = mapf(unmap, 0.3f, 0.8f);

					float xL_base = (float)x * fragSizeX + (fragSizeX * 0.5f) - (fragSizeX * rectSize) - hx;
					float yB_base = (float)y * fragSizeY + (fragSizeY * 0.5f) - (fragSizeY * rectSize) - hx;
					float zF_base = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - (fragSizeZ * rectSize) - hx;

					float xW = fragSizeX * rectSize * 2.0f;
					float yH = fragSizeY * rectSize * 2.0f;
					float zD = fragSizeZ * rectSize * 2.0f;

					// Neighbor indices and corresponding plane types from original pattern23
					// ind[12] = { 4, 1, 10, 2, 12, 0, 14, 0, 16, 2, 22, 1 };
					// Pairs are: (neighborIndex, planeType)
					// PlaneType: 0=XY, 1=YZ, 2=XZ
					struct NeighborCheck {
						int neighborIndex;
						int planeType;
					};
					NeighborCheck checks[6] = {
						{4, 1},   // Neighbor 4, YZ plane
						{10, 2},  // Neighbor 10, XZ plane
						{12, 0},  // Neighbor 12, XY plane
						{14, 0},  // Neighbor 14, XY plane
						{16, 2},  // Neighbor 16, XZ plane
						{22, 1}   // Neighbor 22, YZ plane
					};

					// Check each neighbor and draw plane if neighbor state is 0
					for (int i = 0; i < 6; i++) {
						Cell* neighbor = ptrWorld->rule()->getNeighbor(currentCell, checks[i].neighborIndex);
						if (neighbor->states[ptrWorld->index()] == 0.0f) {
							// Draw filled rectangle on the specified plane
							addPlaneInstance(
								vec3(xL_base, yB_base, zF_base),
								vec3(xW, yH, zD),
								config.color,
								checks[i].planeType,
								false  // filled, not wireframe
							);
						}
					}
				}
				else if (patternId == 17) {
					// Pattern17: Combined animated lines (pattern24 + pattern25)
					float cstate = config.customFloats.at("cstate");

					// --- Pattern24 part: Single sine-wave animated line ---
					float xL = x * fragSizeX + fragSizeX - (fragSizeX * 2.0f * cstate);
					float yB = y * fragSizeY + (fragSizeY * 0.5f);
					float zF = z * fragSizeX + (fragSizeX * 0.5f);

					xL -= hx;
					yB -= hy;
					zF -= hx;

					float zD = zF;

					// Sine wave animation
					yB += (fragSizeY * 4.0f * sin(cstate * 2.0f * M_PI)) - (fragSizeY * 8.0f * sin(cstate * 2.0f * M_PI));

					float xW = fragSizeX * cstate * 4.0f;
					float yH = yB;

					// Color using configured pattern color
					ColorA line1Color = config.color;
					addLineInstance(vec3(xL, yB, zF), vec3(xL + xW, yH, zD), line1Color, 1.0f);

					// --- Pattern25 part: Cross pattern with 4 lines ---
					// Line 1: horizontal top-left to offset
					xL = x * fragSizeX + (fragSizeX * 0.25f);
					yB = y * fragSizeY + (fragSizeY * 0.25f);
					zF = z * fragSizeX + (fragSizeX * 0.5f);
					xW = fragSizeX * cstate * 2.0f;

					xL -= hx;
					yB -= hy;
					zF -= hx;

					ColorA line2Color = config.color;
					addLineInstance(vec3(xL, yB, zF), vec3(xL + xW, yB, zF), line2Color, 1.0f);

					// Line 2: horizontal bottom-right to offset (opposite direction)
					xL = x * fragSizeX + (fragSizeX * 0.75f);
					yB = y * fragSizeY + (fragSizeY * 0.75f);
					xW = fragSizeX * cstate * -2.0f;

					xL -= hx;
					yB -= hy;

					ColorA line3Color = config.color;
					addLineInstance(vec3(xL, yB, zF), vec3(xL + xW, yB, zF), line3Color, 1.0f);

					// Line 3: vertical top-right to offset (inverted color)
					xL = x * fragSizeX + (fragSizeX * 0.75f);
					yB = y * fragSizeY + (fragSizeY * 0.25f);
					float yH_offset = fragSizeX * cstate * 2.0f;

					xL -= hx;
					yB -= hy;

					ColorA line4Color(
						1.0f - config.color.r,
						1.0f - config.color.g,
						1.0f - config.color.b,
						config.color.a
					);
					addLineInstance(vec3(xL, yB, zF), vec3(xL, yB + yH_offset, zF), line4Color, 1.0f);

					// Line 4: vertical bottom-left to offset (opposite direction, inverted color)
					xL = x * fragSizeX + (fragSizeX * 0.25f);
					yB = y * fragSizeY + (fragSizeY * 0.75f);
					yH_offset = fragSizeX * cstate * -2.0f;

					xL -= hx;
					yB -= hy;

					ColorA line5Color(
						1.0f - config.color.r,
						1.0f - config.color.g,
						1.0f - config.color.b,
						config.color.a
					);
					addLineInstance(vec3(xL, yB, zF), vec3(xL, yB + yH_offset, zF), line5Color, 1.0f);
				}
				else if (patternId == 19 || patternId == 20) {
					// Pattern19/20: Dynamic neighbor polygon fan (pattern35)
					// Draws a polygon connecting the center cell to all active neighbors
					float unmap = config.customFloats.at("unmap");
					bool filled = (config.customFloats.at("filled") > 0.5f);

					// Center vertex position
					float centerX = (float)x * fragSizeX + (fragSizeX * 0.5f) - hx;
					float centerY = (float)y * fragSizeY + (fragSizeY * 0.5f) - hx;
					float centerZ = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - hx;
					vec3 centerPos(centerX, centerY, centerZ);

					// Collect all active neighbor vertices
					std::vector<vec3> neighborPositions;
					std::vector<ColorA> neighborColors;

					int nSize = ptrWorld->rule()->nSize();
					for (int i = 0; i < nSize; i++) {
						Cell* neighbor = ptrWorld->rule()->getNeighbor(currentCell, i);
						if (neighbor->phase > 0.0f) {
							// Calculate neighbor position
							float nX = (float)neighbor->x * fragSizeX + (fragSizeX * 0.5f) - hx;
							float nY = (float)neighbor->y * fragSizeY + (fragSizeY * 0.5f) - hx;
							float nZ = (float)neighbor->z * fragSizeZ + (fragSizeZ * 0.5f) - hx;

							neighborPositions.push_back(vec3(nX, nY, nZ));

							// Calculate color for this neighbor
							float maxState = ptrWorld->rule()->numStates() - 1;
							float neighborUnmap = 1.0f - (neighbor->phase / maxState);

							// Use pattern's color mapping
							Pattern* pattern = getPattern(patternId);
							if (pattern) {
								ColorA neighborColor;
								float colorMapValue = pattern->getColorMap();
								float alphaMapValue = pattern->getAlphaMap();
								Color baseColor = pattern->getColor();
								float baseAlpha = pattern->getAlpha();

								neighborColor.r = baseColor.r * abs(colorMapValue - neighborUnmap);
								neighborColor.g = baseColor.g * abs(colorMapValue - neighborUnmap);
								neighborColor.b = baseColor.b * abs(colorMapValue - neighborUnmap);
								neighborColor.a = baseAlpha * abs(alphaMapValue - neighborUnmap);

								neighborColors.push_back(neighborColor);
							} else {
								neighborColors.push_back(config.color);
							}
						}
					}

					// Draw polygon fan
					if (neighborPositions.size() >= 2) {
						if (filled) {
							// Pattern20: Batched filled triangles (GPU instanced)
							// Add all triangles to the batch for efficient rendering
							for (size_t i = 0; i < neighborPositions.size(); i++) {
								size_t nextIdx = (i + 1) % neighborPositions.size();

								// Add triangle: center -> neighbor[i] -> neighbor[i+1]
								addTriangleInstance(
									centerPos, neighborPositions[i], neighborPositions[nextIdx],
									config.color, neighborColors[i], neighborColors[nextIdx]
								);
							}
						} else {
							// Pattern19: Wireframe lines
							for (size_t i = 0; i < neighborPositions.size(); i++) {
								size_t nextIdx = (i + 1) % neighborPositions.size();

								ColorA colors[3] = {
									config.color,  // Center color
									neighborColors[i],
									neighborColors[nextIdx]
								};

								// Draw edges with appropriate colors
								addLineInstance(centerPos, neighborPositions[i],
									ColorA((colors[0].r + colors[1].r) * 0.5f,
										   (colors[0].g + colors[1].g) * 0.5f,
										   (colors[0].b + colors[1].b) * 0.5f,
										   (colors[0].a + colors[1].a) * 0.5f), 1.0f);
							}

							// Also connect neighbors to each other to complete the fan
							for (size_t i = 0; i < neighborPositions.size(); i++) {
								size_t nextIdx = (i + 1) % neighborPositions.size();
								addLineInstance(neighborPositions[i], neighborPositions[nextIdx],
									ColorA((neighborColors[i].r + neighborColors[nextIdx].r) * 0.5f,
										   (neighborColors[i].g + neighborColors[nextIdx].g) * 0.5f,
										   (neighborColors[i].b + neighborColors[nextIdx].b) * 0.5f,
										   (neighborColors[i].a + neighborColors[nextIdx].a) * 0.5f), 1.0f);
							}
						}
					}
				}
				else if (patternId == 23) {
					// Pattern23: Hexagonal boundary cells
					float unmap = config.customFloats.at("unmap");

					bool onXNeg = config.customFloats.at("onXNeg") > 0.5f;
					bool onYNeg = config.customFloats.at("onYNeg") > 0.5f;
					bool onZNeg = config.customFloats.at("onZNeg") > 0.5f;
					bool onXPos = config.customFloats.at("onXPos") > 0.5f;
					bool onYPos = config.customFloats.at("onYPos") > 0.5f;
					bool onZPos = config.customFloats.at("onZPos") > 0.5f;

					// Draw 4 nested hexagons at sizes: 0.25, 0.5, 0.75, 1.0
					float sizes[] = {0.25f, 0.5f, 0.75f, 1.0f};

					for (int i = 0; i < 4; i++) {
						float size = sizes[i];
						float alpha = 1.0f / size;  // Inverse relationship
						alpha = std::min(alpha, 1.0f);

						// Create color with scaled alpha
						ColorA hexColor = config.color;
						hexColor.a *= alpha;

						// First (smallest) hexagon is filled, others are wireframe
						bool isFilled = (i == 0);

						// Calculate hexagon center and radius
						float centerX = (float)x * fragSizeX + (fragSizeX * 0.5f) - hx;
						float centerY = (float)y * fragSizeY + (fragSizeY * 0.5f) - hx;
						float centerZ = (float)z * fragSizeZ + (fragSizeZ * 0.5f) - hx;

						float radius = fragSizeX * unmap * size;

						// Draw hexagons on appropriate boundaries
						if (onXNeg || onXPos) {
							// YZ plane hexagon
							for (int v = 0; v < 6; v++) {
								float angle1 = (v / 6.0f) * 2.0f * M_PI;
								float angle2 = ((v + 1) / 6.0f) * 2.0f * M_PI;

								vec3 v1(centerX, centerY + radius * cos(angle1), centerZ + radius * sin(angle1));
								vec3 v2(centerX, centerY + radius * cos(angle2), centerZ + radius * sin(angle2));

								if (isFilled) {
									// Draw filled triangle from center
									addTriangleInstance(vec3(centerX, centerY, centerZ), v1, v2, hexColor, hexColor, hexColor);
								} else {
									// Draw wireframe edge
									addLineInstance(v1, v2, hexColor, 1.0f);
								}
							}
						}

						if (onYNeg || onYPos) {
							// XZ plane hexagon
							for (int v = 0; v < 6; v++) {
								float angle1 = (v / 6.0f) * 2.0f * M_PI;
								float angle2 = ((v + 1) / 6.0f) * 2.0f * M_PI;

								vec3 v1(centerX + radius * cos(angle1), centerY, centerZ + radius * sin(angle1));
								vec3 v2(centerX + radius * cos(angle2), centerY, centerZ + radius * sin(angle2));

								if (isFilled) {
									addTriangleInstance(vec3(centerX, centerY, centerZ), v1, v2, hexColor, hexColor, hexColor);
								} else {
									addLineInstance(v1, v2, hexColor, 1.0f);
								}
							}
						}

						if (onZNeg || onZPos) {
							// XY plane hexagon
							for (int v = 0; v < 6; v++) {
								float angle1 = (v / 6.0f) * 2.0f * M_PI;
								float angle2 = ((v + 1) / 6.0f) * 2.0f * M_PI;

								vec3 v1(centerX + radius * cos(angle1), centerY + radius * sin(angle1), centerZ);
								vec3 v2(centerX + radius * cos(angle2), centerY + radius * sin(angle2), centerZ);

								if (isFilled) {
									addTriangleInstance(vec3(centerX, centerY, centerZ), v1, v2, hexColor, hexColor, hexColor);
								} else {
									addLineInstance(v1, v2, hexColor, 1.0f);
								}
							}
						}
					}
				}
				break;
			}

			default:
				break;
		}
	}

}

// pattern00 removed - now using new pattern system in pattern.cpp
// pattern01 removed - now using new pattern system in pattern.cpp

void GraphicsRenderer::pattern02(int x, int y, int z) {

	float cstate;
	
	if (ptrWorld->ruleType() == CONT) {
		cstate = state;
	}
	else {
		if (state != 0)
		{
			cstate = 1.0 / state;
		}
		else {
			cstate = 0.0f;
		}
	}
	
	xL = ((ptrWorld->sizeX()*fragSizeX/2) - ((float)x * fragSizeX)) * cstate + (fragSizeX * 0.5) - (fragSizeX * 2.0 * cstate);
	yB = ((ptrWorld->sizeY()*fragSizeX/2) - ((float)y * fragSizeY)) * cstate + (fragSizeY * 0.5) - (fragSizeY * 2.0 * cstate);
	zF = ((ptrWorld->sizeZ()*fragSizeX/2) - ((float)z * fragSizeZ)) * cstate + (fragSizeZ * 0.5) - (fragSizeZ * 2.0 * cstate);
	
	xW = mapf(fragSizeX * cstate, fragSizeX * 0.5, fragSizeX * 2.0);
	yH = mapf(fragSizeX * cstate, fragSizeX * 0.5, fragSizeX * 2.0);
	zD = mapf(fragSizeX * cstate, fragSizeX * 0.5, fragSizeX * 2.0);
	
	Pattern* p = getPattern(2);
	if (!p) return;
	red = p->getColor().r * abs(p->getColorMap() - cstate);
	green = p->getColor().g * abs(p->getColorMap() - cstate);
	blue = p->getColor().b * abs(p->getColorMap() - cstate);
	alpha = p->getAlpha() * abs(p->getAlphaMap() - cstate);
    
    gl::color(red, green, blue, alpha);
			
    

}

void GraphicsRenderer::pattern03(int x, int y, int z) {
    float cstate;

    if (ptrWorld->ruleType() == CONT) {
        cstate = state;
    }
    else {
        if (state != 0)
        {
            cstate = 1.0 / state;
        }
        else {
            cstate = 0.0f;
        }
    }

    if (ptrWorld->ruleType() == CONT || cstate != 0.0f) {

        float mapState, maxState;

        maxState = ptrWorld->rule()->numStates() - 1;
        mapState = (maxState - currentCell->states[ptrWorld->index()]) * (1 / maxState);

        xL = (float)x * fragSizeX + fragSizeX - (fragSizeX * 0.5);
        yB = (float)y * fragSizeX + fragSizeX - (fragSizeX * 0.5);
        zF = (float)z * fragSizeX + fragSizeX - (fragSizeX * 0.5);

        xW = yH = zD = mapf(fragSizeX * mapState, 0.25, 0.75);

        xL -= hx;
        yB -= hx;
        zF -= hx;

        Pattern* p = getPattern(3);
        if (!p) return;
        red = p->getColor().r * abs(p->getColorMap() - mapState);
        green = p->getColor().g * abs(p->getColorMap() - mapState);
        blue = p->getColor().b * abs(p->getColorMap() - mapState);
        alpha = p->getAlpha() * abs(p->getAlphaMap() - mapState);

        // Collect sphere instances instead of immediate drawing
        ColorA color(red, green, blue, alpha);
        vec3 position(xL, yB, zF);
        addSphereInstance(position, color, xW);

    }
}

void GraphicsRenderer::pattern04(int x, int y, int z) {


        float unmap, cstate;

        unmap = 1.0-unmapf(state, 0, ptrWorld->rule()->numStates()-1);
        cstate = currentCell->states[ptrWorld->index()];

    if (ptrWorld->ruleType() == CONT || cstate != 0.0f) {

        xL = (float)x * fragSizeX + (fragSizeX * 0.5) - (fragSizeX * unmap);
        yB = (float)y * fragSizeX + (fragSizeY * 0.5) - (fragSizeY * unmap);
        zF = (float)z * fragSizeZ + (fragSizeZ * 0.5) - (fragSizeZ * unmap);

        xW = fragSizeX * unmap * 2.0;
        yH = fragSizeX * unmap * 2.0;
        zD = fragSizeZ * unmap * 2.0;

        xL -= hx;
        yB -= hx;
        zF -= hx;

        Pattern* p = getPattern(4);
        if (!p) return;
        red = p->getColor().r * abs(p->getColorMap() - unmap);
        green = p->getColor().g * abs(p->getColorMap() - unmap);
        blue = p->getColor().b * abs(p->getColorMap() - unmap);
        alpha = p->getAlpha() * abs(p->getAlphaMap() - unmap);

        ColorA color(red, green, blue, alpha);

        // Collect cube instances instead of immediate drawing
        if (x == 0) {
            vec3 pos(xL + fragSizeX * 0.05f, yB + yH * 0.5f, zF + zD * 0.5f);
            vec3 scale(fragSizeX * 0.1f, yH, zD);
            addCubeInstance(pos, color, scale);
        }

        if (y == 0) {
            vec3 pos(xL + xW * 0.5f, yB + fragSizeY * 0.05f, zF + zD * 0.5f);
            vec3 scale(xW, fragSizeY * 0.1f, zD);
            addCubeInstance(pos, color, scale);
        }

        if (z == 0) {
            vec3 pos(xL + xW * 0.5f, yB + yH * 0.5f, zF + fragSizeZ * 0.05f);
            vec3 scale(xW, yH, fragSizeZ * 0.1f);
            addCubeInstance(pos, color, scale);
        }

        if (x == ptrWorld->sizeX()-1) {
            float xL2 = xL + (xW * cstate);
            vec3 pos(xL2 + fragSizeX * 0.05f, yB + yH * 0.5f, zF + zD * 0.5f);
            vec3 scale(fragSizeX * 0.1f, yH, zD);
            addCubeInstance(pos, color, scale);
        }
        if (y == ptrWorld->sizeY()-1 ) {
            float yB2 = yB + (yH * cstate);
            vec3 pos(xL + xW * 0.5f, yB2 + fragSizeY * 0.05f, zF + zD * 0.5f);
            vec3 scale(xW, fragSizeY * 0.1f, zD);
            addCubeInstance(pos, color, scale);
        }
        if (z == ptrWorld->sizeZ()-1 ) {
            float zF2 = zF + (zD * cstate);
            vec3 pos(xL + xW * 0.5f, yB + yH * 0.5f, zF2 + fragSizeZ * 0.05f);
            vec3 scale(xW, yH, fragSizeZ * 0.1f);
            addCubeInstance(pos, color, scale);
        }

    }
}



void GraphicsRenderer::fillRect(int plane) {
    // Define vertices for each plane
    std::vector<vec3> vertices;
    switch (plane) {
        case 0: // XY plane
            vertices = {
                vec3(xL, yB, zF),
                vec3(xL + xW, yB, zF),
                vec3(xL + xW, yB + yH, zF),
                vec3(xL, yB + yH, zF)
            };
            break;

        case 1: // YZ plane
            vertices = {
                vec3(xL, yB, zF),
                vec3(xL, yB, zF + zD),
                vec3(xL, yB + yH, zF + zD),
                vec3(xL, yB + yH, zF)
            };
            break;

        case 2: // XZ plane
            vertices = {
                vec3(xL, yB, zF),
                vec3(xL + xW, yB, zF),
                vec3(xL + xW, yB, zF + zD),
                vec3(xL, yB, zF + zD)
            };
            break;

        default:
            return; // Invalid plane
    }

    // Apply randomized color
//    float randRed = red * mapf(randFloat(), 0.9f, 1.1f);
//    float randGreen = green * mapf(randFloat(), 0.9f, 1.1f);
//    float randBlue = blue * mapf(randFloat(), 0.9f, 1.1f);

    gl::ScopedColor scopedColor(ColorA(red, green, blue, alpha));

    // Create geometry and draw
    gl::VboMeshRef mesh = gl::VboMesh::create(geom::Plane()
                                              .subdivisions(ivec2(1, 1))
                                              .size(vec2(xW, yH)));
    gl::ScopedModelMatrix scopedModel;
    gl::translate(vec3(xL, yB, zF));
    gl::draw(mesh);
}


// strokeRect removed - pattern00 now uses instanced line rendering

void GraphicsRenderer::drawEdges(const std::vector<vec3>& vertices) {
    for (size_t i = 0; i < vertices.size(); i += 2) {
        gl::drawLine(vertices[i], vertices[i + 1]);
    }
}

void GraphicsRenderer::drawPlaneFilled(float xL, float yB, float zF, float xW, float yH, float zD, int planeType, const ColorA& color) {
    gl::ScopedColor scopedColor(color);
    gl::ScopedModelMatrix scopedModel;

    vec3 v0, v1, v2, v3;

    switch (planeType) {
        case 0: // XY plane (Z fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL + xW, yB, zF);
            v2 = vec3(xL + xW, yB + yH, zF);
            v3 = vec3(xL, yB + yH, zF);
            break;
        case 1: // YZ plane (X fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL, yB + yH, zF);
            v2 = vec3(xL, yB + yH, zF + zD);
            v3 = vec3(xL, yB, zF + zD);
            break;
        case 2: // XZ plane (Y fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL + xW, yB, zF);
            v2 = vec3(xL + xW, yB, zF + zD);
            v3 = vec3(xL, yB, zF + zD);
            break;
    }

    // Draw filled quad
    gl::begin(GL_QUADS);
    gl::vertex(v0);
    gl::vertex(v1);
    gl::vertex(v2);
    gl::vertex(v3);
    gl::end();
}

void GraphicsRenderer::drawPlaneWireframe(float xL, float yB, float zF, float xW, float yH, float zD, int planeType, const ColorA& color) {
    gl::ScopedColor scopedColor(color);

    vec3 v0, v1, v2, v3;

    switch (planeType) {
        case 0: // XY plane (Z fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL + xW, yB, zF);
            v2 = vec3(xL + xW, yB + yH, zF);
            v3 = vec3(xL, yB + yH, zF);
            break;
        case 1: // YZ plane (X fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL, yB + yH, zF);
            v2 = vec3(xL, yB + yH, zF + zD);
            v3 = vec3(xL, yB, zF + zD);
            break;
        case 2: // XZ plane (Y fixed)
            v0 = vec3(xL, yB, zF);
            v1 = vec3(xL + xW, yB, zF);
            v2 = vec3(xL + xW, yB, zF + zD);
            v3 = vec3(xL, yB, zF + zD);
            break;
    }

    // Draw wireframe rectangle
    gl::drawLine(v0, v1);
    gl::drawLine(v1, v2);
    gl::drawLine(v2, v3);
    gl::drawLine(v3, v0);
}




