/*
 *  codepanel.cpp
 *  kv
 *
 *  Code panel for live coding text display
 *
 *	This file is part of kv.
 *
 *	kv is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.

 *	kv is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.

 *	You should have received a copy of the GNU General Public License
 *	along with kv.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "codepanel.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "util.h"

using namespace ci;

CodePanel::CodePanel()
{
	show = false;
	opacity	= 0.0f;
    max_opacity = 0.8f;
	fadeTime = 150;
	maxLines = 67;
	counter = 0;
	title = "sc.code";
}

void CodePanel::createTexture()
{
	TextLayout layout;

	// Add title with different styling
	if (!title.empty()) {
		layout.setFont(Font("Courier-Bold", 13));
		layout.setColor(Color(1.0f, 0.08f, 0.58f));  // Pink/magenta title
		layout.addLine(title);
		layout.addLine("--------------------------------------------------------");
	}

	// Add code lines
	layout.setFont(Font("Courier", 11));
	layout.setColor(Color(0.8f, 0.8f, 0.8f));

	for (int i = lines.size() - 1; i >= 0; i--) {
		layout.addLine(lines[i]);
	}

	Surface8u rendered = layout.render(true);
	texture = gl::Texture::create(rendered);
}

void CodePanel::update(vec2 windowSize)
{
	if (show) {
		if (counter == fadeTime) {
			show = false;
		}
		else {
			opacity = glm::clamp(opacity + 0.2f, 0.0f, max_opacity);
			counter++;
		}
	} else {
		opacity = glm::clamp(opacity - 0.05f, 0.0f, max_opacity);
	}

	if (opacity > 0.05f) {
		render(windowSize);
	}
}

void CodePanel::render(vec2 windowSize)
{
	createTexture();
	if (texture) {
		float x = windowSize.x - texture->getWidth() - 40.0f;
		float y = windowSize.y - texture->getHeight() - 25.0f;
		gl::color(1.0f, 1.0f, 1.0f, opacity);
		gl::draw(texture, vec2(x, y));
	}
}

void CodePanel::bind()
{
	if (show) {
		if (counter == fadeTime) {
			show = false;
		}
		else {
			opacity = glm::clamp(opacity + 0.2f, 0.0f, max_opacity);
			counter++;
		}
	} else {
		opacity = glm::clamp(opacity - 0.05f, 0.0f, max_opacity);
	}

	createTexture();

	if (texture) {
		texture->bind();
	}
}

void CodePanel::unbind()
{
	if (texture) {
		texture->unbind();
	}
}

void CodePanel::addLine(string line) {

	show = true;
	counter = 0;

	vector<string>::iterator it;

	if (lines.size() >= maxLines) {
		lines.pop_back();
	}

	it = lines.begin();
	lines.insert(it, line);
}

void CodePanel::putLine(int index, string line) {
    show = true;
    counter = 0;

    if (index >= 0 && index < lines.size()) {
        lines[index] = line;
    }
}

void CodePanel::makeHeader() {
	// Reserved for future use
}
