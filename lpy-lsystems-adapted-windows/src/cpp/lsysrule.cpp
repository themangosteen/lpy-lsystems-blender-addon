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
#include "lsysrule.h"
#include "matching.h"
#include "lpy_parser.h"
#include "lsyscontext.h"
#include "tracker.h"
#include "argcollector_core.h"
#include <boost/version.hpp>
#include <sstream>

using namespace boost::python;
LPY_USING_NAMESPACE

#define bp boost::python

/*---------------------------------------------------------------------------*/

/*LsysRule::LsysRule():
m_id(0),
m_gid(0),
m_prefix('p'),
m_hasquery(false),
lineno(-1){
  IncTracker(LsysRule)
}*/

LsysRule::LsysRule(const LsysRule& other):
m_id(other.m_id),
m_gid(other.m_gid),
m_prefix(other.m_prefix),
m_predecessor(other.m_predecessor),
m_leftcontext(other.m_leftcontext),
m_newleftcontext(other.m_newleftcontext),
m_rightcontext(other.m_rightcontext),
m_newrightcontext(other.m_newrightcontext),
m_formalparameters(other.m_formalparameters),
m_nbParams(other.m_nbParams),
m_definition(other.m_definition),
m_hasquery(other.m_hasquery),
m_isStatic(other.m_isStatic),
m_staticResult(other.m_staticResult),
m_function(other.m_function),
lineno(other.lineno),
m_codelength(other.m_codelength),
m_consider(other.m_consider),
m_lstringmatcher(){
  IncTracker(LsysRule)
}

LsysRule::LsysRule( size_t id, size_t gid, char prefix, int _lineno):
m_id(id),
m_gid(gid),
m_prefix(prefix),
m_nbParams(0),
m_hasquery(false),
m_isStatic(false),
lineno(_lineno),
m_codelength(0),
m_lstringmatcher(){
  IncTracker(LsysRule)
}

LsysRule::~LsysRule() { 
	DecTracker(LsysRule) 
}



size_t LsysRule::nbContexts() const {
  size_t c = (m_leftcontext.empty()?0:1);
  c += (m_newleftcontext.empty()?0:1);
  c += (m_newrightcontext.empty()?0:1);
  c += (m_rightcontext.empty()?0:1);
  return c;
}

void LsysRule::clear(){
  m_id = 0;
  m_gid = 0;
  m_prefix = 'p';
  m_predecessor.clear();
  m_newleftcontext.clear();
  m_leftcontext.clear();
  m_rightcontext.clear();
  m_newrightcontext.clear();
  m_formalparameters.clear();
  m_nbParams = 0;
  m_definition.clear();
  m_function = object();
  m_hasquery = false;
  m_isStatic = false;
  m_staticResult.clear();
  lineno = -1;
  m_codelength = 0;
  m_consider = ConsiderFilterPtr();
  m_lstringmatcher = LstringMatcherPtr();
}

std::string LsysRule::str() const {
  std::string res = name();
  if(res.empty())res += "(none)";
  res += " : \n";
  if(m_definition.empty())res += "\t(none)";
  else res += m_definition;
  return res;
}

std::string LsysRule::name() const {
  std::string res;
  if (!m_leftcontext.empty())
		  res = m_leftcontext.str() + " < ";
  if (!m_newleftcontext.empty())
	res += m_newleftcontext.str() + " << ";
  res += m_predecessor.str();
  if (!m_newrightcontext.empty())
	res +=  " >> " + m_newrightcontext.str();
  if (!m_rightcontext.empty())
	res +=  " > " + m_rightcontext.str();
  return res;
}

std::string LsysRule::uname() const {
  std::string res;
  if (!m_leftcontext.empty())
		  res = "'"+m_leftcontext.str() + "' < ";
  if (!m_newleftcontext.empty())
	res += "'"+m_newleftcontext.str() + "' << ";
  res += "'"+m_predecessor.str()+"'";
  if (!m_newrightcontext.empty())
	res +=  " >> '" + m_newrightcontext.str()+"'";
  if (!m_rightcontext.empty())
	res +=  " > '" + m_rightcontext.str()+"'";
  return res;
}

