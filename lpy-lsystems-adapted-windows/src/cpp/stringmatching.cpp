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

#include "stringmatching.h"
#include "tracker.h"
#include <iostream>

LPY_USING_NAMESPACE

/*---------------------------------------------------------------------------*/

StringMatching::const_iterator::const_iterator()
{
}

void StringMatching::const_iterator::increment(){
    ++m_target;
    if (m_iter == m_end)++m_original;
    else {
        if ((m_iter+1) == m_end){
            if (m_iter->idpolicy == eIncreasing)++m_original;
        }
        else {
            if(m_target == (m_iter+1)->target){
                while ( ((m_iter+1) != m_end) && (m_target == (m_iter+1)->target) ){
                    ++m_iter;
                    m_original = m_iter->original;
                }
            }
            else {
                if (m_iter->idpolicy == eIncreasing)++m_original;
            }
        }
    }
}

bool StringMatching::const_iterator::isOnMark() const
{
    if (m_iter == m_end)return false;
    return (m_target == m_iter->target);
}

StringMatching::eIdPolicy 
StringMatching::const_iterator::currentIdPolicy() const
{
    if (m_iter == m_end)return (m_iter-1)->idpolicy;
    return m_iter->idpolicy;
}

/*---------------------------------------------------------------------------*/

StringMatching::StringMatching(){
    m_matching.push_back(StringMark(0,0,eIncreasing));
	IncTracker(StringMatching)
}

StringMatching::~StringMatching()
{
	DecTracker(StringMatching)
}

void StringMatching::clear()
{
    m_matching.clear();
    m_matching.push_back(StringMark(0,0,eIncreasing));
}
/*---------------------------------------------------------------------------*/

StringMatching::const_iterator StringMatching::begin() const
{
    StringMatching::const_iterator it;
    it.m_iter = m_matching.begin();
    it.m_end = m_matching.end();
    if(it.m_iter != it.m_end){
        size_t firstval = it.m_iter->target;
        while((it.m_iter+1) != it.m_end && (it.m_iter+1)->target == firstval)++it.m_iter;
    }
    it.m_target = it.m_iter->target;
    it.m_original = it.m_iter->original;
    return it;
}

StringMatching::const_iterator StringMatching::end() const
{
    StringMatching::const_iterator it;
    it.m_iter = m_matching.end();
    it.m_end = m_matching.end();
    it.m_original = (it.m_iter-1)->original;
    it.m_target = (it.m_iter-1)->target;
    return it;
}

/*---------------------------------------------------------------------------*/

void StringMatching::append(size_t original, size_t target)
{
    StringMarkList::iterator it = m_matching.end()-1;
    if(target == 1 && original == 1){
        addIdentity(original);
    }
    else {
        it->idpolicy = eFixed;
        m_matching.push_back(StringMark(it->original+original,it->target+target,eIncreasing));
    }
}

void StringMatching::addIdentity(size_t length)
{
    StringMarkList::iterator it = m_matching.end()-1;
    if (it == m_matching.begin()){
        it->idpolicy = eIncreasing;
        m_matching.push_back(StringMark(it->original+length,it->target+length,eIncreasing));
    }
    else {
        StringMarkList::const_iterator itp = it-1;
        if( itp->idpolicy == eIncreasing){
            it->target+=length;
            it->original+=length;
        }
        else {
            it->idpolicy = eIncreasing;
            m_matching.push_back(StringMark(it->original+length,it->target+length,eIncreasing));
        }
    }
}

bool StringMatching::isIdentity() const
{
    if (m_matching.empty()) return true;
    else {
        bool identity = true;
        for(StringMarkList::const_iterator it = m_matching.begin();it != m_matching.end();++it)
            if( it->idpolicy == eFixed)return false;
        return true;
    }
}

StringMatching StringMatching::operator+(const StringMatching& newmatching)
{
   StringMatching matching;
   // StringMark::const_iterator itn = newmatching.begin();
   StringMarkList::const_iterator itn = newmatching.m_matching.begin();
   // matching.push_back(m_matching.begin());
   for(StringMarkList::const_iterator it = m_matching.begin();it != m_matching.end();++it){
       StringMark s;
       while (itn->original < it->target)++itn;
       --itn;
       if (itn->original == it->target){
           s.original = it->original;
           s.target = itn->target;
       }

       if( it->idpolicy == eFixed){
           
       }
       else {
       }
   }
   return matching;
}

/*---------------------------------------------------------------------------*/
