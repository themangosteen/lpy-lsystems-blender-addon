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
#include "lsystem.h"
#include "tracker.h"
#include <QtCore/QThread>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include "../plantgl/tool/sequencer.h"
#include "../plantgl/math/util_math.h"

using namespace boost::python;
LPY_USING_NAMESPACE

#ifndef LPY_NO_PLANTGL_INTERPRETATION
#include "interpretation.h"
#include "plot.h"
PGL_USING_NAMESPACE
#endif

/*---------------------------------------------------------------------------*/

#ifdef MULTI_THREADED_LSYSTEM
#define ACQUIRE_RESSOURCE  LsysAcquirer ressource(this); 
#define RELEASE_RESSOURCE  

#define PRINT_RESSOURCE(msg) 
// printf(msg": %i : %i %i %i; %s %i\n",__LINE__,(int)m_ressource,(int)this,(int)QThread::currentThread(),(isRunning()?"True":"False"),m_ressource->count);
#else
#define ACQUIRE_RESSOURCE  
#define PRINT_RESSOURCE(msg) 
#define RELEASE_RESSOURCE
#endif

/*---------------------------------------------------------------------------*/

#ifdef MULTI_THREADED_LSYSTEM
Lsystem::LsysRessource::LsysRessource() : mutex(QMutex::NonRecursive) {}
Lsystem::LsysAcquirer::LsysAcquirer(const Lsystem * lsys) : m_lsys(lsys) { lsys->acquire(); }
Lsystem::LsysAcquirer::~LsysAcquirer() { m_lsys->release(); }
#endif

/*---------------------------------------------------------------------------*/

Lsystem::RuleGroup::RuleGroup():
  m_prodhasquery(false),
  m_dechasquery(false),
  m_inthasquery(false)
{}

const RuleSet& 
Lsystem::RuleGroup::getGroup(eRuleType t) const
{
    switch (t){
        case eProduction:
            return production;
            break;
        case eDecomposition:
            return decomposition;
            break;
        case eInterpretation:
            return interpretation;
            break;
        default:
            return production;
            break;
    }
}

RuleSet& 
Lsystem::RuleGroup::getGroup(eRuleType t)
{
    switch (t){
        case eProduction:
            return production;
            break;
        case eDecomposition:
            return decomposition;
            break;
        case eInterpretation:
            return interpretation;
            break;
        default:
            return production;
            break;
    }
}

bool Lsystem::RuleGroup::hasQuery(eRuleType t) const
{
    switch (t){
        case eProduction:
            return m_prodhasquery;
            break;
        case eDecomposition:
            return m_dechasquery;
            break;
        case eInterpretation:
            return m_inthasquery;
            break;
        default:
            return m_prodhasquery;
            break;
    }
}


/*---------------------------------------------------------------------------*/

Lsystem::Lsystem():
m_max_derivation(1),
m_decomposition_max_depth(1),
m_interpretation_max_depth(1),
m_currentGroup(0),
m_context(),
m_newrules(false)
#ifdef MULTI_THREADED_LSYSTEM
,m_ressource(new LsysRessource())
#endif
{
  IncTracker(Lsystem)
  PRINT_RESSOURCE("create")
}



Lsystem::Lsystem(const std::string& filename):
m_max_derivation(1),
m_decomposition_max_depth(1),
m_interpretation_max_depth(1),
m_context(),
m_newrules(false)
#ifdef MULTI_THREADED_LSYSTEM
,m_ressource(new LsysRessource())
#endif
{
  IncTracker(Lsystem)
  PRINT_RESSOURCE("create")
  read(filename);
}

Lsystem::Lsystem(const std::string& filename, 
			     const boost::python::dict& parameters):
m_max_derivation(1),
m_decomposition_max_depth(1),
m_interpretation_max_depth(1),
m_context(),
m_newrules(false)
#ifdef MULTI_THREADED_LSYSTEM
,m_ressource(new LsysRessource())
#endif
{
  IncTracker(Lsystem)
  PRINT_RESSOURCE("create")
  read(filename,parameters);
}

Lsystem::Lsystem(const Lsystem& lsys):
m_rules(lsys.m_rules),
m_max_derivation(lsys.m_max_derivation),
m_decomposition_max_depth(lsys.m_decomposition_max_depth),
m_interpretation_max_depth(lsys.m_interpretation_max_depth),
m_context(lsys.m_context),
m_newrules(lsys.m_newrules)
#ifdef MULTI_THREADED_LSYSTEM
,m_ressource(new LsysRessource())
#endif
{
  IncTracker(Lsystem)
  PRINT_RESSOURCE("create")
}

Lsystem& Lsystem::operator=(const Lsystem& lsys)
{
    m_rules = lsys.m_rules;
    m_max_derivation =lsys.m_max_derivation;
    m_decomposition_max_depth = lsys.m_decomposition_max_depth;
    m_interpretation_max_depth = lsys.m_interpretation_max_depth;
    m_context = lsys.m_context;
    return *this;
}

Lsystem::RuleGroup& Lsystem::group(size_t group_)
{
    if (group_ >= m_rules.size()){
        size_t s = m_rules.size();
        for(;s <= group_;++s)
            m_rules.push_back(RuleGroup());
    }
    return m_rules[group_];
}

const Lsystem::RuleGroup& Lsystem::group(size_t group_) const
{
    return m_rules[group_];
}

Lsystem::~Lsystem()
{
 DecTracker(Lsystem)
 clear();
#ifdef MULTI_THREADED_LSYSTEM
 delete m_ressource;
#endif
 if(isCurrent())done();
 PRINT_RESSOURCE("delete")
}

void 
Lsystem::clear(){
  ACQUIRE_RESSOURCE
  clearLsys();
  RELEASE_RESSOURCE
}
void 
Lsystem::clearLsys(){
  m_axiom.clear();
  m_rules.clear();
  m_max_derivation = 1;
  m_decomposition_max_depth = 1;
  m_interpretation_max_depth = 1;
  m_context.clear();
  reference_existing_object::apply<Lsystem*>::type converter;
  PyObject* obj = converter( this );
  object real_obj = object( handle<>( obj ) );
  m_context.setObject("__lsystem__",real_obj);

}