std::string LsysRule::functionName() const {
  std::stringstream ss;
  ss << "m_" << m_prefix << "_" << m_gid << "_" << m_id << "_";
  std::string name = m_leftcontext.str();
  std::string::const_iterator _it;
  for (_it = name.begin();_it != name.end(); ++_it){
	if (isalpha(*_it)) ss << *_it;
	else ss << '_';
  }
  name = m_newleftcontext.str();
  for (_it = name.begin(); _it != name.end(); ++_it){
	if (isalpha(*_it)) ss << *_it;
	else ss << '_';
  }
  name = m_predecessor.str();
  for (_it = name.begin(); _it != name.end(); ++_it){
	if (isalpha(*_it)) ss << *_it;
	else ss << '_';
  }
  name = m_newrightcontext.str();
  for (_it = name.begin(); _it != name.end(); ++_it){
	if (isalpha(*_it)) ss << *_it;
	else ss << '_';
  }
  name = m_rightcontext.str();
  for (_it = name.begin(); _it != name.end(); ++_it){
	if (isalpha(*_it)) ss << *_it;
	else ss << '_';
  }
  return ss.str();
}

std::string LsysRule::callerFunctionName() const { return functionName()+"_caller"; }

std::string 
LsysRule::getCode() {
    return getCoreCode() +getCallerCode();
}


void LsysRule::setStatic()
{
  if(!m_isStatic){
	m_isStatic = true;
	if(!m_leftcontext.empty())     m_leftcontext.setUnnamedVariables();
	if(!m_newleftcontext.empty())  m_newleftcontext.setUnnamedVariables();
	if(!m_predecessor.empty())     m_predecessor.setUnnamedVariables();  
	if(!m_newrightcontext.empty()) m_newrightcontext.setUnnamedVariables();  
	if(!m_rightcontext.empty())    m_rightcontext.setUnnamedVariables();
	m_formalparameters.clear();
	m_nbParams = 0;
  }
}

#ifdef USE_PYTHON_LIST_COLLECTOR
#define MAX_LRULE_DIRECT_ARITY  0
#else
#define MAX_LRULE_DIRECT_ARITY  15
#endif

std::string 
LsysRule::getCallerCode() const{
  if (m_nbParams <= MAX_LRULE_DIRECT_ARITY) return "";
  std::stringstream res;
  res << "def " << callerFunctionName() << "(args=[]) : return " << functionName() << "(*args)\n";
  return res.str();
}
void LsysRule::compile(){
	if (!isCompiled()){ recompile(); }
	else LsysWarning("Python code already imported.");
}

void LsysRule::recompile(){
	std::string fname = (m_nbParams<=MAX_LRULE_DIRECT_ARITY?functionName():callerFunctionName());
	  m_function = LsysContext::currentContext()->compile(fname,getCode());
      // LsysContext::currentContext()->getObject(fname);
	  if (!isCompiled()) LsysError("Compilation failed.");
	// m_function = LsysContext::currentContext()->compile(functionName(),getCode());
	  initStaticProduction();
}

void LsysRule::importPyFunction(){
	if (!isCompiled()){
      m_function = LsysContext::currentContext()->getObject(m_nbParams<=MAX_LRULE_DIRECT_ARITY?functionName():callerFunctionName());
      // m_function = LsysContext::currentContext()->getObject(functionName());
	  initStaticProduction();
	}
	else LsysWarning("Python code already imported.");
}

void LsysRule::initStaticProduction(){
  if(m_isStatic){
	  m_isStatic = false;
	  if(m_nbParams==0) m_staticResult = apply();
	  else {
		  ArgList args;
		  for (size_t i =0; i < m_nbParams; ++i)args.push_back(object());
		  m_staticResult = apply(args);
	  }
	  m_isStatic = true;
  }
}

void LsysRule::precall_function( size_t nbargs ) const
{
  LsysContext::currentContext()->reset_nproduction();
  if (nbargs != m_nbParams) {
      std::stringstream res;
      res << name() << " takes exactly " << m_nbParams << " argument(s) (" << nbargs << " given).\n";
      LsysError(res.str());
    }
}
void LsysRule::precall_function( size_t nbargs, const ArgList& args ) const
{
  LsysContext::currentContext()->reset_nproduction();
  if (nbargs != m_nbParams) {
      std::stringstream res;
      res << name() << " takes exactly " << m_nbParams << " argument(s) (" << nbargs << " given).\n"
		  << bp::extract<std::string>(bp::str(toPyList(args)))();
      LsysError(res.str());
    }
}


