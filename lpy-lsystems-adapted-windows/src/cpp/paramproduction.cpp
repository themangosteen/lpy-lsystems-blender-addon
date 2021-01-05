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
#define BOOST_PYTHON_STATIC_LIB
#include "paramproduction.h"
#include "../plantgl/tool/util_string.h"

using namespace boost::python;
LPY_USING_NAMESPACE

#define bp boost::python

ParametricProductionPtr ParametricProduction::create()
{
	ParametricProductionPtr value(new ParametricProduction());
	ParamProductionManager::get().add_production(*value);
	return value;
}

ParametricProductionPtr ParametricProduction::get(size_t pid)
{
	return ParamProductionManager::get().get_production(pid);
}

ParametricProduction::~ParametricProduction(){
	ParamProductionManager::get().remove_production(*this);
}

ParamProductionManager * ParamProductionManager::Instance(0);

ParamProductionManager& ParamProductionManager::get()
{
	if (ParamProductionManager::Instance == NULL)
		ParamProductionManager::Instance = new ParamProductionManager();
	return *ParamProductionManager::Instance;
}

ParamProductionManager::ParamProductionManager():
	m_productions(), m_free_indices()
{
}

ParametricProductionPtr ParamProductionManager::get_production(size_t pid)
{
	if (pid < m_productions.size())
	{
		return ParametricProductionPtr(m_productions[pid]);
	}
	else 
	{
		std::string msg("Cannot find parametric production ");
		msg += TOOLS(number(pid));
		LsysError(msg);
		return ParametricProductionPtr();
	}
}

void ParamProductionManager::add_production(ParametricProduction& value)
{

	size_t pid;
	if (m_free_indices.empty()){
		pid = m_productions.size();
		m_productions.push_back(&value);
	}
	else {
		pid = m_free_indices.front();
		m_free_indices.pop();
		m_productions[pid] = & value;
	}

	value.m_pid = pid;
}

void ParamProductionManager::remove_production(ParametricProduction& value)
{
	size_t pid = value.pid();
	if (pid == m_productions.size())
		m_productions.pop_back();
	else {
		m_free_indices.push(pid);
		m_productions[pid] = NULL;
	}
}
