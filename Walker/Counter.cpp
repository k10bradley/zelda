////////////////////////////////////////////////////////////////////////////////////////////////////////
// Counter.cpp
//
// Created By: Ten Bradley
// Date: 2019.
//
// Walks Clang's AST to determine the number of occurrances of various
// expressions and statements.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include "clang/AST/Mangle.h"
#include "Counter.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../Graph/ZeldaNode.h"
#include "clang/AST/DeclCXX.h"
#include <fstream>
#include <sstream>

using namespace std;

CountTypes Counter::tries;
CountTypes Counter::catches;
CountTypes Counter::nonExceptional;

CountTypes::CountTypes(){
  occurs["try"];
  occurs["throw"];
  occurs["catch"];
  occurs["switch"];
  occurs["if"];
  occurs["while"];
  occurs["do"];
  occurs["for"];
  occurs["return"];
  occurs["continue"];
  occurs["break"];
  occurs["delete"];
}

int& CountTypes::get(const std::string& s) { return occurs[s]; }

const map<string,int> CountTypes::getMap() const { return occurs; }
/**
 * Constructor
 * @param Context AST Context
 */
Counter::Counter(ASTContext *Context) : ParentWalker(Context) {}

/**
 * Destructor
 */

bool Counter::TraverseFunctionDecl(FunctionDecl* func){
  if ( ! isInSystemHeader(func) ) {
    if ( func->isThisDeclarationADefinition() ){
      current.emplace_back(&nonExceptional);
      TraverseStmt(func->getBody());
      current.pop_back();
    }
  }
  return true;
}

bool Counter::TraverseCXXTryStmt(CXXTryStmt* stmt){
  if ( ! isInSystemHeader(stmt) ){
    current.emplace_back(&tries);
    VisitCXXTryStmt(stmt);
    for( auto child: stmt->children() ) { TraverseStmt(child); } 
    current.pop_back();
  }
  return true;
}

bool Counter::TraverseCXXCatchStmt(CXXCatchStmt* stmt){
  if ( ! isInSystemHeader(stmt) ){
    current.emplace_back(&catches);
    VisitCXXCatchStmt(stmt);
    for( auto child: stmt->children() ) { TraverseStmt(child); } 
    current.pop_back();
  }
  return true;
}

bool Counter::VisitCXXTryStmt(CXXTryStmt*){
  return addCount("try");
}

bool Counter::VisitCXXCatchStmt(CXXCatchStmt*){
  return addCount("catch");
}



bool Counter::VisitCXXThrowExpr(CXXThrowExpr*){
  return addCount("throw");
}


bool Counter::VisitSwitchCase(SwitchCase*){
  return addCount("switch");
}

bool Counter::VisitIfStmt(IfStmt*){
  return addCount("if");
}

bool Counter::VisitWhileStmt(WhileStmt*){
  return addCount("while");
}

bool Counter::VisitDoStmt(DoStmt*){
  return addCount("do");
}

bool Counter::VisitForStmt(ForStmt*){
  return addCount("for");
}

bool Counter::VisitReturnStmt(ReturnStmt*){
  return addCount("return");
}

bool Counter::VisitContinueStmt(ContinueStmt*){
  return addCount("continue");
}

bool Counter::VisitBreakStmt(BreakStmt*){
  return addCount("break");
}

bool Counter::VisitCXXDeleteExpr(CXXDeleteExpr*){
  return addCount("delete");
}


bool Counter::addCount(const std::string& s){
  if ( ! current.empty() ){
    ++((*current.back()).get(s));
  }
  return true;
}

void Counter::printData(int exceptions, std::ostream& out ){

  CountTypes* toPrint = nullptr;
  if ( exceptions == 0 ) toPrint = &tries;
  else if ( exceptions == 1 ) toPrint = &catches;
  else toPrint = &nonExceptional;

  
  out << *toPrint; 
}

std::ostream& operator<<(std::ostream& out, const CountTypes& ct){
  for ( auto elem : ct.getMap()  ){
    out << elem.first <<"," << elem.second << endl; 
  }
  return out;
}