AxialTree LsysRule::postcall_function( boost::python::object res, bool * isApplied ) const
{
  if (res == object()) 
  { 
      // no production. look for nproduction
      AxialTree nprod = LsysContext::currentContext()->get_nproduction(); 
	  if (nprod.empty()) {
		  if(isApplied != NULL) *isApplied = false;
		  return AxialTree();
	  }
      else { 
		  if(isApplied != NULL) *isApplied = true;
          LsysContext::currentContext()->reset_nproduction(); // to avoid deep copy
          return nprod;
      }
  }
  else {
	 if(isApplied != NULL) *isApplied = true;
      // production. add nproduction if needed
      AxialTree nprod = LsysContext::currentContext()->get_nproduction(); 
      AxialTree pres = AxialTree(extract<boost::python::list>(res));
      if (!nprod.empty()){
          LsysContext::currentContext()->reset_nproduction(); //  to avoid deep copy
          nprod += pres;
          return nprod;
      }
      else return pres;
  }
}


boost::python::object LsysRule::call_function( size_t nbargs, const ArgList& args ) const
{
	ConsiderFilterMaintainer cm(m_consider);
	switch(nbargs){
		case 0: return m_function(); break;
#if MAX_LRULE_DIRECT_ARITY > 0
		case 1: return m_function(args[0]); break;
		case 2: return m_function(args[0],args[1]); break;
		case 3: return m_function(args[0],args[1],args[2]); break;
		case 4: return m_function(args[0],args[1],args[2],args[3]); break;
		case 5: return m_function(args[0],args[1],args[2],args[3],args[4]); break;
		case 6: return m_function(args[0],args[1],args[2],args[3],args[4],args[5]); break;
		case 7: return m_function(args[0],args[1],args[2],args[3],args[4],args[5],args[6]); break;
		case 8: return m_function(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]); break;
		case 9: return m_function(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8]); break;
		case 10: return m_function(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9]); break;
		case 11: return m_function(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10]); break;
		case 12: return m_function(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11]); break;
		case 13: return m_function(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],args[12]); break;
		case 14: return m_function(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],args[12],args[13]); break;
		case 15: return m_function(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11],args[12],args[13],args[14]); break;
#endif
		default: return m_function(toPyList(args)); break;
	}
}

AxialTree 
LsysRule::apply( const ArgList& args, bool * isApplied ) const
{ 
  if(m_isStatic) { 
    if(isApplied) *isApplied = true;
	return m_staticResult;
  }
  if (!isCompiled()) LsysError("Python code of rule not compiled");

  LstringMatcherMaintainer m(m_lstringmatcher);
  size_t argsize = len(args);
  precall_function(argsize,args);
  return postcall_function(call_function(argsize,args),isApplied); 
}



AxialTree 
LsysRule::apply( bool * isApplied ) const
{ 
  if(m_isStatic) { 
    if(isApplied) *isApplied = true;
	return m_staticResult;
  }
  if (!isCompiled()) LsysError("Python code of rule not compiled");

  LstringMatcherMaintainer m(m_lstringmatcher);
  precall_function();
  return postcall_function(m_function(),isApplied); 
}

template<class T>
inline void extend_vec(T& v, const T& v2) { v.insert(v.end(),v2.begin(),v2.end()); }

void 
LsysRule::parseParameters(){
  m_formalparameters.clear();
  if(!m_leftcontext.empty())
	extend_vec(m_formalparameters,m_leftcontext.getVarNames());
  if(!m_newleftcontext.empty())
	extend_vec(m_formalparameters,m_newleftcontext.getVarNames());
  if(!m_predecessor.empty())
	extend_vec(m_formalparameters,m_predecessor.getVarNames());  
  if(!m_newrightcontext.empty())
	extend_vec(m_formalparameters,m_newrightcontext.getVarNames());  
  if(!m_rightcontext.empty())
	extend_vec(m_formalparameters,m_rightcontext.getVarNames());
  int rp = redundantParameter();
  if(rp != -1){
      LsysError("Ill-formed Rule Header : Multiple definition of parameter "+m_formalparameters[rp]+": "+uname());
  }
  m_nbParams = m_formalparameters.size();
}

int
LsysRule::redundantParameter() const {
    if (m_formalparameters.empty()) return -1;
    std::vector<std::string>::const_iterator _first = m_formalparameters.begin();
    std::vector<std::string>::const_iterator _last = m_formalparameters.end() - 1;
    while (_first != _last) {
      if (std::find(_first + 1,m_formalparameters.end(),*_first) 
				!= m_formalparameters.end()) return distance(m_formalparameters.begin(),_first);
      _first++;
    };
    return -1;
}

inline bool isAlNum_(char c) { return isalnum(c) || c == '_'; }

