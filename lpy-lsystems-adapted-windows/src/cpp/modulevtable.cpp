#define BOOST_PYTHON_STATIC_LIB
#include "modulevtable.h"
#include "lsyscontext.h"
#include "tracker.h"

LPY_BEGIN_NAMESPACE

/*---------------------------------------------------------------------------*/

BaseModuleProperty::BaseModuleProperty(const std::string& _name): 
	  TOOLS(RefCountObject)(), name(name) { IncTracker(ModuleProperty) }

BaseModuleProperty::~BaseModuleProperty() { DecTracker(ModuleProperty) }

/*---------------------------------------------------------------------------*/

ModuleVTable::ModuleVTable(ModuleClassPtr owner, ModuleClassPtr base) : 
m_owner(owner.get()), m_modulebases(is_null_ptr(base)?0:1,base.get()), scale(ModuleClass::DEFAULT_SCALE)
{ 
	LsysContext::current()->m_modulesvtables.push_back(ModuleVTablePtr(this));
	IncTracker(ModuleVTable)
	if(!m_modulebases.empty()) updateInheritedParameters();
}

ModuleVTable::~ModuleVTable() { DecTracker(ModuleVTable) }

ModulePropertyPtr ModuleVTable::getProperty(const std::string& name) const
{
	PropertyMap::const_iterator it = m_propertymap.find(name);
	if(it == m_propertymap.end()) return it->second;
	else return ModulePropertyPtr();
}

void ModuleVTable::setProperty(ModulePropertyPtr prop)
{
	if(!prop || prop->name.empty())LsysError("Invalid property");
	m_propertymap[prop->name] = prop;
}

bool ModuleVTable::removeProperty(const std::string& name)
{
	PropertyMap::iterator it = m_propertymap.find(name);
	if(it == m_propertymap.end()) return false;
	else { m_propertymap.erase(it); return true; }
}

void ModuleVTable::activate()
{
	if(m_owner)m_owner->m_vtable = ModuleVTablePtr(this);
}

void ModuleVTable::desactivate()
{
	if(m_owner)m_owner->m_vtable = ModuleVTablePtr();
}

void ModuleVTable::setBase(ModuleClassPtr mclass) 
{ 
	m_modulebases.clear();
	m_modulebases.push_back(mclass.get()); 
	updateInheritedParameters();
}


void ModuleVTable::setBases(const ModuleClassList& mclass) 
{ 
	m_modulebases.clear();
	for(ModuleClassList::const_iterator it = mclass.begin(); it != mclass.end(); ++it)
		m_modulebases.push_back(it->get());			
	updateInheritedParameters();
}

bool ModuleVTable::issubclass(const ModuleClassPtr& mclass) const
{  return m_modulebasescache.find(mclass->getId()) != m_modulebasescache.end(); }

void ModuleVTable::updateInheritedParameters()
{
	if(!m_modulebases.empty()){
		std::vector<ModuleClass *> bases = m_modulebases;
		m_modulebasescache.clear();
		while(!bases.empty()){
			ModuleClass * base = bases[0];
			if (base == m_owner){
				m_modulebasescache.clear();
				m_modulebases.clear();
				LsysError("Cyclic inheritance");
			}
			bases.erase(bases.begin());
			m_modulebasescache.insert(base->getId());
			if (base != NULL){
				ModuleVTable * basevtable = base->m_vtable;
				if( basevtable ) {
					if(scale == ModuleClass::DEFAULT_SCALE) scale = basevtable->scale;
					bases.insert(bases.begin(),basevtable->m_modulebases.begin(),basevtable->m_modulebases.end());
				}
			}
		}
		std::vector<std::string> params =  m_owner->getParameterNames();
		for(std::vector<ModuleClass *>::const_iterator it = m_modulebases.begin(); it != m_modulebases.end(); ++it){
			std::vector<std::string> iparams =  (*it)->getParameterNames();
			params.insert(params.end(),iparams.begin(),iparams.end());
		}
		m_owner->setParameterNames(params);
	}
}

/*---------------------------------------------------------------------------*/

LPY_END_NAMESPACE
