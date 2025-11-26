/*
 *  codepanel.h
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

#ifndef CODEPANEL_H
#define CODEPANEL_H

#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include <vector>

using namespace ci;
using namespace std;

class CodePanel {
public:
	CodePanel();
	void createTexture();
	void update(vec2 windowSize);
	void render(vec2 windowSize);
	void addLine(string line);
    void putLine(int index, string line);
	void bind();
	void unbind();
	void setCodeColor(float r, float g, float b);
	void setCodeFont(string fontName, int fontSize);

	vec2 loc;

	float opacity, max_opacity;
	bool show;
	int fadeTime, maxLines, counter;
	gl::TextureRef texture;

	vector<string> lines;

	string title;
	Color codeColor;
	string codeFontName;
	int codeFontSize;

private:
	void makeHeader();
};

#endif
