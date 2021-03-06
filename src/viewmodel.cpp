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

// a model viewer
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DATADIR "./data/"
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "system.h"
#include "vector3.h"
#include "datadirs.h"
#include "font.h"
#include "model.h"
#include "texture.h"
#include "image.h"
#include "make_mesh.h"
#include "xml.h"
#include "filehelper.h"
#include "widget.h"
#include "objcache.h"
#include "log.h"
#include "primitives.h"
#include "cfg.h"
#include <glu.h>
#include <SDL.h>

#define VIEWMODEL

#include "mymain.cpp"

using std::vector;


// switch features on/off:

#undef  IS_INSIDE_TEST



int res_x, res_y;
font* font_arial = 0;
font *font_vtremington12 = 0;

vector4t<GLfloat> lposition(0,0,0,1);

#define LIGHT_ANG_PER_SEC 30

// Forward declaration
void view_model(const string& modelfilename, const string& datafilename);

string model_layout;

struct constraint {
	unsigned obj;
	float min;
	float max;
	bool increasing;
	float current;
};

class model_load_dialog
{
public:
        model_load_dialog();
        virtual ~model_load_dialog() {}

        void get_model_list(const list<string>& namelist);
        const list<string>& get_models();

        void load_menu();
        void load_model(widget_list *list);

private:
        struct model_entry
        {
                string name;
                string dir;
        };

        auto_ptr<widget::theme> theme;
        vector<model_entry> files;
        
        int selected_model;

        void message(const string& msg);
};



model_load_dialog::model_load_dialog() : 
        theme(0)
{
        theme.reset( new widget::theme("widgetelements_menu.png", "widgeticons_menu.png", font_vtremington12, color(182, 146, 137), color(222, 208, 195), color(92, 72, 68)) );
}



void model_load_dialog::get_model_list(const list<string>& namelist)
{
	for (list<string>::const_iterator it = namelist.begin(); it != namelist.end(); ++it) {
		model_entry new_model = { *it, data_file().get_path(*it) };
		files.push_back(new_model);
	}
}



void model_load_dialog::load_menu()
{
        widget_menu *wm;
        widget_text *title;
        widget_text *models_lbl;
        widget_list *models_list;
        
        widget w(0, 0, 1024, 768, "", 0, "threesubs.jpg");
        w.set_theme(theme);

        title = new widget_text(10, 10, 800, 80, "Danger from the Deep Viewmodel OpenGL Frontend.\nCopyright (C) 2003-2006 Thorsten Jordan.");
        w.add_child(title);

        models_lbl = new widget_text(300, 100, 424, 30, "Available Models:", 0, 1);
        w.add_child(models_lbl);

        models_list = new widget_list(300, 150, 424, 438, NULL);
        
        vector<model_entry>::iterator it = files.begin();
        for (; it != files.end(); ++it) {
                models_list->append_entry(it->name);
        }

        w.add_child(models_list);

        wm = new widget_menu(87, 650, 400, 40, "", true);

        wm->set_entry_spacing(50);

        wm->add_entry("Load", new widget_caller_arg_button<model_load_dialog, void (model_load_dialog::*)(widget_list*), widget_list*>(this, &model_load_dialog::load_model, models_list));

        wm->add_entry("Quit", new widget_caller_arg_button<widget, void (widget::*)(int), int>(&w, &widget::close, 0, 0, 0, 0, 0));

        w.add_child(wm);

        w.run(0,false);
}



void
model_load_dialog::load_model(widget_list* models)
{
        selected_model = models->get_selected();

        model_entry entry = files[selected_model];

        string data_filename = entry.dir + entry.name;
        string model_filename;

        try {
                xml_doc dataxml(data_filename);
                dataxml.load();
                xml_elem root = dataxml.first_child().child("classification");
                model_filename = root.attr("modelname");

                view_model(model_filename, data_filename);

        } catch (...) {
                message("Unable to read one of the files:\n\n"+data_filename+"\n"+model_filename);
        }
}