void LsysRule::keepOnlyRelevantVariables()
{
	std::vector<PatternString*> pstrings;
	pstrings.push_back(&m_leftcontext); 
	pstrings.push_back(&m_newleftcontext); 
	pstrings.push_back(&m_predecessor); 
	pstrings.push_back(&m_newrightcontext); 
	pstrings.push_back(&m_rightcontext); 
	for(std::vector<PatternString*>::const_iterator itS = pstrings.begin(); 
		itS != pstrings.end(); ++itS)
	{
		std::vector<std::string> varnames =  (*itS)->getVarNames();
		std::vector<size_t> toRemove;
		for(std::vector<std::string>::const_iterator it = varnames.begin(); 
			it != varnames.end(); ++it)
		{	  
			size_t pos = 0;
			bool found = false;
			while (pos != std::string::npos){
				if ((pos = m_definition.find(*it,pos)) != std::string::npos ){
					size_t endpos = pos + it->size();
					if( (pos == 0 || !isAlNum_(m_definition[pos-1])) && 
					    (endpos == m_definition.size() || !isAlNum_(m_definition[endpos]))){
						found = true;
						break;
					}
					else { 
						pos+=1;
					}
				}
			}
			if(!found) {
				toRemove.push_back(std::distance<std::vector<std::string>::const_iterator>(varnames.begin(),it));
			}
		}
		std::vector<size_t>::const_reverse_iterator rend = toRemove.rend();
		for(std::vector<size_t>::const_reverse_iterator itR = toRemove.rbegin(); 
			itR != rend; ++itR){
				(*itS)->setUnnamedVariable(*itR);
		}

	}
}

/*---------------------------------------------------------------------------*/


bool
LsysRule::match(const AxialTree& src,
			   AxialTree::const_iterator pos,
			   const AxialTree& dest,
			   AxialTree::const_iterator& endpos,
               ArgList& args,
               eDirection direction) const 
{
  ConsiderFilterMaintainer cm(m_consider);
  args.reserve(m_nbParams);
  ArgList args_pred;
  AxialTree::const_iterator endpos1;
  AxialTree::const_iterator last_match = pos;

  // strict predecessor
  if (direction == eForward){
   if(!MatchingEngine::match(pos,src.const_begin(),src.const_end(),m_predecessor.const_begin(),m_predecessor.const_end(),endpos1,last_match,args_pred)){
	 return false;
   }
  }
  else{
    AxialTree::const_iterator tmp;
    if(!MatchingEngine::reverse_match(pos,src.const_begin(),src.const_end(),
		                              m_predecessor.const_rbegin(),m_predecessor.const_rend(),
									  tmp,args_pred))
 	   return false;
    endpos1 = (pos == src.end()?pos:pos+1);
    pos = tmp;
  }

  // left context
  AxialTree::const_iterator endposLeft = (direction == eForward?pos:pos+1);
  if(!m_leftcontext.empty()){
      if(!MatchingEngine::left_match(endposLeft,src.const_begin(),src.const_end(),
		                              m_leftcontext.const_rbegin(),m_leftcontext.const_rend(),
									  endposLeft,args))
	       return false;
  }

  // new left context
  AxialTree::const_iterator endposNewLeft;
  if(direction == eForward && !m_newleftcontext.empty()){
	ArgList args_ncg;
    // Here we do a hack to add the current element to the new string to have scale information.
    AxialTree *dest2 = const_cast<AxialTree *>(&dest);
    dest2->push_back(pos);
    if(!MatchingEngine::left_match(dest2->const_end()-1,dest2->const_begin(),dest2->const_end(),
		                          m_newleftcontext.const_rbegin(),m_newleftcontext.const_rend(),
								  endposNewLeft,args_ncg)){
        dest2->erase(dest2->end()-1);
        return false;
    }
    dest2->erase(dest2->end()-1);
	ArgsCollector::append_args(args,args_ncg);
  }

  ArgsCollector::append_args(args,args_pred);

  // new right context
  AxialTree::const_iterator endposNewRight;
  AxialTree::const_iterator endposNewRightLastMatch = last_match;
  if(direction == eBackward && !m_newrightcontext.empty()){
	ArgList args_ncd;
    if(!MatchingEngine::right_match(dest.const_begin(),dest.const_begin(),dest.const_end(),
		                          m_newrightcontext.const_begin(),m_newrightcontext.const_end(),
								  endposNewRightLastMatch,endposNewRight,args_ncd)) return false;
    							  // last_match,endpos2,args_ncd)) return false;
	ArgsCollector::append_args(args,args_ncd);
  }

  // right context
  AxialTree::const_iterator endposRight = endpos1;
  AxialTree::const_iterator endposRightLastMatch = last_match;
  if(!m_rightcontext.empty()){
	ArgList args_cd;
    if(!MatchingEngine::right_match(endposRight,src.const_begin(),src.const_end(),
		                          m_rightcontext.const_begin(),m_rightcontext.const_end(),
								  endposRightLastMatch,endposRight,args_cd))return false;
	ArgsCollector::append_args(args,args_cd);
  }
  const_cast<LsysRule *>(this)->m_lstringmatcher = LstringMatcherPtr(new LstringMatcher(src.const_begin(),	
					   src.const_end(),
					   endposLeft,
					   // endposNewLeft,
					   endposRight,
					   endposRightLastMatch //,
					   // endposNewRight,
					   // endposRightLastMatch
					   ));


  if (direction == eForward) endpos = endpos1;
  else                       endpos = pos;
  return true;
}

