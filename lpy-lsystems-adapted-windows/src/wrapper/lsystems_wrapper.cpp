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
#include <iostream>
#include <boost/python.hpp>
#include "export_lsystem.h"
#include "../cpp/moduleclass.h"
#include "../cpp/lsyscontext.h"
#include "../cpp/tracker.h"
#include "../plantgl/python/exception_core.h"
#include "../plantgl/python/export_refcountptr.h"

#ifndef LPY_NO_PLANTGL_INTERPRETATION
#include "../plantgl/gui/base/application.h"
#endif

using namespace boost::python;
LPY_USING_NAMESPACE


void cleanLsys() 
{
#ifdef TRACKER_ENABLED
	std::cerr << "****** pre-cleaning *******" << std::endl;
	Tracker::printReport();
#endif
	LsysContext::cleanContexts();
	ModuleClassTable::clearModuleClasses ();
#ifndef LPY_NO_PLANTGL_INTERPRETATION
	ViewerApplication::exit ();
#endif
#ifdef TRACKER_ENABLED
	std::cerr << "****** post-cleaning ******" << std::endl;
	Tracker::printReport();
#endif
}

BOOST_PYTHON_MODULE(lpy)
{
	define_stl_exceptions();
	export_Options();
	export_ModuleClass();
	export_Module();
	export_PatternModule();
    export_NodeModule();
	export_AxialTree();
	export_PatternString();
    export_Consider();
    export_LsysRule();
    export_LsysContext();
    export_Lsystem();
	export_parser();
    export_StringMatching();
#ifndef LPY_NO_PLANTGL_INTERPRETATION
    export_Debugger();
    export_Interpretation();
    export_plot();
#endif
	// def("cleanLsys",&cleanLsys);
	Py_AtExit(&cleanLsys);
};