std::string 
Lsystem::str() const {
  ACQUIRE_RESSOURCE
  std::stringstream s;
  s << "Lsystem:\n";
  s << "Axiom: " << m_axiom.str() << '\n';

  if(!m_context.m_modules.empty()){
	s << "module ";
	for(ModuleClassList::const_iterator it = m_context.m_modules.begin();
		it != m_context.m_modules.end(); ++it){
		if(it != m_context.m_modules.begin()) s << ',';
		s << (*it)->name; 
	}
	s << '\n';
  }

  s << "derivation length: " << m_max_derivation << '\n';
  s << "production:\n";
  size_t gid = 0;
  for (RuleGroupList::const_iterator g = m_rules.begin(); g != m_rules.end(); ++g){
          if (gid != 0){
            s << "group: " << gid << std::endl;
            s << "production:\n";
          }
          for (RuleSet::const_iterator   i = g->production.begin();
              i != g->production.end(); i++){
                  s << i->str() << '\n';
          }
          if(!g->decomposition.empty()){
              s << "decomposition:\n";
              if (gid == 0)
                s << "maximum depth:"  << m_decomposition_max_depth << '\n';
              for (RuleSet::const_iterator i = g->decomposition.begin();
                  i != g->decomposition.end(); i++){
                      s << i->str()+'\n';
              }
          }
          if(!g->interpretation.empty()){
              s << "interpretation:\n";
              if (gid == 0)
                s << "maximum depth:"  << m_interpretation_max_depth << '\n';
              for (RuleSet::const_iterator i = g->interpretation.begin();
                  i != g->interpretation.end(); i++){
                      s << i->str()+'\n';
              }
          }
          ++gid;
  }
  s << "endlsystem";
  return s.str();
  RELEASE_RESSOURCE
}

std::string 
Lsystem::code()  {
  ACQUIRE_RESSOURCE
  std::stringstream s;
  s << "Lsystem:\n";
  s << "Axiom: " << m_axiom.str() << '\n';
  s << "derivation length: " << m_max_derivation << '\n';
  s << "production:\n";
  size_t gid = 0;
  for (Lsystem::RuleGroupList::iterator g = m_rules.begin(); g != m_rules.end(); ++g){
          if (gid != 0){
            s << "group: " << gid << std::endl;
            s << "production:\n";
          }
          for (RuleSet::iterator   i = g->production.begin(); i != g->production.end(); ++i){
                  s << i->getCode() << '\n';
          }
          if(!g->decomposition.empty()){
              s << "decomposition:\n";
              if (gid == 0)
                s << "maximum depth:"  << m_decomposition_max_depth << '\n';
              for (RuleSet::iterator i = g->decomposition.begin(); i != g->decomposition.end(); ++i){
                      s << i->getCode()+'\n';
              }
          }
          if(!g->interpretation.empty()){
              s << "interpretation:\n";
              if (gid == 0)
                s << "maximum depth:"  << m_interpretation_max_depth << '\n';
              for (RuleSet::iterator i = g->interpretation.begin(); i != g->interpretation.end(); ++i){
                      s << i->getCode()+'\n';
              }
          }
          ++gid;
  }
  s << "endlsystem";
  return s.str();
  RELEASE_RESSOURCE
}

bool 
Lsystem::isCompiled(){
  ACQUIRE_RESSOURCE
  RuleSet::const_iterator i;
  for (RuleGroupList::const_iterator g = m_rules.begin(); g != m_rules.end(); ++g)
  {
      for ( i = g->production.begin();    i != g->production.end(); ++i)
          if(!i->isCompiled())return false;
      for ( i = g->decomposition.begin(); i != g->decomposition.end(); ++i)
          if(!i->isCompiled())return false;
      for ( i = g->interpretation.begin();  i != g->interpretation.end(); ++i)
          if(!i->isCompiled())return false;
  }
  return true;
  RELEASE_RESSOURCE
}

void 
Lsystem::compile(){
  ACQUIRE_RESSOURCE
  ContextMaintainer m(&m_context);
  RuleSet::iterator i;
  for (RuleGroupList::iterator g = m_rules.begin();
      g != m_rules.end(); ++g)
  {
      for (i = g->production.begin();    i != g->production.end(); ++i)
          i->compile();
      for (i = g->decomposition.begin(); i != g->decomposition.end(); ++i)
          i->compile();
      for (i = g->interpretation.begin();  i != g->interpretation.end(); ++i)
          i->compile();
  }
  RELEASE_RESSOURCE
}

void 
Lsystem::importPyFunctions(){
  ContextMaintainer m(&m_context);
  RuleSet::iterator i;
  for (RuleGroupList::iterator g = m_rules.begin(); g != m_rules.end(); ++g)
  {
      for ( i = g->production.begin();    i != g->production.end(); ++i)
          i->importPyFunction();
      for ( i = g->decomposition.begin(); i != g->decomposition.end(); ++i)
          i->importPyFunction();
      for ( i = g->interpretation.begin();  i != g->interpretation.end(); ++i)
          i->importPyFunction();
  }
}

#include <fstream>

void 
Lsystem::read(const std::string& filename, 
			  const boost::python::dict& parameters){
  clear();
  std::ifstream file(filename.c_str());
  if(file){
    setFilename(filename);

/* This part raise seg fault while trying to read certain lpy files.
 * It is replaced by the usage of file.rdbuf here below. TC. nov 2009
 *
 * std::string content;
#define bufsize 100000
	char text[bufsize+1];
	text[bufsize] = '\0';
	while(!file.eof()){
		file.read(text,bufsize);
	  content += std::string(text);
	}
	set(content);
*/

  std::stringstream buffer; 
  buffer << file.rdbuf();
  file.close();
  //std::cout << "buffer : " << buffer.str() << '\n';
  //std::cout << "Taille du buffer : " << buffer.str().size() << '\n';
  set(buffer.str(),NULL,parameters);
  }
  else {
	LsysError('\''+filename+"': No such file or directory.");
  }
}

void Lsystem::setFilename( const std::string& filename )
{
    m_context.setObject("__file__",boost::python::object(filename));
    std::string name =  QFileInfo(filename.c_str()).baseName().toStdString();
    m_context.setObject("__name__",boost::python::object(name));
	// std::string path =  QFileInfo(filename.c_str()).absoluteDir().path().toStdString();
    // m_context.setObject("__path__",boost::python::object(path));
}

std::string Lsystem::getFilename( ) const
{
    if(m_context.hasObject("__file__"))
        return extract<std::string>(m_context.getObject("__file__"));
    else return std::string();
}

std::string Lsystem::getShortFilename( ) const
{
    std::string filename = getFilename();
    if (filename.empty())return filename;
    else return QFileInfo(filename.c_str()).fileName().toStdString();
}