bool
LsysRule::applyTo( AxialTree& dest, 
				   const ArgList& args, 
				   size_t * length,
				   eDirection direction) const {
  
   AxialTree prod;
   if(m_isStatic) prod = m_staticResult;
   else {
	bool success = false;
	prod = apply(args,&success);
	if(!success)return false;
   }
   ModuleClassPtr cl;
   if (!prod.empty() && ((cl=prod[0].getClass()) == ModuleClass::Star || cl == ModuleClass::None)){ 
		if(length!=NULL)*length = 0;
   }
   else {
	if(length!=NULL)*length = prod.size();
	if(direction == eForward) dest += prod;
	else dest.prepend(prod);
   }
   return true;
}


AxialTree
LsysRule::process( const AxialTree& src ) const {
  AxialTree dest;
  AxialTree::const_iterator _it = src.begin();
  while(_it != src.end()){
	ArgList args;
	if(!match(src,_it,dest,_it,args)){
	  dest.push_back(_it);
	  ++_it;
	}
	else { applyTo(dest,args); }
  }
  return dest;
}
/*---------------------------------------------------------------------------*/

void 
LsysRule::consider(const ConsiderFilterPtr consider)
{
	m_consider = consider;
}

void 
LsysRule::consider(const std::string& modules)
{
	m_consider = ConsiderFilterPtr(new ConsiderFilter(modules));
}

void 
LsysRule::ignore(const std::string& modules)
{
	m_consider = ConsiderFilterPtr(new ConsiderFilter(modules,eIgnore));
}

/*---------------------------------------------------------------------------*/

RulePtrMap::RulePtrMap(const RulePtrSet& rules, eDirection direction):
	m_map(ModuleClass::getMaxId()), m_nbrules(rules.size()), m_maxsmb(0)
{
	/* all classes. Required for inheritance tests */
	ModuleClassList allclasses = ModuleClassTable::get().getClasses();
	// preprocess classes to test only classes that derived from others
    ModuleClassList derivedclasses;
	for(ModuleClassList::const_iterator itCl = allclasses.begin(); itCl != allclasses.end(); ++itCl)
		if ((*itCl)->hasBaseClasses()) derivedclasses.push_back(*itCl);


    // Process all rules and get ids that match first pattern module
	for(RulePtrSet::const_iterator it = rules.begin(); it != rules.end(); ++it){
		std::vector<size_t> ids = (direction == eForward?(*it)->predecessor().getFirstClassId():(*it)->predecessor().getLastClassId());
		for(std::vector<size_t>::const_iterator itid = ids.begin(); itid != ids.end(); ++itid){
			// star module match everythings.
			if(*itid == ModuleClass::Star->getId()){
				for(RulePtrSetMap::iterator itmap = m_map.begin(); itmap != m_map.end(); ++itmap)
					itmap->push_back(*it);
				m_defaultset.push_back(*it);
			}
			else { 
				m_map[*itid].push_back(*it);
				/* In the case of inheritance, we should find derived classes 
				   that can match a base pattern */
				if (MatchingEngine::isInheritanceModuleMatchingActivated() && !derivedclasses.empty()){
					ModuleClassPtr mclass = ModuleClassTable::get().find(*itid);
					for(ModuleClassList::const_iterator itCl = derivedclasses.begin(); itCl != derivedclasses.end(); ++itCl)
						if(*itCl != mclass && (*itCl)->issubclass(mclass)){
							m_map[(*itCl)->getId()].push_back(*it);
						}
				}
			}
		}
	}
	// we check for now how much symbol are included
	m_maxsmb = m_map.size();
}

RulePtrMap::RulePtrMap():
	m_map(0), m_nbrules(0), m_maxsmb(0)
{
}

/*---------------------------------------------------------------------------*/
