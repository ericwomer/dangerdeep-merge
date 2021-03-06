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

// simple height gen for simple map

#ifndef HEIGHT_GENERATOR_MAP_H
#define HEIGHT_GENERATOR_MAP_H

#include "height_generator.h"
#include "bivector.h"

class height_generator_map : public height_generator
{
	const unsigned subdivision_steps;
	bivector<float> height_data;
	vector2 realoffset;
	double realwidth, realheight;
	unsigned mapw, maph;
	double pixelw_real;
	vector2i mapoff;
	random_generator rndgen;
	std::vector<uint8_t> ct[8];
	unsigned cw, ch;
	bivector<float> noisemaps[7+3];
public:
	height_generator_map(const std::string& filename);

	void compute_heights(int detail, const vector2i& coord_bl,
			     const vector2i& coord_sz, float* dest, unsigned stride = 0,
			     unsigned line_stride = 0, bool noise = true);

	void compute_colors(int detail, const vector2i& coord_bl,
			    const vector2i& coord_sz, Uint8* dest);

	void get_min_max_height(double& minh, double& maxh) const;

protected:
	bivector<float> generate_patch(int detail, const vector2i& coord_bl,
				       const vector2i& coord_sz);

	void gen_col(float h, Uint8* c);
};

#endif
