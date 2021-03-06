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

// Object to display the ship recognition manual
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_RECOGMANUAL_DISPLAY_H
#define SUB_RECOGMANUAL_DISPLAY_H

#include "user_display.h"
#include "datadirs.h"
#include "widget.h"
#include "ptrvector.h"

class sub_recogmanual_display : public user_display
{
protected:

	class widget_button_next : public widget_button 
	{
	protected:
		int direction;
		int& page;
	public:
		widget_button_next(int x, int y, int w, int h, int dir, int& att_page, const std::string& text_,  const std::string& bg_image_, widget* parent_ =0);
		void draw() const;
		void on_release ();		
	};
	
	int page;
	widget_button_next btn_left;
	widget_button_next btn_right;
	std::auto_ptr<image> background;
	ptrvector<image> silhouettes;
	ptrvector<std::string> classes;
	ptrvector<std::string> lengths;
	ptrvector<std::string> displacements;
	ptrvector<std::string> weapons;
	ptrvector<std::string> countries;
	
public:
	sub_recogmanual_display(class user_interface& ui_);

	virtual void display(class game& gm) const;
	virtual void process_input(class game& gm, const SDL_Event& event);

	void enter(bool is_day);
	void leave();

};
#endif
