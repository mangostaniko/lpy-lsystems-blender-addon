/* ---------------------------------------------------------------------------
 #
 #       L-Py: L-systems in Python
 #
 #       Copyright 2003-2008 UMR Cirad/Inria/Inra Dap - Virtual Plant Team
 #
 #       File author(s): F. Boudon (frederic.boudon@cirad.fr)
 #
 # ---------------------------------------------------------------------------
 #
 #                      GNU General Public Licence
 #
 #       This program is free software; you can redistribute it and/or
 #       modify it under the terms of the GNU General Public License as
 #       published by the Free Software Foundation; either version 2 of
 #       the License, or (at your option) any later version.
 #
 #       This program is distributed in the hope that it will be useful,
 #       but WITHOUT ANY WARRANTY; without even the implied warranty of
 #       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
 #       GNU General Public License for more details.
 #
 #       You should have received a copy of the GNU General Public
 #       License along with this program; see the file COPYING. If not,
 #       write to the Free Software Foundation, Inc., 59
 #       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 #
 # ---------------------------------------------------------------------------
 */

#pragma once

#include "lpy_config.h"
#include <string>
#include <memory>
#include <vector>

LPY_BEGIN_NAMESPACE

/*---------------------------------------------------------------------------*/

class LPY_API LsysOptionValue {
public:
	LsysOptionValue(const std::string& name, 
					const std::string& comment = "");
	virtual ~LsysOptionValue();

	std::string name;
	std::string comment;

	virtual void activate() = 0;
};

/*---------------------------------------------------------------------------*/

typedef void (*DirectSlot)();
class LsysOptionDirectSlotValue : public LsysOptionValue {
public:
	LsysOptionDirectSlotValue(const std::string& name, 
							  DirectSlot slot, 
							  const std::string& comment = ""):
		LsysOptionValue(name, comment), m_slot(slot) {}
		
	DirectSlot m_slot;

	virtual void activate() {  (*m_slot)();  }
};

/*---------------------------------------------------------------------------*/

template<class U>
class LsysOptionSlotValue : public LsysOptionValue {
public:

	typedef void (* Slot)(U);
	LsysOptionSlotValue(const std::string& name,
					Slot slot, U argument , 
					const std::string& comment = ""):
		LsysOptionValue(name, comment),
			m_slot(slot),
			m_argument(argument){
		}
		
	Slot m_slot;
	U m_argument;

	virtual void activate() {  (*m_slot)(m_argument);  }
};

/*---------------------------------------------------------------------------*/

template<class T, class U>
class LsysOptionClassSlotValue : public LsysOptionValue {
public:
	typedef void (T::* Slot)(U);
	LsysOptionClassSlotValue(const std::string& name,
					T * obj,
					Slot slot, 
					U argument , 
					const std::string& comment = ""):
		LsysOptionValue(name, comment),
			m_obj(obj),
			m_slot(slot),
			m_argument(argument){
		}
		
	T * m_obj;
	Slot m_slot;
	U m_argument;

	virtual void activate() {  (m_obj->*m_slot)(m_argument);  }
};

/*---------------------------------------------------------------------------*/

class LPY_API LsysOption {
public:
	typedef LsysOptionValue element_type;
	typedef std::vector<LsysOption::element_type *> OptionValueList;
	typedef OptionValueList::iterator iterator;
	typedef OptionValueList::const_iterator const_iterator;

	iterator begin() { return m_optionvalues.begin(); }
	const_iterator begin() const { return m_optionvalues.begin(); }
	iterator end() { return m_optionvalues.end(); }
	const_iterator end() const { return m_optionvalues.end(); }
	size_t size() const { return m_optionvalues.size(); }
	bool empty() const { return m_optionvalues.empty(); }

	LsysOption(const std::string& name, 
			   const std::string& comment = "",
			   const std::string& category = "");

	~LsysOption();

	void addValue(const std::string& name, 
				  DirectSlot slot,  
				  const std::string& comment = "")
	{ m_optionvalues.push_back(new LsysOptionDirectSlotValue(name,slot,comment)); }

	template <class U>
	void addValue(const std::string& name, 
				  void (* slot)(U),
				  U argument,
				  const std::string& comment = "")
	{ m_optionvalues.push_back(new LsysOptionSlotValue<U>(name,slot,argument,comment)); }

	template <class T, class U>
	void addValue(const std::string& name, 
				  T * object,
				  void (T::* slot)(U),
				  U argument,
				  const std::string& comment = "")
	{ m_optionvalues.push_back(new LsysOptionClassSlotValue<T,U>(name,object,slot,argument,comment)); }

	bool activate(const std::string&);
	bool activateSelection();
	bool setSelection(const std::string& value);
	bool setSelection(size_t valueid);

	size_t getCurrentValueId() const { return current_value_id; }
	void setCurrentValueId(size_t);

	const std::string& currentValue() const { return m_optionvalues[current_value_id]->name;}

	size_t defaultValueId() const { return default_value_id; }
	bool isToDefault() const { return current_value_id == default_value_id; }
	void setDefault(size_t value) { current_value_id = value; default_value_id = value; }

	bool isGlobal() const { return global; }
	void setGlobal(bool value) { global = value; }

	bool isEnabled() const { return enabled; }
	void setEnabled(bool value) { enabled = value; }

	std::string name;
	std::string comment;
	std::string category;
	size_t default_value_id;
	bool global;
	bool enabled;
	OptionValueList m_optionvalues;

	typedef void(*UpdateSlot)(size_t value);
	void connectTo(UpdateSlot);
protected:
	size_t current_value_id;
	void valueChanged(size_t value);

	typedef std::vector<UpdateSlot> UpdateSlotList;
	UpdateSlotList m_connectedSlots;
};

/*---------------------------------------------------------------------------*/

class LPY_API LsysOptions {
public:
	typedef LsysOption element_type;
	typedef std::vector<LsysOptions::element_type *>  OptionList;
	typedef OptionList::iterator iterator;
	typedef OptionList::const_iterator const_iterator;

	iterator begin() { return m_options.begin(); }
	const_iterator begin() const { return m_options.begin(); }
	iterator end() { return m_options.end(); }
	const_iterator end() const { return m_options.end(); }
	size_t size() const { return m_options.size(); }
	bool empty() const { return m_options.empty(); }

	LsysOptions();
	~LsysOptions();

	LsysOption* add(const std::string& name, const std::string& comment = "", const std::string& category = "");
	bool activate(const std::string& option, const std::string& value);
	bool activateSelection(const std::string& option);
	bool setSelection(const std::string& option, const std::string& value);
	bool setSelection(const std::string& option, size_t valueid);
	LsysOption* find(const std::string& name);

protected:
	OptionList m_options;
};

/*---------------------------------------------------------------------------*/

LPY_END_NAMESPACE
