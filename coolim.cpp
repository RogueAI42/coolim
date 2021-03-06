// Copyright 2017 Declan Hoare
// This file is part of CoolIm.
//
// CoolIm is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CoolIm is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CoolIm.  If not, see <http://www.gnu.org/licenses/>.

#include <string>
#include <sstream>

#include <boost/interprocess/streams/bufferstream.hpp>
#include <boost/algorithm/string.hpp>

#include <cpptk.h>
#include <Magick++.h>

#define BUFFERSIZE 16777216
#include <b64/encode.h>

using namespace Tk;

Magick::Image orig;
Magick::Geometry currsize;
bool loaded = false;

void updateLabel(void)
{
	if (loaded)
	{
		Magick::Image conv(orig);
		
		// Scale the image for the display area.
		// ImageMagick will figure out the correct aspect ratio.
		Magick::Geometry resize((int) winfo(width, ".label"), (int) winfo(height, ".label"));
		if (resize != currsize)
		{
			conv.sample(resize);
			
			// Encode in Base64.
			Magick::Blob imdata;
			conv.write(&imdata);
			base64::encoder enc;
			boost::interprocess::bufferstream instr((char*) imdata.data(), imdata.length());
			std::stringstream outstr;
			enc.encode(instr, outstr);
			std::string newdata = outstr.str();
			boost::erase_all(newdata, "\n");
			
			// Put the data onto the Tk PhotoImage.
			".photo" << configure() -width(conv.columns()) -height(conv.rows());
			".photo" << put(newdata);
			currsize = resize;
		}
	}
}

void loadFile(std::string filename)
{
	orig.read(filename);
	orig.magick("PNG");
	orig.defineValue("PNG", "compression-level", "0");
	loaded = true;
	wm(title, ".", filename + " - CoolIm");
	currsize = Magick::Geometry();
	updateLabel();
}

void openImage(void)
{
	// TODO: add filename filters
	std::string filename = tk_getOpenFile();
	if (!filename.empty())
		loadFile(filename);
}

int main(int argc, char** argv)
{
	Tk::init(argv[0]);
	Magick::InitializeMagick(argv[0]);
	
	wm(title, ".", "CoolIm");
	wm(geometry, ".", "800x600");
	bind(".", "<Configure>", updateLabel);
	frame(".mbar") -borderwidth(1) -relief(raised);
	pack(".mbar") -fill(x);
	
	menubutton(".mbar.file") -text("File") -submenu(".mbar.file.m");
	pack(".mbar.file") -side(left);
	
	menu(".mbar.file.m");
	".mbar.file.m" << add(command) -menulabel("Open") -command(openImage);
	".mbar.file.m" << add(command) -menulabel("Exit") -command(std::string("exit"));
	
	label(".label");
	pack(".label") -fill(both) -expand(true);
	
	images(create, photo, ".photo");
	".label" << configure() -image(".photo");
	
	if (argc > 1)
		loadFile(argv[1]);
	
	Tk::runEventLoop();
	return 0;
}