LsysRule& 
Lsystem::addProductionRule( const std::string& code, size_t groupid, int lineno, const ConsiderFilterPtr filter ){
  RuleGroup& group_ = group(groupid);
  LsysRule r(group_.production.size(),groupid,'p',lineno);
  r.set(code);
  r.consider(filter);
  group_.production.push_back(r);
  if (r.hasQuery())group_.m_prodhasquery = true;
  m_newrules = true;
  return *(group_.production.end()-1);
}

LsysRule& 
Lsystem::addDecompositionRule( const std::string& code, size_t groupid , int lineno, const ConsiderFilterPtr filter ){
  RuleGroup& group_ = group(groupid);
  LsysRule r(group_.decomposition.size(),groupid,'d',lineno);
  r.set(code);
  r.consider(filter);
  group_.decomposition.push_back(r);
  if (r.hasQuery())group_.m_dechasquery = true;
  m_newrules = true;
  return *(group_.decomposition.end()-1);
}

LsysRule&
Lsystem::addInterpretationRule( const std::string& code, size_t groupid, int lineno, const ConsiderFilterPtr filter ){
  RuleGroup& group_ = group(groupid);
  LsysRule r(group_.interpretation.size(),groupid,'h',lineno);
  r.set(code);
  r.consider(filter);
  if (!r.isContextFree())LsysWarning("Interpretation rules should be context free. Contexts not supported for multiple iterations.");
  group_.interpretation.push_back(r);
  if (r.hasQuery())group_.m_inthasquery = true;
  m_newrules = true;
  return *(group_.interpretation.end()-1);
}

LsysRule&
Lsystem::addRule( const std::string& rule, int type, size_t group_, int lineno, const ConsiderFilterPtr filter ){
  switch(type){
  case 1:
	return addDecompositionRule(rule,group_,lineno,filter);
	break;
  case 2:
	return addInterpretationRule(rule,group_,lineno,filter);
	break;
  default:
	return addProductionRule(rule,group_,lineno,filter);
	break;
  } 
}

void 
Lsystem::addProductionRule( const std::string& code, size_t group_, const ConsiderFilterPtr filter ){
  // ACQUIRE_RESSOURCE
  ContextMaintainer m(&m_context);
  LsysRule& r = addProductionRule(code,group_,-1,filter);
  r.compile();
  // RELEASE_RESSOURCE
}

void 
Lsystem::addDecompositionRule( const std::string& code, size_t group_, const ConsiderFilterPtr filter ){
  // ACQUIRE_RESSOURCE
  ContextMaintainer m(&m_context);
  LsysRule& r = addDecompositionRule(code,group_,-1,filter);
  r.compile();
  // RELEASE_RESSOURCE
}

void 
Lsystem::addInterpretationRule( const std::string& code, size_t group_, const ConsiderFilterPtr filter ){
  // ACQUIRE_RESSOURCE
  ContextMaintainer m(&m_context);
  LsysRule& r = addInterpretationRule(code,group_,-1,filter);
  r.compile();
  // RELEASE_RESSOURCE
}

void 
Lsystem::addRule(  const LsysRule& rule, int type, size_t groupid){
  switch(type){
  case 1:
    group(groupid).decomposition.push_back(rule);
    if (rule.hasQuery())group(groupid).m_dechasquery = true;
    break;
  case 2:
    if (!rule.isContextFree())LsysWarning("Interpretation rules should be context free. Contexts not supported for multiple iterations.");
    group(groupid).interpretation.push_back(rule);
    if (rule.hasQuery())group(groupid).m_inthasquery = true;
	break;
  default:
    group(groupid).production.push_back(rule);
    if (rule.hasQuery())group(groupid).m_prodhasquery = true;
	break;
  }
  m_newrules = true;
}

void Lsystem::addRule( const std::string& rule, int type, size_t group_, const ConsiderFilterPtr filter ){
	m_newrules = true;
    ContextMaintainer m(&m_context);
    LsysRule& r = addRule(rule,type,group_,-1,filter);
	r.compile();
}

bool 
Lsystem::empty( ) const {
  return m_rules.empty();
}

size_t 
Lsystem::nbProductionRules( size_t group_ ) const {
  if (m_rules.size() < group_) return 0;
  return group(group_).production.size();
}

size_t 
Lsystem::nbDecompositionRules( size_t group_ ) const {
  if (m_rules.size() < group_) return 0;
  return group(group_).decomposition.size();
}

size_t Lsystem::nbInterpretationRules( size_t group_ ) const {
  if (m_rules.size() < group_) return 0;
  return group(group_).interpretation.size();
}

size_t Lsystem::nbTotalRules(  ) const {
  size_t nbrules = 0;
  for(RuleGroupList::const_iterator it = m_rules.begin(); it != m_rules.end(); ++it)
        nbrules += it->production.size()+it->decomposition.size()+it->interpretation.size();
  return nbrules;
}

size_t Lsystem::nbGroups( ) const {
  return m_rules.size();
}

void Lsystem::addSubLsystem(const std::string& lfile)
{
	addSubLsystem(Lsystem(lfile));
}

void Lsystem::addSubLsystem(const Lsystem& sublsystem)
{
	printf("Add info from sublsystem '%s'",sublsystem.getFilename().c_str());
	context()->importContext(*sublsystem.context());
	size_t groupid = 0;
	for(std::vector<RuleGroup>::const_iterator itg = sublsystem.m_rules.begin(); itg != sublsystem.m_rules.end(); ++itg, ++groupid)
	{
		RuleGroup& rg = group(groupid);
		rg.production.insert(rg.production.end(), itg->production.begin(),itg->production.end());
		rg.decomposition.insert(rg.decomposition.end(),itg->decomposition.begin(),itg->decomposition.end());
		rg.interpretation.insert(rg.interpretation.end(), itg->interpretation.begin(),itg->interpretation.end());
		rg.m_prodhasquery = rg.m_prodhasquery & itg->m_prodhasquery;
		rg.m_dechasquery = rg.m_dechasquery & itg->m_dechasquery;
		rg.m_inthasquery = rg.m_inthasquery & itg->m_inthasquery;
	}

}


void 
Lsystem::setAxiom( const AxialTree& axiom ){
  ACQUIRE_RESSOURCE
  m_axiom = axiom;
  RELEASE_RESSOURCE
}


const AxialTree& 
Lsystem::getAxiom( ) const {
  return m_axiom;
}

