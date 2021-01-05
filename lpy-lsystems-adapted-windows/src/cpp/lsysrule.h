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

#include "axialtree.h"
#include "patternstring.h"
#include "argcollector.h"
#include "paramproduction.h"
#include "lstringmatcher.h"
#include "consider.h"

LPY_BEGIN_NAMESPACE

/*---------------------------------------------------------------------------*/

class LPY_API LsysRule {

public:

	// LsysRule();
	LsysRule(const LsysRule&);
	LsysRule(size_t = 0, size_t = 0, char prefix = 'p', int lineno = -1);
	~LsysRule();

	void set( const std::string& code );

	void consider(const ConsiderFilterPtr consider);
	void consider(const std::string& modules);
	void ignore(const std::string& modules);
	ConsiderFilterPtr getConsiderFilter() const { return m_consider; }

	inline size_t getId() const { return m_id; }
	inline void setId(size_t id) { m_id = id; }

	inline size_t getGroupId() const { return m_gid; }
	inline void setGroupId(size_t id) { m_gid = id; }

	AxialTree apply(bool * isApplied = NULL) const;
	AxialTree apply( const ArgList& args, bool * isApplied = NULL ) const ;
//	boost::python::object apply( const boost::python::tuple& args ) const;

	inline bool isCompiled() const {  return m_function != boost::python::object(); }
	void compile();
	void recompile();
	void importPyFunction();

	void clear();
	
	inline const PatternString& predecessor() const
	{ return m_predecessor; }
	
	inline const PatternString& leftContext() const
	{ return m_leftcontext; }
	
	inline const PatternString& newLeftContext() const
	{ return m_newleftcontext; }
	
	inline const PatternString& rightContext() const
	{ return m_rightcontext; }
	
	inline const PatternString& newRightContext() const
	{ return m_newrightcontext; }
	
	inline const std::string& definition() const
	{ return m_definition; }
	
	inline const std::vector<std::string>& formalParameters() const
	{ return m_formalparameters; }
	
	inline size_t nbParameters() const 
	{ return m_nbParams; }
	
	size_t nbContexts() const;

	inline bool isContextFree() const
	{ return nbContexts() == 0; }

	inline bool hasQuery() const
	{ return m_hasquery; }

	inline const boost::python::object& function() const
	{ return m_function; }

    inline bool forwardCompatible() const 
        { return m_newrightcontext.empty(); }

    inline bool backwardCompatible() const 
        { return m_newleftcontext.empty(); }

    inline bool isCompatible(eDirection direction) const 
        { return (direction == eForward? forwardCompatible() : backwardCompatible()); }

	bool match(const AxialTree& src,
			   AxialTree::const_iterator pos,
			   const AxialTree& dest,
			   AxialTree::const_iterator& endpos,
			   ArgList& args,
               eDirection direction = eForward) const ;

    inline bool reverse_match(const AxialTree& src,
			   AxialTree::const_iterator pos,
			   const AxialTree& dest,
			   AxialTree::const_iterator& endpos,
               ArgList& args) const 
    { return match(src,pos,dest,endpos,args,eBackward); }

	bool applyTo( AxialTree& dest, 
				  const ArgList& args, 
				  size_t * length = NULL,
				  eDirection direction = eForward) const;

	inline bool reverseApplyTo( AxialTree& dest, 
				  const ArgList& args, 
				  size_t * length = NULL,
				  eDirection direction = eForward) const
	{ return applyTo(dest,args,length,eBackward); }

	AxialTree process( const AxialTree& src ) const;

	std::string str() const ;

	std::string functionName() const ;
	std::string callerFunctionName() const ;
	std::string name() const ;
	std::string uname() const ;
	
	std::string getCode() ;
	std::string getCoreCode() ;
	std::string getCallerCode() const;

	void setStatic();
	void keepOnlyRelevantVariables();

	int redundantParameter() const;

	int lineno;
	uint32_t getCodeLength() const { return m_codelength; }

	inline bool isStatic() const { return m_isStatic; }
	inline AxialTree getStaticProduction() const { return m_staticResult; }

protected:

	void parseHeader( const std::string& name);
	void parseParameters();
	void initStaticProduction();

	size_t m_id;
	size_t m_gid;
    char m_prefix;
	PatternString m_predecessor;
	PatternString m_leftcontext;
	PatternString m_newleftcontext;
	PatternString m_rightcontext;
	PatternString m_newrightcontext;
	std::vector<std::string> m_formalparameters;
	size_t m_nbParams;
	std::string m_definition;
	boost::python::object m_function;
	bool m_hasquery;
	bool m_isStatic;
	AxialTree m_staticResult;
	uint32_t m_codelength;
	ConsiderFilterPtr m_consider;
	LstringMatcherPtr m_lstringmatcher;

private:
    void precall_function( size_t nbargs = 0 ) const;
    void precall_function( size_t nbargs,  const ArgList& obj ) const;
    boost::python::object call_function( size_t nbargs,  const ArgList& obj ) const;
    AxialTree postcall_function( boost::python::object, bool * isApplied = NULL ) const;

};

/*---------------------------------------------------------------------------*/

typedef std::vector<LsysRule> RuleSet;
typedef std::vector<const LsysRule *> RulePtrSet;

/*---------------------------------------------------------------------------*/

class RulePtrMap {
public:
	typedef std::vector<RulePtrSet> RulePtrSetMap;

	RulePtrMap();
	RulePtrMap(const RulePtrSet& rules, eDirection direction = eForward);

	inline const RulePtrSet& operator[](size_t id) const 
	{ return (id < m_maxsmb?m_map[id]:m_defaultset); }
	inline bool empty() const {  return m_nbrules == 0; }
	inline size_t size() const { return m_nbrules; }

protected:
	RulePtrSetMap m_map;
	RulePtrSet m_defaultset;
	size_t m_nbrules;
	size_t m_maxsmb;

};

/*---------------------------------------------------------------------------*/

LPY_END_NAMESPACE
