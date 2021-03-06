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

// xml access interface
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef XML_H
#define XML_H

#include <string>
#include "error.h"
#include "vector3.h"
#include "quaternion.h"
#include "angle.h"

class TiXmlElement;
class TiXmlDocument;


///\brief General exception for an error while using the XML interface
class xml_error : public error
{
 public:
	xml_error(const std::string& name, const std::string& fn)
		: error(std::string("xml error: ") + name + std::string(", file: ") + fn) {}
};



///\brief XML element specific exception
class xml_elem_error : public xml_error
{
 public:
	xml_elem_error(const std::string& name, const std::string& fn)
		: xml_error(std::string("failed to get element ") + name, fn) {}
};



///\brief A XML element representation with interface for handling of elements like adding or requesting children or data.
class xml_elem
{
 private:
	xml_elem();
 protected:
	TiXmlElement* elem;
	xml_elem(TiXmlElement* e) : elem(e) {}

	friend class xml_doc;
 public:
	bool has_attr(const std::string& name = "value") const;
	std::string attr(const std::string& name = "value") const;
	int attri(const std::string& name = "value") const;
	unsigned attru(const std::string& name = "value") const;
	double attrf(const std::string& name = "value") const;
	vector3 attrv3() const;
	vector2 attrv2() const;
	quaternion attrq() const;
	angle attra() const;
	bool attrb(const std::string& name = "value") const;
	xml_elem child(const std::string& name) const;
	bool has_child(const std::string& name) const;
	xml_elem add_child(const std::string& name);
	void set_attr(const std::string& val, const std::string& name = "value");
	void set_attr(unsigned u, const std::string& name = "value");
	void set_attr(int i, const std::string& name = "value");
	void set_attr(double f, const std::string& name = "value");
	void set_attr(const vector3& v);
	void set_attr(const vector2& v);
	void set_attr(const quaternion& q);
	void set_attr(angle a);
	void set_attr(bool b, const std::string& name = "value");
	std::string get_name() const;
	void add_child_text(const std::string& txt); // add text child
	std::string child_text() const;	// returns value of text child, throws error if there is none

	// get name of document
	std::string doc_name() const;

	class iterator {
	private:
		iterator();
	protected:
		const xml_elem& parent;
		TiXmlElement* e;
		bool samename;	// iterate over any children or only over children with same name
		iterator(const xml_elem& parent_, TiXmlElement* elem_ = 0, bool samename_ = true)
			: parent(parent_), e(elem_), samename(samename_) {}

		friend class xml_elem;
	public:
		xml_elem elem() const;
		void next();
		bool end() const { return e == 0; }
	};
	friend class iterator;

	// iterate this way: for (iterator it = e.iterate("name"); !it.end(); it.next()) { ... }
	iterator iterate(const std::string& childname) const;
	// iterate over any child
	iterator iterate() const;
};



///\brief A XML document representation with interface for handling of documents.
class xml_doc
{
 private:
	xml_doc();
	xml_doc(const xml_doc& );
	xml_doc& operator= (const xml_doc& );
 protected:
	// can't use auto_ptr here because TiXmlDocument is not yet defined.
	class TiXmlDocument* doc;
 public:
	xml_doc(std::string fn);
	~xml_doc();
	void load();
	void save();
	xml_elem first_child();
	xml_elem child(const std::string& name);
	xml_elem add_child(const std::string& name);
	std::string get_filename() const;
};

#endif // XML_H