pgl_hash_map_string<std::string> Lsystem::get_rule_fonction_table() const
{
  pgl_hash_map_string<std::string> result;
  for(RuleGroupList::const_iterator it = m_rules.begin(); it != m_rules.end(); ++it)
	{
		for(RuleSet::const_iterator itr = it->production.begin(); itr != it->production.end(); ++itr)
		{ result[itr->functionName()] = itr->name(); }
		for(RuleSet::const_iterator itr = it->decomposition.begin(); itr != it->decomposition.end(); ++itr)
		{ result[itr->functionName()] = itr->name(); }
		for(RuleSet::const_iterator itr = it->interpretation.begin(); itr != it->interpretation.end(); ++itr)
		{ result[itr->functionName()] = itr->name(); }
	}
  return result;
}

RulePtrMap Lsystem::getRules(eRuleType type, size_t groupid, eDirection direction, bool * hasQuery)
{
    if(hasQuery)*hasQuery = false;
 	size_t nbgroups = m_rules.size();
    if (groupid >= nbgroups) {
		if (nbgroups == 0) return RulePtrMap();
		else return getRules(type,0,direction,hasQuery);
	}
    RulePtrSet result;
    const RuleSet& rules = group(groupid).getGroup(type);
    for(RuleSet::const_iterator itr = rules.begin(); itr != rules.end(); ++itr)
        if(itr->isCompatible(direction)){
            result.push_back(&(*itr));
            if(hasQuery && itr->hasQuery())*hasQuery = true;
        }
    if (groupid > 0)
    {
        const RuleSet& rules = group(0).getGroup(type);
        for(RuleSet::const_iterator itr = rules.begin(); itr != rules.end(); ++itr)
            if(itr->isCompatible(direction)){
                result.push_back(&(*itr));
                if(hasQuery && itr->hasQuery())*hasQuery = true;
            }
    }
    return RulePtrMap(result,direction);
}

AxialTree Lsystem::debugStep(AxialTree& workingstring,
						  const RulePtrMap& ruleset,
						  bool query,
						  bool& matching,
						  eDirection direction,
						  Debugger& debugger){
  ContextMaintainer c(&m_context);
  matching = false;
  if( workingstring.empty()) return workingstring;
  AxialTree targetstring;
  targetstring.reserve(workingstring.size());
#ifndef LPY_NO_PLANTGL_INTERPRETATION
  if ( query )turtle_interpretation(workingstring,m_context.envturtle);
#endif
  debugger.begin(workingstring,direction);
  if ( direction == eForward){
      AxialTree::const_iterator _it = workingstring.begin();
      AxialTree::const_iterator _it3 = _it;
      AxialTree::const_iterator _endit = workingstring.end();

      while ( _it != _endit ) {
          if ( _it->isCut() )
              _it = workingstring.endBracket(_it);
          else{
              int match = 0;
			  const RulePtrSet& mruleset = ruleset[_it->getClassId()];
              for(RulePtrSet::const_iterator _it2 = mruleset.begin();
                  _it2 != mruleset.end(); _it2++){
					  ArgList args;
					  size_t prodlength;
                      if((*_it2)->match(workingstring,_it,targetstring,_it3,args)){
						  try {
							match = (*_it2)->applyTo(targetstring,args,&prodlength);
						  }catch(error_already_set){
							  if(!debugger.error_match(_it,_it3,targetstring,*_it2,args)){
								boost::python::throw_error_already_set();
							  }
							  else { PyErr_Clear(); }
							  match = -1;
						  }
						  if(match == 1) { 
							  if(debugger.shouldStop(_it,_it3,*_it2))debugger.total_match(_it,_it3,targetstring,prodlength,*_it2,args);
							  _it = _it3; break; 
						  }
						  else if(match == 0 && debugger.shouldStop(_it,_it3,*_it2))debugger.partial_match(_it,_it3,targetstring,*_it2,args);
                      }
              }
              if (match != 1){
                 targetstring.push_back(_it);
				 if(debugger.shouldStop(_it)) debugger.identity(_it,targetstring);
				 ++_it;
              }
              else matching = true;
          }
      }
  }
  else {
      AxialTree::const_iterator _it = workingstring.end()-1;
      AxialTree::const_iterator _it3 = _it;
      AxialTree::const_iterator _lastit = workingstring.begin();
      AxialTree::const_iterator _beg = workingstring.begin();
      AxialTree::const_iterator _end = workingstring.end();
      while ( _it !=  _end) {
          bool match = false;
		  const RulePtrSet& mruleset = ruleset[_it->getClassId()];
          for(RulePtrSet::const_iterator _it2 = mruleset.begin();
              _it2 != mruleset.end();  _it2++){
				  ArgList args;
				  size_t prodlength;
                  if((*_it2)->reverse_match(workingstring,_it,targetstring,_it3,args)){
					  try {
						match = (*_it2)->reverseApplyTo(targetstring,args,&prodlength);
					  }catch(error_already_set){
						if(!debugger.error_match(_it3==_end?_beg:_it3+1,_it+1,targetstring,*_it2,args))
								boost::python::throw_error_already_set();
						else { PyErr_Clear(); match = false; }
					  }
                      if(match) { 							  
						  if(debugger.shouldStop(_it3==_end?_beg:_it3+1,_it+1,*_it2))debugger.total_match(_it3==_end?_beg:_it3+1,_it+1,targetstring,prodlength,*_it2,args);
						  _it = _it3; break; 
					  }
 					  else if(debugger.shouldStop(_it3==_end?_beg:_it3+1,_it+1,*_it2))debugger.partial_match(_it3==_end?_beg:_it3+1,_it+1,targetstring,*_it2,args);
                 }
          }
          if (!match){
              targetstring.push_front(_it);
			  if(debugger.shouldStop(_it))debugger.identity(_it,targetstring);
              if(_it != _lastit) --_it;
			  else _it = _end;
          }
          else matching = true;
      } ;
  }
  debugger.end(targetstring);
  return targetstring;
}



