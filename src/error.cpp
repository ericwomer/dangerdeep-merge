/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Danger from the Deep, standard error/exception
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "error.h"
#include <sstream>
#include <SDL.h>

#if defined(DEBUG) && defined(__GNUC__)

std::string error::str(const char* file, unsigned line)
{
	std::ostringstream oss;
	oss << ", in file: " << file << ", in line: " << line;
	return oss.str();
}
#endif



sdl_error::sdl_error(const std::string& msg)
	: error(std::string("SDL error: ") + msg + std::string(", SDL: ")
		+ SDL_GetError())
{
}