void
model_load_dialog::message(const string& msg)
{
        widget w(0,0,1024,768,"", 0, "threesubs.jpg");

        widget_text *msgtext = new widget_text(0, 0, 0, 0, msg, 0, 1);
        w.add_child(msgtext);

        widget_menu *wm = new widget_menu(112,120,200,40,"",true);
        wm->add_entry("OK",new widget_caller_arg_button<widget, void(widget::*)(int),int>(&w, &widget::close, 0, 0, 0, 0, 0));
        w.add_child(wm);
        
        w.run(0,false);
}



int scalelength(int i)
{
	return (i % 50 == 0) ? 5 : ((i % 10 == 0) ? 4 : ((i % 5 == 0) ? 2 : 1));
}


void view_model(const string& modelfilename, const string& datafilename)
{
	model* mdl = new model(/*get_model_dir() + */ modelfilename);
	mdl->register_layout(model_layout);
	mdl->set_layout(model_layout);

	mdl->write_to_dftd_model_file("test.xml");

//	mdl->get_mesh(0).write_off_file("test.off");

#ifdef IS_INSIDE_TEST	// Test hack for is_inside function
	const unsigned iizz = 6, iiyy = 10, iixx = 8;
	//fixme: maybe better use double resolution in every direction, then merge
	//the is inside (some anti aliasing to make it more accurate)
	std::vector<bool> ii(iizz*iiyy*iixx);
	unsigned total_in = 0;
	vector3f dimmin = mdl->get_min(), dimmax = mdl->get_max();
	vector3f dimlen = dimmax - dimmin;
	vector3f voxelsz(dimlen.x/iixx, dimlen.y/iiyy, dimlen.z/iizz);
	vector3f voxeloff = dimmin + voxelsz * 0.5f;
	vector3f voxeloff2 = voxeloff + vector3f(0.5f, 0.5f, 0.5f);
	std::cout << "voxelsize: " << voxelsz << " voxeloff " << voxeloff << "\n";
	for (unsigned izz = 0; izz < iizz; ++izz) {
		std::cout << "crosssection picture " << izz+1 << "/" << iizz << "\n";
		for (unsigned iyy = 0; iyy < iiyy; ++iyy) {
			for (unsigned ixx = 0; ixx < iixx; ++ixx) {
				bool is_in = 
					(mdl->is_inside(vector3f(voxelsz.x*ixx,
								 voxelsz.y*iyy,
								 voxelsz.z*izz) + voxeloff));
				is_in = is_in || (mdl->is_inside(vector3f(voxelsz.x*ixx,
								 voxelsz.y*iyy,
								 voxelsz.z*izz) + voxeloff2));
				total_in += is_in ? 1 : 0;
				ii[(izz * iiyy + iyy) * iixx + ixx] = is_in;
				std::cout << (is_in ? "X" : " ");
			}
			std::cout << "\n";
		}
	}
	std::cout << "Center of gravity of mesh#0: " << mdl->get_mesh(0).compute_center_of_gravity() << "\n";
	std::cout << "Inertia tensor of mesh#0:\n";
	mdl->get_mesh(0).compute_inertia_tensor().print();
	std::cout << "Inside: " << total_in << " of " << ii.size() << "\n";

	/* // test code
	unsigned t1 = sys().millisec();
	for (int i = 0; i < 1000; ++i)
		mdl->get_mesh(0).compute_center_of_gravity();
	unsigned t2 = sys().millisec();
	for (int i = 0; i < 1000; ++i)
		mdl->get_mesh(0).compute_inertia_tensor();
	unsigned t3 = sys().millisec();
	std::cout << "Time A = " << t2-t1 << " Time B = " << t3-t2 << "\n";
	*/

#endif

        // rendering mode

        int wireframe = 0;

        // Smoke variables

        bool smoke = false;
        int smoke_type = 0;
        vector3 smoke_pos;
        bool smoke_display = false;
        
        // Read xml file if supplied
        
        try {
                xml_doc dataxml(datafilename);
                dataxml.load();
                xml_elem smoke_elem = dataxml.first_child().child("smoke");
                smoke_type = smoke_elem.attri("type");
                float smokex = smoke_elem.attrf("x");
                float smokey = smoke_elem.attrf("y");
                float smokez = smoke_elem.attrf("z");

                smoke_pos = vector3(smokex, smokey, smokez);
                smoke = true;

        } catch (...) {
                smoke = false;
        }


/*
	// fixme test hack
	delete mdl;
	mdl = new model("test.xml", true, false);
	mdl->write_to_dftd_model_file("test2.xml");
	delete mdl;
	mdl = new model("test2.xml", true, false);
	mdl->write_to_dftd_model_file("test3.xml");
*/

	double sc = (mdl->get_boundbox_size()*0.5).length();
	vector3 viewangles;
	// place viewer along positive z-axis
	vector3 pos(0, 0, sc);
	bool lightmove = true;
	bool coordinatesystem = false;

	vector<Uint8> pixels(32*32*3, 64);
	for (unsigned i = 0; i < 32*32; ++i) { pixels[3*i+2] = (((i/32) + (i % 32)) & 1) * 127 + 128; }
	vector<Uint8> bumps(32*32);
	for (unsigned i = 0; i < 32*32; ++i) { bumps[i] = 0; } //(((i/32)%8)<7 && ((i%32)%8)<7)?255:0; }
	model::material::map* dmap = new model::material::map();
	model::material::map* bmap = new model::material::map();
	dmap->set_texture(new texture(pixels, 32, 32, GL_RGB, texture::NEAREST, texture::CLAMP));
	bmap->set_texture(new texture(bumps, 32, 32, GL_LUMINANCE, texture::LINEAR, texture::CLAMP, true));
	model::material* mat = new model::material();
	mat->specular = color::white();
	mat->colormap.reset(dmap);
	mat->normalmap.reset(bmap);
	mdl->add_material(mat);
	model::mesh* msh = make_mesh::cube(3*sc, 3*sc, 3*sc, 1.0f, 1.0f, false);
//	model::mesh* msh = make_mesh::sphere(1.5*sc, 3*sc, 16, 16, 1.0f, 1.0f, false);
//	model::mesh* msh = make_mesh::cylinder(1.5*sc, 3*sc, 16, 1.0f, 1.0f, true, false);
	msh->mymaterial = mat;
	//mdl->add_mesh(msh);
        msh->compile();
	mdl->compile();

	unsigned time1 = sys().millisec();
	double ang = 0;

	model::mesh* lightsphere = make_mesh::sphere(5.0f, 5.0f, 8, 8, 1, 1, true, "sun");
//	model::mesh* lightsphere = make_mesh::cube(5.0f, 5.0f, 5.0f, 1, 1, true, "sun");
	lightsphere->mymaterial = new model::material();
	lightsphere->mymaterial->diffuse = color(255, 255, 128);
	lightsphere->mymaterial->specular = color(255, 255, 128);
	lightsphere->compile();

	unsigned frames = 1;
	unsigned lastframes = 1;
	double fpstime = sys().millisec() / 1000.0;
	double totaltime = sys().millisec() / 1000.0;
	double measuretime = 5;	// seconds

	unsigned max_objects=0;
	while(mdl->object_exists(max_objects))
		++max_objects;

	log_info("Found " << (max_objects-1) << " objects");

	vector<constraint> constraints;

	for(unsigned ix=0;ix<max_objects;++ix) {
		vector2f mm=mdl->get_object_angle_constraints(ix);
		// get_object_translation_constraints
		constraint c = {ix,mm.x,mm.y,true,0};
		constraints.push_back(c);
	}

	float ang_delta = 0.0f;

	while (true) {
		// rotate light
		unsigned time2 = sys().millisec();
		if (lightmove && time2 > time1) {
			ang_delta = LIGHT_ANG_PER_SEC*(time2-time1)/1000.0;
			ang += ang_delta;
			if (ang > 360) ang -= 360;
			time1 = time2;
			lposition.x = 1.4*sc*cos(3.14159*ang/180);
			lposition.z = 1.4*sc*sin(3.14159*ang/180);
		} else {
			ang_delta = 0.0f;
		}

		// test: rotate objects
		for( vector<constraint>::iterator it=constraints.begin(); it!=constraints.end(); ++it){
			constraint& c=*it;
			if(c.max<c.current || c.min>c.current) 
				c.increasing=!c.increasing;
			if(c.increasing) {
				c.current+=ang_delta;
			} else {
				c.current-=ang_delta;
			}
			mdl->set_object_angle(c.obj,c.current);
		}
//		mdl->set_object_translation(4, ang*50/360);
	
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, 0);

		glLoadIdentity();
		glTranslated(-pos.x, -pos.y, -pos.z);
		glRotatef(viewangles.z, 0, 0, 1);
		glRotatef(viewangles.y, 0, 1, 0);
		glRotatef(viewangles.x, 1, 0, 0);

		glPushMatrix();
		glTranslatef(lposition.x, lposition.y, lposition.z);
		vector4t<GLfloat> null(0,0,0,1);
		glLightfv(GL_LIGHT0, GL_POSITION, &null.x);
		lightsphere->display();
		glPopMatrix();

		primitives::line(vector3f(0,0,0), vector3f(lposition.x,lposition.y,lposition.z), colorf(1, 1, 1)).render();

		primitives::line(vector3f(0,0,0), vector3f(1,0,0), colorf(1,0,0)).render();
		primitives::line(vector3f(0,0,0), vector3f(0,1,0), colorf(1,1,0)).render();
		primitives::line(vector3f(0,0,0), vector3f(0,0,1), colorf(0,1,0)).render();



		if (coordinatesystem) {
			glDisable(GL_LIGHTING);
			vector3f min = mdl->get_min();
			vector3f max = mdl->get_max();
			float h = max.z;
			float w = max.x;
			int c = 0;
			primitive<244> coord1(GL_LINES, colorf(1,0,0,1));
 			coord1.vertices[c++] = vector3f(-30, 0, h);
			coord1.vertices[c++] = vector3f(30, 0, h);
			coord1.vertices[c++] = vector3f(-30, 0, -h);
			coord1.vertices[c++] = vector3f(30, 0, -h);
			for (int i = 0; i <= 30; ++i) {
				int l = scalelength(i);
				coord1.vertices[c++] = vector3f(i, 0, h);
				coord1.vertices[c++] = vector3f(i, 0, h+l);
				coord1.vertices[c++] = vector3f(-i, 0, h);
				coord1.vertices[c++] = vector3f(-i, 0, h+l);
				coord1.vertices[c++] = vector3f(i, 0, -h);
				coord1.vertices[c++] = vector3f(i, 0, -h+l);
				coord1.vertices[c++] = vector3f(-i, 0, -h);
				coord1.vertices[c++] = vector3f(-i, 0, -h+l);
			}
			coord1.render();

			c = 0;
			primitive<1204> coord2(GL_LINES, colorf(0,1,0,1));
			coord2.vertices[c++] = vector3f(0, -150, h);
			coord2.vertices[c++] = vector3f(0, 150, h);
			coord2.vertices[c++] = vector3f(0, -150, -h);
			coord2.vertices[c++] = vector3f(0, 150, -h);
			for (int i = 0; i <= 150; ++i) {
				int l = scalelength(i);
				coord2.vertices[c++] = vector3f(0, i, h);
				coord2.vertices[c++] = vector3f(0, i, h+l);
				coord2.vertices[c++] = vector3f(0, -i, h);
				coord2.vertices[c++] = vector3f(0, -i, h+l);
				coord2.vertices[c++] = vector3f(0, i, -h);
				coord2.vertices[c++] = vector3f(0, i, -h+l);
				coord2.vertices[c++] = vector3f(0, -i, -h);
				coord2.vertices[c++] = vector3f(0, -i, -h+l);
			}
			coord2.render();

			c = 0;
			primitive<244> coord3(GL_LINES, colorf(1,1,0,1));
 			coord3.vertices[c++] = vector3f(w, 0, 30);
			coord3.vertices[c++] = vector3f(w, 0, -30);
			coord3.vertices[c++] = vector3f(-w, 0, 30);
			coord3.vertices[c++] = vector3f(-w, 0, -30);
			for (int i = 0; i <= 30; ++i) {
				int l = scalelength(i);
				coord3.vertices[c++] = vector3f(w, 0, i);
				coord3.vertices[c++] = vector3f(w+l, 0, i);
				coord3.vertices[c++] = vector3f(w, 0, -i);
				coord3.vertices[c++] = vector3f(w+l, 0, -i);
				coord3.vertices[c++] = vector3f(-w, 0, i);
				coord3.vertices[c++] = vector3f(-w+l, 0, i);
				coord3.vertices[c++] = vector3f(-w, 0, -i);
				coord3.vertices[c++] = vector3f(-w+l, 0, -i);
			}
			coord3.render();

			glEnable(GL_LIGHTING);
		}

                msh->display();

                if (wireframe == 1) {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }

		matrix4 mvp = matrix4::get_gl(GL_PROJECTION_MATRIX) * matrix4::get_gl(GL_MODELVIEW_MATRIX);
		mdl->display();
		// test
		//mdl->display_mirror_clip();

                // display smoke origin
                if (smoke && smoke_display) {
                        glDisable(GL_LIGHTING);
                        glDisable(GL_DEPTH_TEST);

						primitive<6> origin(GL_LINES, colorf(1,0,0));
 						origin.vertices[0] = vector3f(smoke_pos.x-1, smoke_pos.y, smoke_pos.z);
						origin.vertices[1] = vector3f(smoke_pos.x+1, smoke_pos.y, smoke_pos.z);
						origin.vertices[2] = vector3f(smoke_pos.x, smoke_pos.y-1, smoke_pos.z);
						origin.vertices[3] = vector3f(smoke_pos.x, smoke_pos.y+1, smoke_pos.z);
						origin.vertices[4] = vector3f(smoke_pos.x, smoke_pos.y, smoke_pos.z-1);
						origin.vertices[5] = vector3f(smoke_pos.x, smoke_pos.y, smoke_pos.z+1);
						origin.render();

                        glEnable(GL_DEPTH_TEST);
                        glEnable(GL_LIGHTING);
                }

                if (wireframe == 1) {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }

		// draw scales if requested
		if (coordinatesystem) {
			glColor4f(1,1,1,1);
		}

		list<SDL_Event> events = sys().poll_event_queue();
		for (list<SDL_Event>::iterator it = events.begin(); it != events.end(); ++it) {
			SDL_Event& event = *it;
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					return;
				case SDLK_KP4: pos.x -= 1.0; break;
				case SDLK_KP6: pos.x += 1.0; break;
				case SDLK_KP8: pos.y -= 1.0; break;
				case SDLK_KP2: pos.y += 1.0; break;
				case SDLK_KP1: pos.z -= 1.0; break;
				case SDLK_KP3: pos.z += 1.0; break;
				case SDLK_l: lightmove = !lightmove; break;
				case SDLK_s:
					{
                                                // Allow user to save smoke position
                                                if (smoke && (SDL_GetModState() & (KMOD_LCTRL | KMOD_RCTRL))) {
                                                        try {
                                                                xml_doc dataxml(datafilename);
                                                                dataxml.load();
                                                                xml_elem smoke_elem = dataxml.first_child().child("smoke");
                                                        
                                                                smoke_elem.set_attr(smoke_pos.x, "x");
                                                                smoke_elem.set_attr(smoke_pos.y, "y");
                                                                smoke_elem.set_attr(smoke_pos.z, "z");

                                                                dataxml.save();
                                                        } catch (...) { std::cout << "unable to save smoke origin" << "\n"; }
                                                        
                                                } else {
                                                        pair<model::mesh*, model::mesh*> parts = mdl->get_mesh(0).split(vector3f(0,1,0), -1);
                                                        parts.first->transform(matrix4f::trans(0, 30, 50));
                                                        parts.second->transform(matrix4f::trans(0, -30, 50));
                                                        mdl->add_mesh(parts.first);
                                                        mdl->add_mesh(parts.second);
                                                }
					}
					break;
				case SDLK_c: coordinatesystem = !coordinatesystem; break;
                                case SDLK_p: smoke_display = !smoke_display; break;
                                case SDLK_w: if (++wireframe>1) wireframe = 0; break;
				default: break;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				vector2 motion = sys().translate_motion(event) * 0.5;
				if (event.motion.state & SDL_BUTTON_LMASK) {
					viewangles.y += motion.x;
					viewangles.x += motion.y;
				} else if (event.motion.state & SDL_BUTTON_RMASK) {
					viewangles.z += motion.x;
					viewangles.x += motion.y;
				} else if (event.motion.state & SDL_BUTTON_MMASK) {
					pos.x += motion.x;
					pos.y += motion.y;
				}
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
                                // Check if x,,y,z are pressed and if so mouse wheel moves smoke position by delta.
                                // delta is either 1.0 if a shift key is pressed or 0.1 otherwise.
                                Uint8 *keys = SDL_GetKeyState(NULL);
                                float delta = (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT))? 1.000 : 0.100; 

				if (event.button.button == SDL_BUTTON_WHEELUP) {
                                        if (keys[SDLK_x]) {
                                                smoke_pos.x += delta;
                                        } else if (keys[SDLK_y]) {
                                                smoke_pos.y += delta;
                                        } else if (keys[SDLK_z]) {
                                                smoke_pos.z += delta;
                                        } else {
                                                pos.z -= 2;
                                        }
				} else if (event.button.button == SDL_BUTTON_WHEELDOWN) {
                                        if (keys[SDLK_x]) {
                                                smoke_pos.x -= delta;
                                        } else if (keys[SDLK_y]) {
                                                smoke_pos.y -= delta;
                                        } else if (keys[SDLK_z]) {
                                                smoke_pos.z -= delta;
                                        } else {
                                                pos.z += 2;
                                        }
				}
                                if (smoke_pos.x < 0.00001 && smoke_pos.x > -0.00001)
                                        smoke_pos.x = 0;
                                if (smoke_pos.y < 0.00001 && smoke_pos.y > -0.00001)
                                        smoke_pos.y = 0;
                                if (smoke_pos.z < 0.00001 && smoke_pos.z > -0.00001)
                                        smoke_pos.z = 0;
			}
		}
		sys().prepare_2d_drawing();
		vector3f minp = mdl->get_min(), maxp = mdl->get_max();
		ostringstream os;
		os << "A simple model viewer for Danger from the Deep.\n"
			<< "Press any key to exit.\n"
			<< "Press left mouse button and move mouse to rotate x/y.\n"
			<< "Press right mouse button and move mouse to rotate z.\n"
                        << "Press 'p' to toggle smoke origin.\n"
                        << "Press 'w' to toggle wireframe mode.\n"
			<< "Rotation " << viewangles.x << ", " << viewangles.y << ", " << viewangles.z <<
			"\nTranslation " << pos.x << ", " << pos.y << ", " << pos.z <<
			"\nmin " << minp.x << ", " << minp.y << ", " << minp.z <<
			"\nmax " << maxp.x << ", " << maxp.y << ", " << maxp.z <<
			"\nbounding sphere radius=" << mdl->get_bounding_sphere_radius() <<
			"\n";
		
                if (smoke) {	
                        os << "Smoke: " << ((smoke_display)? "On" : "Off") << "\n";
                        os << "Smoke origin " << smoke_pos.x << ", " << smoke_pos.y << ", " << smoke_pos.z << "\n";
                } else {
                        os << "Smoke: Off (no info found).\n";
                }
                        
		font_arial->print(0, 0, os.str());

		// print scale descriptions
		if (coordinatesystem) {
			matrix4 xf = matrix4::trans(res_x/2, res_y/2, 0) * matrix4::diagonal(res_x/2, -res_y/2, 1) * mvp;
			vector3f min = mdl->get_min();
			vector3f max = mdl->get_max();
			float h = max.z;
			float w = max.x;
			
			glColor4f(1,0,0,1);
			for (int i = 0; i <= 30; i += 5) {
				vector3 a(i, 0, h), b(i, 0, -h);
				vector2 c = (xf * a).xy();
				vector2 d = (xf * b).xy();
				char tmp[20];
				sprintf(tmp, "%i", i);
				// 2006-12-01 doc1972 print() needs int pixel coords, so we cast
				font_arial->print((int)c.x, (int)c.y, tmp);
				font_arial->print((int)d.x, (int)d.y, tmp);
			}
			glColor4f(0,1,0,1);
			for (int i = 0; i <= 150; i += 5) {
				vector3 a(0, i, h), b(0, i, -h);
				vector2 c = (xf * a).xy();
				vector2 d = (xf * b).xy();
				char tmp[20];
				sprintf(tmp, "%i", i);
				// 2006-12-01 doc1972 print() needs int pixel coords, so we cast
				font_arial->print((int)c.x, (int)c.y, tmp);
				font_arial->print((int)d.x, (int)d.y, tmp);
			}
			glColor4f(1,1,0,1);
			for (int i = 0; i <= 30; i += 5) {
				vector3 a(w, 0, i), b(-w, 0, i);
				vector2 c = (xf * a).xy();
				vector2 d = (xf * b).xy();
				char tmp[20];
				sprintf(tmp, "%i", i);
				// 2006-12-01 doc1972 print() needs int pixel coords, so we cast
				font_arial->print((int)c.x, (int)c.y, tmp);
				font_arial->print((int)d.x, (int)d.y, tmp);
			}
		}

		sys().unprepare_2d_drawing();

		// record fps
		++frames;
		totaltime = sys().millisec() / 1000.0;
		if (totaltime - fpstime >= measuretime) {
			fpstime = totaltime;
			log_info("fps " << (frames - lastframes)/measuretime);
			lastframes = frames;
		}

		sys().swap_buffers();
	}

        delete msh;
	delete mdl;
}