AxialTree 
Lsystem::step(AxialTree& workingstring,
				const RulePtrMap& ruleset,
				bool query,
				bool& matching,
                eDirection direction){
  ContextMaintainer c(&m_context);
  matching = false;
  if( workingstring.empty()) return workingstring;
  AxialTree targetstring;
  targetstring.reserve(workingstring.size());
#ifndef LPY_NO_PLANTGL_INTERPRETATION
  if ( query )turtle_interpretation(workingstring,m_context.envturtle);
#endif
  if ( direction == eForward){
      AxialTree::const_iterator _it = workingstring.begin();
      AxialTree::const_iterator _it3 = _it;
      AxialTree::const_iterator _endit = workingstring.end();

      while ( _it != _endit ) {
          if ( _it->isCut() )
              _it = workingstring.endBracket(_it);
          else{
              bool match = false;
			  const RulePtrSet& mruleset = ruleset[_it->getClassId()];
              for(RulePtrSet::const_iterator _it2 = mruleset.begin();
                  _it2 != mruleset.end(); _it2++){
					  ArgList args;
                      if((*_it2)->match(workingstring,_it,targetstring,_it3,args)){
                          match = (*_it2)->applyTo(targetstring,args);
						  if(match) { _it = _it3; break; }
                      }
              }
              if (!match){
                 targetstring.push_back(_it);++_it;
              }
              else matching = true;
          }
      }
  }
  else {
      AxialTree::const_iterator _it = workingstring.end()-1;
      AxialTree::const_iterator _it3 = _it;
      AxialTree::const_iterator _lastit = workingstring.begin();
      AxialTree::const_iterator _beg = workingstring.begin();
      AxialTree::const_iterator _end = workingstring.end();
      while ( _it !=  _end) {
          bool match = false;
		  const RulePtrSet& mruleset = ruleset[_it->getClassId()];
          for(RulePtrSet::const_iterator _it2 = mruleset.begin();
              _it2 != mruleset.end();  _it2++){
				  ArgList args;
                  if((*_it2)->reverse_match(workingstring,_it,targetstring,_it3,args)){
                      match = (*_it2)->reverseApplyTo(targetstring,args);
                      if(match) { _it = _it3; break; }
                  }
          }
          if (!match){
              targetstring.push_front(_it);
              if(_it != _lastit) --_it;
			  else _it = _end;
          }
          else matching = true;
      }
  }
  return targetstring;
}

AxialTree 
Lsystem::stepWithMatching(AxialTree& workingstring,
				const RulePtrMap& ruleset,
				bool query,
                StringMatching& matching)
{
  ContextMaintainer c(&m_context);
  if( workingstring.empty()) return workingstring;
  AxialTree targetstring;
  targetstring.reserve(workingstring.size());
#ifndef LPY_NO_PLANTGL_INTERPRETATION
  if ( query )LPY::turtle_interpretation(workingstring,m_context.turtle);
#endif
  AxialTree::const_iterator _it = workingstring.begin();
  AxialTree::const_iterator _it3 = _it;
  AxialTree::const_iterator _endit = workingstring.end();
  size_t prodlength;
  matching.clear();
  while ( _it != _endit ) {
      if ( _it->isCut() )
          _it = workingstring.endBracket(_it);
      else{
          bool match = false;
		  const RulePtrSet& mruleset = ruleset[_it->getClassId()];
          for(RulePtrSet::const_iterator _it2 = mruleset.begin();
              _it2 != mruleset.end();  _it2++){
				  ArgList args;
                  if((*_it2)->match(workingstring,_it,targetstring,_it3,args)){
                      match = (*_it2)->applyTo(targetstring,args,&prodlength);
					  if (match){
						matching.append(distance(_it,_it3),prodlength);
						_it = _it3;
						break;
					  }
                  }
          }
          if (!match){              
              targetstring.push_back(_it);++_it;
              matching.addIdentity(1);
          }
      }
  }
  return targetstring;
}

AxialTree 
Lsystem::recursiveSteps(AxialTree& workingstring,
				          const RulePtrMap& ruleset, 
                          size_t maxdepth)
{
  ContextMaintainer c(&m_context);
  if( workingstring.empty()) return workingstring;
  AxialTree::const_iterator _it = workingstring.begin();
  AxialTree::const_iterator _it3 = _it;
  AxialTree::const_iterator _endit = workingstring.end();
  AxialTree targetstring;
  targetstring.reserve(workingstring.size());
  while ( _it != workingstring.end() ) {
      if ( _it->isCut() )
          _it = workingstring.endBracket(_it);
      else{
          AxialTree ltargetstring;
          bool match = false;
		  const RulePtrSet& mruleset = ruleset[_it->getClassId()];
          for(RulePtrSet::const_iterator _it2 = mruleset.begin();
              _it2 != mruleset.end(); _it2++){
				ArgList args;
                if((*_it2)->match(workingstring,_it,ltargetstring,_it3,args)){
                      match = (*_it2)->applyTo(ltargetstring,args);
					  if(match) { _it = _it3; break; }
                  }
          }
          if (match){
              if(maxdepth >1) {
                  targetstring += recursiveSteps(ltargetstring,ruleset,maxdepth-1);
              }
              else targetstring += ltargetstring;
          }
          else { targetstring.push_back(_it);++_it; }
      }
  }
  return targetstring;
}


AxialTree 
Lsystem::derive(  const AxialTree& wstring, 
                  size_t starting_iter , 
                  size_t nb_iter , 
                  bool previouslyinterpreted ){
  ACQUIRE_RESSOURCE
  enableEarlyReturn(false);
  if ( (m_rules.empty() || wstring.empty()) && m_context.return_if_no_matching )return wstring;
  ContextMaintainer c(&m_context);
  AxialTree res = derive(starting_iter,nb_iter,wstring,previouslyinterpreted);
  enableEarlyReturn(false);
  return res;
  RELEASE_RESSOURCE
}



