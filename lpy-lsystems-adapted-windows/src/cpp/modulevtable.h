#pragma once

#include "error.h"
#include "../plantgl/tool/util_hashmap.h"
#include "../plantgl/tool/util_hashset.h"
#include "../plantgl/tool/rcobject.h"
#include <boost/python.hpp>
#include <vector>

LPY_BEGIN_NAMESPACE

/*---------------------------------------------------------------------------*/

class LPY_API BaseModuleProperty : public TOOLS(RefCountObject) {
public:
	BaseModuleProperty(const std::string& _name);
	virtual ~BaseModuleProperty();
	virtual bool isPythonProperty() const { return false; }
	std::string name;
};

typedef RCPtr<BaseModuleProperty> ModulePropertyPtr;

template<class Type>
class LPY_API ModuleProperty : public BaseModuleProperty {
public:
	typedef Type element_type;

	ModuleProperty(const std::string& name, element_type _value) : 
		BaseModuleProperty(name), value(_value) {}

    element_type value;
  
};

class LPY_API ModulePyProperty : public ModuleProperty<boost::python::object> {
public:
	ModulePyProperty(const std::string& name, boost::python::object _value) : 
		ModuleProperty< boost::python::object > ( name, _value) {}

	virtual bool isPythonProperty() const { return true; }
};

typedef RCPtr<ModulePyProperty> ModulePyPropertyPtr;

/*---------------------------------------------------------------------------*/

class ModuleClass;
typedef RCPtr<ModuleClass> ModuleClassPtr;
typedef std::vector<ModuleClassPtr> ModuleClassList;


/// Module Virtual Table
class LPY_API ModuleVTable : public TOOLS(RefCountObject) {
public:
	typedef pgl_hash_map_string<ModulePropertyPtr> PropertyMap;
	friend class ModuleClass;
	
	ModuleVTable(ModuleClassPtr owner = ModuleClassPtr(), ModuleClassPtr base = ModuleClassPtr());
	~ModuleVTable();

	ModulePropertyPtr getProperty(const std::string& name) const;
	void setProperty(ModulePropertyPtr prop);
	bool removeProperty(const std::string& name);

	int scale;

	inline ModuleClassPtr getModuleClass() const { return m_owner; }
	inline void setModuleClass(ModuleClassPtr mclass) { m_owner = mclass; }

	inline ModuleClassList getBases() const { 
		ModuleClassList bases;
		for(ModuleClassDirectPtrList::const_iterator it = m_modulebases.begin(); it != m_modulebases.end(); ++it)
			bases.push_back(*it);
		return bases; 
	}

	inline bool hasBaseClasses() const { return !m_modulebases.empty(); }

	void setBase(ModuleClassPtr mclass) ;
	void setBases(const ModuleClassList& mclasses) ;

	inline std::vector<size_t> getAllBaseIds() const {
		std::vector<size_t> basesid;
		for(pgl_hash_set<size_t>::const_iterator it = m_modulebasescache.begin(); it != m_modulebasescache.end(); ++it)
			basesid.push_back(*it);
		return basesid; 
	}

	bool issubclass(const ModuleClassPtr& mclass) const;

	void activate();
	void desactivate(); 

protected:
	void updateInheritedParameters() ;

	PropertyMap m_propertymap;
	ModuleClass * m_owner;
	typedef ModuleClass * ModuleClassDirectPtr;
	typedef std::vector<ModuleClassDirectPtr> ModuleClassDirectPtrList;
	ModuleClassDirectPtrList m_modulebases;
	pgl_hash_set<size_t> m_modulebasescache;

};

typedef RCPtr<ModuleVTable> ModuleVTablePtr;
typedef std::vector<ModuleVTablePtr> ModuleVTableList;

/*---------------------------------------------------------------------------*/

LPY_END_NAMESPACE