void run_gui()
{
        model_load_dialog ml;

	// fixme: use data_file() 's lists here, no parsing of directories
        ml.get_model_list(data_file().get_ship_list());
        ml.get_model_list(data_file().get_submarine_list());
        ml.get_model_list(data_file().get_airplane_list());

        ml.load_menu();
}



int mymain(list<string>& args)
{
	// command line argument parsing
	res_x = 1024;
	bool fullscreen = true;
        bool use_gui = false;

	string modelfilename;
        string datafilename;
	model_layout = model::default_layout;
	/* default fps */
	unsigned maxfps = 60;

	for (list<string>::iterator it = args.begin(); it != args.end(); ++it) {
		if (*it == "--help") {
			cout << "DftD viewmodel, usage:\n--help\t\tshow this\n"
			     << "--res n\t\tuse resolution n horizontal,\n\t\tn is 512,640,800,1024 (recommended) or 1280\n"
			     << "--nofullscreen\tdon't use fullscreen\n"
			     << "--layout layoutname\tuse layout with specific name for skins\n"
			     << "--maxfps x\tset maximum fps to x frames per second (default 60). Use x=0 to disable fps limit.\n"
				 << "--gui starts viewmodel in GUI mode, with model list.\n"
			     << "MODELFILENAME\n";
			return 0;
		} else if (*it == "--nofullscreen") {
			fullscreen = false;
		} else if (*it == "--res") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				int r = atoi(it2->c_str());
				if (r==512||r==640||r==800||r==1024||r==1280)
					res_x = r;
				++it;
			}
                } else if (*it == "--dataxml") {
                        if (++it != args.end()) {
                                datafilename = it->c_str();
                        } else --it;
                } else if (*it == "--layout") {
                        if (++it != args.end()) {
                                model_layout = it->c_str();
                        } else --it;
                } else if (*it == "--gui") {
                        use_gui = true;
		} else if (*it == "--maxfps") {
			list<string>::iterator it2 = it; ++it2;
			if (it2 != args.end()) {
				int mf = atoi(it2->c_str());
				if (mf >= 0) maxfps = unsigned(mf);
				++it;
			}
		} else {
			modelfilename = *it;
		}
	}

	res_y = res_x*3/4;

	// parse configuration
	cfg& mycfg = cfg::instance();
	mycfg.register_option("screen_res_x", 1024);
	mycfg.register_option("screen_res_y", 768);
	mycfg.register_option("fullscreen", true);
	mycfg.register_option("debug", false);
	mycfg.register_option("sound", true);
	mycfg.register_option("use_hqsfx", true);
	mycfg.register_option("use_ani_filtering", false);
	mycfg.register_option("anisotropic_level", 1.0f);
	mycfg.register_option("use_compressed_textures", false);
	mycfg.register_option("multisampling_level", 0);
	mycfg.register_option("use_multisampling", false);
	mycfg.register_option("bloom_enabled", false);
	mycfg.register_option("hdr_enabled", false);
	mycfg.register_option("hint_multisampling", 0);
	mycfg.register_option("hint_fog", 0);
	mycfg.register_option("hint_mipmap", 0);
	mycfg.register_option("hint_texture_compression", 0);
	mycfg.register_option("vsync", false);
	mycfg.register_option("water_detail", 128);
	mycfg.register_option("wave_fft_res", 128);
	mycfg.register_option("wave_phases", 256);
	mycfg.register_option("wavetile_length", 256.0f);
	mycfg.register_option("wave_tidecycle_time", 10.24f);
	mycfg.register_option("usex86sse", true);
	mycfg.register_option("language", 0);
	mycfg.register_option("cpucores", 1);
	mycfg.register_option("terrain_texture_resolution", 0.1f);

	system::parameters params(1.0, 1000.0, res_x, res_y, fullscreen);
	system::create_instance(new class system(params));
	sys().set_res_2d(1024, 768);
	sys().set_max_fps(maxfps);
	
	log_info("A simple model viewer for DftD-.mdl files");
	log_info("copyright and written 2003 by Thorsten Jordan");

	GLfloat lambient[4] = {0.1,0.1,0.09,1};
	GLfloat ldiffuse[4] = {1,1,0.9,1};
	GLfloat lspecular[4] = {1,1,0.9,1};
	glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lspecular);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	font_arial = new font(get_font_dir() + "font_arial");
        font_vtremington12 = new font(get_font_dir() + "font_vtremington12");
        
	sys().draw_console_with(font_arial, 0);

	objcachet<class image> imagecache(get_image_dir());
	widget::set_image_cache(&imagecache);

        if (use_gui) {
                run_gui();
        } else {
		view_model(modelfilename,datafilename);
        }

	delete font_arial;
        delete font_vtremington12;

	system::destroy_instance();

	return 0;
}