AxialTree 
Lsystem::derive( size_t starting_iter , 
                    size_t nb_iter , 
                    const AxialTree& wstring, 
                    bool previouslyinterpreted){
  m_context.frameDisplay(true);
  AxialTree workstring = wstring;
  if(starting_iter == 0) {
	m_context.setIterationNb(0);
    apply_pre_process(workstring,false);
  }
  if ( (m_rules.empty() || workstring.empty()) && m_context.return_if_no_matching ){
	  if(starting_iter+nb_iter == m_max_derivation) {
		m_context.setIterationNb(m_max_derivation);
#ifndef LPY_NO_PLANTGL_INTERPRETATION
        apply_post_process(workstring,false);
#endif
	  }
	  return workstring;
  }
  if (!workstring.empty() && nb_iter > 0){
	bool matching = true;
	bool no_match_no_return = !m_context.return_if_no_matching;
	if(!m_rules.empty()||no_match_no_return){
      eDirection ndir;
      RulePtrMap production;
      bool productionHasQuery;
      RulePtrMap decomposition;
      bool decompositionHasQuery;
	  size_t i = 0;
      if(isEarlyReturnEnabled()) return workstring;
	  for(; (matching||no_match_no_return) && i < nb_iter; ++i){
#ifndef LPY_NO_PLANTGL_INTERPRETATION
		  if (m_context.isSelectionAlwaysRequired() || m_context.isSelectionRequested()){
			  std::vector<uint_t> sel;
			  if (m_context.isSelectionRequested()){
				  sel.push_back(waitSelection(m_context.getSelectionMessage()));
				  m_context.selectionAquired();
			  }
			  else sel = getSelection();
			  if (!sel.empty()) {
				  uint_t added = 0;
				  size_t wstrsize = workstring.size();
				  std::sort(sel.begin(),sel.end());
				  for(std::vector<uint_t>::const_iterator it = sel.begin(); it != sel.end(); ++it)
				  {
					  if(*it < wstrsize){
						  workstring.insertAt(*it+added,ParamModule("X"));
						  added+=1;
					  }
				  }

			  }
		  }
		  m_lastcomputedscene = ScenePtr();
#endif
		  m_context.frameDisplay(i == (nb_iter -1));
		  m_context.setIterationNb(starting_iter+i);
          apply_pre_process(workstring,true);
		  eDirection dir = getDirection();
		  size_t group_ = m_context.getGroup();
		  if (group_ > m_rules.size()) LsysWarning("Group not valid.");
		  if (i == 0 || dir != ndir || group_ != m_currentGroup || m_newrules){
			  ndir = dir;
			  m_currentGroup = group_;
			  production = getRules(eProduction,group_,ndir,&productionHasQuery);
			  decomposition = getRules(eDecomposition,group_,ndir,&decompositionHasQuery);
			  m_newrules = false;
		  }
		  if (!production.empty()){
			  if(!hasDebugger())
				  workstring = step(workstring,production,previouslyinterpreted?false:productionHasQuery,matching,dir);
			  else workstring = debugStep(workstring,production,previouslyinterpreted?false:productionHasQuery,matching,dir,*m_debugger);
			  previouslyinterpreted = false;
		  }
		  if(!decomposition.empty()){
			  bool decmatching = true;
			  for(size_t i = 0; decmatching && i < m_decomposition_max_depth; i++){
				  workstring = step(workstring,decomposition,previouslyinterpreted?false:decompositionHasQuery,decmatching,dir);
				  previouslyinterpreted = false;
				  if (decmatching) matching = true;
			  }
		  }
		  // Call endeach function
#ifndef LPY_NO_PLANTGL_INTERPRETATION
		  if(m_context.hasEndEachFunction())
			m_lastcomputedscene = apply_post_process(workstring);
#endif
		  if(isEarlyReturnEnabled())  break;
#ifndef LPY_NO_PLANTGL_INTERPRETATION
		  if( (i+1) <  nb_iter && m_context.isSelectionRequested()) {
			 plot(workstring,true);
		  }
#endif
	  }
#ifndef LPY_NO_PLANTGL_INTERPRETATION
	  if(starting_iter+i == m_max_derivation) {
		  // Call end function
		  if(m_context.hasEndFunction())
			m_lastcomputedscene = apply_post_process(workstring,false);
	  }
#endif
	}
  }
  return workstring;
}

void 
Lsystem::apply_pre_process(AxialTree& workstring, bool starteach)
{
	// Call endeach function
	object result;
	switch (starteach?m_context.getStartEachNbArgs():m_context.getStartNbArgs()){
		default:
		case 0:
			result = starteach ? m_context.startEach() : m_context.start();
			break;
		case 1:
			result = starteach ? m_context.startEach(workstring) : m_context.start(workstring);
			break;
	}
	// Check result of starteach function
    if (result != object())
	    workstring = extract<AxialTree>(result)();
}


AxialTree 
Lsystem::interpret(AxialTree& wstring){
  ACQUIRE_RESSOURCE
  return homomorphism(wstring);
  RELEASE_RESSOURCE
}

AxialTree 
Lsystem::homomorphism(AxialTree& wstring){
  if ( wstring.empty() || m_rules.empty() || 
       ( group(0).interpretation.empty() && 
        (group(m_currentGroup).interpretation.empty()||
         m_rules.size() < m_currentGroup)))return wstring;
  AxialTree workstring;
  bool homHasQuery = false;  
  RulePtrMap interpretation = getRules(eInterpretation,m_currentGroup,eForward,&homHasQuery);
  if (!interpretation.empty()){
      workstring = recursiveSteps(wstring,interpretation,m_interpretation_max_depth);
  }
  return workstring;
}

std::string conv_number(size_t num, size_t fill){
  std::stringstream ss;
  ss.fill('0');
  ss.width(fill);
  ss << num;
  return ss.str();
}

// ===============================================================================

#ifndef LPY_NO_PLANTGL_INTERPRETATION

template<class Interpreter>
void Lsystem::gRecursiveInterpretation(AxialTree& workingstring,
										 const RulePtrMap& ruleset,
										 Interpreter& interpreter,
										 size_t maxdepth,
										 bool withid)
{
  ContextMaintainer c(&m_context);
  if( workingstring.empty()) return ;
  AxialTree::iterator _itn = workingstring.begin();

  AxialTree::const_iterator _it = workingstring.begin();
  AxialTree::const_iterator _it3 = _it;
  AxialTree::const_iterator _endit = workingstring.end();
  size_t dist = 0;
  if (withid)  {
      interpreter.init();
      AxialTree initturtle = m_context.startInterpretation();
      for(AxialTree::iterator _itl = initturtle.begin(); _itl != initturtle.end(); ++_itl)
            interpreter.interpret(_itl);  
      interpreter.start();
  }
  while ( _it != _endit && !interpreter.earlyReturn() ) {
      if ( _it->isCut() ){
	      _it3 = _it;
          _it = workingstring.endBracket(_it3);
          dist = distance(_it3,_it);
          _itn += dist;
          if(withid)interpreter.incId(dist);
      }
      else{
          AxialTree ltargetstring;
          bool match = false;
		  const RulePtrSet& mruleset = ruleset[_it->getClassId()];
          for(RulePtrSet::const_iterator _it2 = mruleset.begin();
              _it2 != mruleset.end(); _it2++){
				  ArgList args;
                  if((*_it2)->match(workingstring,_it,ltargetstring,_it3,args)){
                      match = (*_it2)->applyTo(ltargetstring,args);
					  if (match) {
						dist = distance(_it,_it3);
						_it = _it3;
						_itn += dist;
						break;
					  }
                  }
          }
          if (match){
              if(maxdepth > 1) m_gRecursiveInterpretation<Interpreter>(ltargetstring,ruleset,interpreter,maxdepth-1,false);
              else { 
                 for(AxialTree::iterator _itl = ltargetstring.begin();
					 _itl != ltargetstring.end(); ++_itl){
					 interpreter.interpret(_itl);
                 } 
              }
              if(withid)interpreter.incId(dist); 
          }
          else { 
			  interpreter.interpret(_itn);
			  if(withid) interpreter.incId();
			  ++_it; ++_itn;
          }
      }
  }
  if (withid)  {
      interpreter.finalize();
      AxialTree finishturtle = m_context.endInterpretation();
      for(AxialTree::iterator _itl = finishturtle.begin(); _itl != finishturtle.end(); ++_itl)
            interpreter.interpret(_itl);  
      interpreter.stop();
  }
}

	

	struct TurtleInterpreter {
		TurtleInterpreter(Turtle& t) : turtle(t) {}
		Turtle& turtle;

		static inline bool earlyReturn() { return false; }

        inline void init() 
        { turtle.start(); turtle.setNoId(); }

        inline void finalize() 
        { turtle.setNoId(); }
        
        
        inline void start() 
        { turtle.setId(0); }
        
		inline void stop()  
		{ turtle.stop();
		  if (!turtle.emptyStack()){
			printf("Turtle stack size : %i\n",turtle.getStack().size());
			LsysError("Ill-formed string in interpretation: unmatched brackets");
			}
		}

		inline void interpret(AxialTree::iterator m){
			  m->interpret(turtle);
		}
		inline void incId(size_t nb = 1){
              turtle.incId(nb); 
		}
	};

void 
Lsystem::recursiveInterpretation(AxialTree& workingstring,
				                const RulePtrMap& ruleset,
                                Turtle& t,
                                size_t maxdepth)
{


	TurtleInterpreter i (t);
	m_gRecursiveInterpretation<TurtleInterpreter>(workingstring,ruleset,i,maxdepth);
}

	struct TurtleStepInterpreter {
		TurtleStepInterpreter(PglTurtle& t, LsysContext& c) : turtle(t), context(c), timer(c.get_animation_timestep()) {}

		PglTurtle& turtle;
		LsysContext& context;
		TOOLS(Sequencer) timer;

		inline bool earlyReturn() { return context.isEarlyReturnEnabled(); }


        inline void init() 
        { turtle.start(); turtle.setNoId(); }

        inline void finalize() 
        { turtle.setNoId(); }

		inline void start() 
		{ turtle.setId(0); context.enableEarlyReturn(false); }

		inline void stop()  
		{ 
			if(!context.isEarlyReturnEnabled()){
				turtle.stop();
				if (!turtle.emptyStack()){
					printf("Turtle stack size : %i\n",turtle.getStack().size());
					LsysError("Ill-formed string in interpretation: unmatched brackets");
				}
				else {
					timer.touch();
					LPY::plot(turtle.getScene());
					LPY::displayMessage("");
				}
			}
			else { turtle.reset(); }
		}

		inline void interpret(AxialTree::iterator m){
			 m->interpret(turtle);
			 timer.touch();
			 if (m->getClass() && m->getClass()->isPredefined()){
				LPY::plot(turtle.partialView());
			 }
			 LPY::displayMessage(m->str());
			 timer.setTimeStep(context.get_animation_timestep());
		}

		inline void incId(size_t nb = 1){
              turtle.incId(nb); 
		}
	};
void 
Lsystem::recursiveStepInterpretation(AxialTree& workingstring,
				                const RulePtrMap& ruleset,
                                PglTurtle& t,
                                size_t maxdepth)
{



	TurtleStepInterpreter i(t,m_context);
	m_gRecursiveInterpretation<TurtleStepInterpreter>(workingstring,ruleset,i,maxdepth);
}


/*

void 
Lsystem::recursiveInterpretation(AxialTree& workingstring,

				                const RulePtrMap& ruleset,

                                Turtle& t,
                                size_t maxdepth, bool withid)

{ 
  ContextMaintainer c(&m_context);

  if( workingstring.empty()) return ;
  AxialTree::iterator _itn = workingstring.begin();
  AxialTree::const_iterator _it = workingstring.begin();

  AxialTree::const_iterator _it3 = _it;
  AxialTree::const_iterator _endit = workingstring.end();

  size_t dist = 0;
  if(withid){ 

      t.start();
      t.setId(0);
  }
  while ( _it != _endit ) {

      if ( _it->isCut() ){
	  _it3 = _it;
          _it = workingstring.endBracket(_it3);

          _itn += distance(_it3,_it);
      }
      else{

          AxialTree ltargetstring;
          bool match = false;
		  const RulePtrSet& mruleset = ruleset[_it->getClassId()];
          for(RulePtrSet::const_iterator _it2 = mruleset.begin();

              _it2 != mruleset.end(); _it2++){
				  ArgList args;
                  if((*_it2)->match(workingstring,_it,ltargetstring,_it3,args)){

                      match = (*_it2)->applyTo(ltargetstring,args);
					  if (match) {
						dist = distance(_it,_it3);
						_it = _it3;

						_itn += dist;
						break;
					  }

                  }
          }
          if (match){
              if(maxdepth > 1) recursiveInterpretation(ltargetstring,ruleset,t,maxdepth-1,false);

              else { 
                 for(AxialTree::iterator _itl = ltargetstring.begin();

					 _itl != ltargetstring.end(); ++_itl){
					 _itl->interpret(t);
                 } 
              }

              if(withid)t.incId(dist); 
          }
          else { 
			  ParamModule& m = *_itn; // const_cast<ParamModule&>(*_it);

			  m.interpret(t);
			  ++_it; ++_itn;
              if(withid)t.incId(); 
          }
      }

  }
  if(withid){ 
      t.stop();

      if (!t.emptyStack()){
        printf("Turtle stack size : %i\n",t.getStack().size());
	    LsysError("Ill-formed string in interpretation: unmatched brackets");
      }
  }
}

*/

ScenePtr
Lsystem::apply_post_process(AxialTree& workstring, bool endeach)
{
	// Call endeach function
	object result;
	ScenePtr rep;
	switch (endeach?m_context.getEndEachNbArgs():m_context.getEndNbArgs()){
		default:
		case 0:
			result = endeach ? m_context.endEach() : m_context.end();
				break;
		case 1:
			result = endeach ? m_context.endEach(workstring) : m_context.end(workstring);
				break;
		case 2:
				// if a frame should be displayed, representation is computed
				if(m_context.isFrameDisplayed()) {
					turtle_interpretation(workstring,m_context.turtle);
					rep = m_context.turtle.getScene();
				}
				result = endeach ? m_context.endEach(workstring,rep) : m_context.end(workstring,rep);
				break;
	}
	// Check result of endeach function
	if(PyTuple_Check(result.ptr()) && len(result) >= 2){
		if (result[0] != object()) {
            workstring = extract<AxialTree>(result[0])();
        }
		if (result[1] == object()) rep = ScenePtr();
		else rep = extract<ScenePtr>(result[1])();
	}
	else if (result != object()){
		extract<ScenePtr> scextract(result);
		if(scextract.check())rep = scextract();
		else {
			workstring = extract<AxialTree>(result)();
			rep = ScenePtr();
		}
	}
	return rep;
}


void 
Lsystem::plot( AxialTree& workstring, bool checkLastComputedScene ){
    ACQUIRE_RESSOURCE
    plot(workstring,checkLastComputedScene);
    RELEASE_RESSOURCE
}

void 
Lsystem::turtle_interpretation( AxialTree& workstring, PGL::Turtle& t )
{
  ACQUIRE_RESSOURCE
  turtle_interpretation(workstring,t);
  RELEASE_RESSOURCE
}

ScenePtr Lsystem::sceneInterpretation( AxialTree& workstring )
{
  ACQUIRE_RESSOURCE
  turtle_interpretation(workstring,m_context.turtle);
  return m_context.turtle.getScene();
  RELEASE_RESSOURCE
}

void Lsystem::stepInterpretation(AxialTree& wstring)
{
  ACQUIRE_RESSOURCE
  if ( wstring.empty() )return;
  bool homHasQuery = false;
  RulePtrMap interpretation = getRules(eInterpretation,m_currentGroup,eForward,&homHasQuery);
  recursiveStepInterpretation(wstring,interpretation,m_context.turtle,m_interpretation_max_depth);
  RELEASE_RESSOURCE
}

void
Lsystem::turtle_interpretation(AxialTree& wstring, PGL::Turtle& t){
    if ( wstring.empty() )return;
    bool homHasQuery = false;
    RulePtrMap interpretation = getRules(eInterpretation,m_currentGroup,eForward,&homHasQuery);
    if (!interpretation.empty()){
      recursiveInterpretation(wstring,interpretation,t,m_interpretation_max_depth);
    }
    else {
        t.start();
        t.setNoId();
        AxialTree initturtle = m_context.startInterpretation();
        for(AxialTree::iterator _itl = initturtle.begin(); _itl != initturtle.end(); ++_itl)
            _itl->interpret(t);  

        t.setId(0);
  		LPY::turtle_do_interpretation(wstring,t);

        t.setNoId();
        AxialTree finalizeturtle = m_context.endInterpretation();
        for(AxialTree::iterator _itl = finalizeturtle.begin(); _itl != finalizeturtle.end(); ++_itl)
            _itl->interpret(t);  
    }
}

void 
Lsystem::plot( AxialTree& workstring, bool checkLastComputedScene){
	ScenePtr result;
	if (checkLastComputedScene) {
		result = m_lastcomputedscene;
	}
	if (is_null_ptr(result)) {
		turtle_interpretation(workstring,m_context.turtle);
		result = m_context.turtle.getScene();
	}
    LPY::plot(result);
	m_context.postDraw();
}

AxialTree
Lsystem::animate(const AxialTree& workstring,double dt,size_t beg,size_t nb_iter){
    ACQUIRE_RESSOURCE
    ContextMaintainer c(&m_context);
    enableEarlyReturn(false);
    AxialTree tree = workstring;
    m_context.set_animation_timestep(dt);
	m_context.setAnimationEnabled(true);
    Sequencer timer(dt);
    timer.touch();
    plot(tree);
    if (nb_iter > 0 && !isEarlyReturnEnabled()){
	  for (size_t i = beg; i < beg+nb_iter; i++){
	    tree = derive(i,1,tree,true);
		if(m_context.isFrameDisplayed()) {
			timer.touch();
			plot(tree,true);
		}
        timer.setTimeStep(m_context.get_animation_timestep());
        if(isEarlyReturnEnabled()) break;
	  }
	}
	m_context.setAnimationEnabled(false);
    enableEarlyReturn(false);
    return tree;
    RELEASE_RESSOURCE
}

void
Lsystem::record(const std::string& prefix,
				const AxialTree& workstring,
				size_t beg, size_t nb_iter){
    ACQUIRE_RESSOURCE
    enableEarlyReturn(false);
    AxialTree tree = workstring;
    ContextMaintainer c(&m_context);
	m_context.setAnimationEnabled(true);
    plot(tree);
	int fill = (int)ceil(log10((float)beg+nb_iter+1));
	LPY::saveImage(prefix+conv_number(beg,fill)+".png");
    if (nb_iter > 0){
	  for (size_t i = beg+1; i <= beg+nb_iter; i++){
		tree = derive(i-1,1,tree,true);
		if(m_context.isFrameDisplayed()) {
			plot(tree,true);
		}
		LPY::saveImage(prefix+conv_number(i,fill)+".png");
        if(isEarlyReturnEnabled()) break;
	  }
	}
	m_context.setAnimationEnabled(false);
    enableEarlyReturn(false);
    RELEASE_RESSOURCE
}

#endif
// ===============================================================================


void Lsystem::enableEarlyReturn(bool val) 
{ 
	m_context.enableEarlyReturn(val);
}

bool Lsystem::isEarlyReturnEnabled() 
{ 
	return m_context.isEarlyReturnEnabled();
}


#ifdef MULTI_THREADED_LSYSTEM
void Lsystem::acquire() const
{
   if (!m_ressource->mutex.tryLock()){
       // std::cerr << "Concurrent Access of the Lsystem" << std::endl;
	   LsysError("Concurrent Access of the Lsystem");
   }
}

void Lsystem::release() const
{    
   m_ressource->mutex.unlock();
}
#endif

bool Lsystem::isRunning() const
{
#ifdef MULTI_THREADED_LSYSTEM
   if (!m_ressource->mutex.tryLock()) return true;
   m_ressource->mutex.unlock();
   return false;
#else
   return false;
#endif
}

void 
Lsystem::forceRelease(){
#ifdef MULTI_THREADED_LSYSTEM
	if(isRunning()) release();
#endif
}
/*---------------------------------------------------------------------------*/

Lsystem::Debugger::~Debugger()  { }

/*---------------------------------------------------------------------------*/
