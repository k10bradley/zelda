////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExceptWalker.cpp
//
// Created By: Ten Bradley
// Date: 2019.
//
// Walks Clang AST to fine all expressions and statements related to
// exception handling and their locations 
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
#include "ExceptWalker.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../Graph/ZeldaNode.h"
#include "clang/AST/DeclCXX.h"
#include <fstream>

using namespace std;

/**
 * Constructor
 * @param Context AST Context
 */
ExceptWalker::ExceptWalker(ASTContext *Context) : ParentWalker(Context), catchOut{"simple/catchOccur.csv", std::ios_base::app},
  throwOut{"simple/throwOccur.csv", std::ios_base::app},tryOut{"simple/tryOut.csv", std::ios_base::app} { }

/**
 * Destructor
 */
ExceptWalker::~ExceptWalker(){ }

std::string ExceptWalker::QualTypeString(const QualType& qt){
  return qt.getAsString();
}

bool ExceptWalker::VisitFunctionDecl(FunctionDecl* decl){
  if ( decl->isThisDeclarationADefinition() &&  decl->hasBody() ){
    currFunction = decl;
    functionName = getCurrFunctionName();
  }
  return true;
}

bool ExceptWalker::VisitCXXMethodDecl(CXXMethodDecl* decl){
  if ( decl->isThisDeclarationADefinition() &&  decl->hasBody() ){
    currFunction = decl;
    string thisType;
    thisType = QualTypeString(decl->getThisType(*Context));
    thisType = thisType.substr(0,thisType.length()-2) + "::";
    functionName = thisType + getCurrFunctionName();
  }
  return true;
}

string ExceptWalker::getCurrFunctionName(){
  string func;
  if ( currFunction && currFunction->getDeclName() ){
    func = currFunction->getNameInfo().getName().getAsString();
    func += "(";
    bool first = true;
    for ( auto it = currFunction->param_begin(); it != currFunction->param_end(); ++it ){
      if ( first ) first = false;
      else func += ", ";
      func += QualTypeString( (*it)->getOriginalType() ); 
    }
    func += ")";
  }
  return func;
}

/**
 * Visits statements for Zelda components.
 * @param statement The statement to visit.
 * @return Whether we should continue.
 */
bool ExceptWalker::VisitCXXThrowExpr(CXXThrowExpr* expr) {
  Stmt* sub = expr->getSubExpr();
  throwOut << functionName << ";throw expr; ";
  string type;
  if ( sub ){
    type = sub->getStmtClassName();
    throwOut << sub->getStmtClassName() << endl;
  } else {
    type = "rethrow";
    throwOut << "rethrow" << endl;
  }
  catchTypes[type] += 1;
  return true;
}

bool ExceptWalker::VisitCXXCatchStmt(CXXCatchStmt* stmt){
  catchOut << functionName << ";catchStmt;";
  string type;
  if ( stmt->getExceptionDecl() ){
    type = stmt->getExceptionDecl()->getType().getAsString();
  } else {
    type = "noExceptionDecl";
  }
  catchOut << type << endl;
  throwTypes[type] += 1;
  return true;
}

bool ExceptWalker::VisitCXXTryStmt(CXXTryStmt* stmt){
  tryOut << functionName << ";try statement;" << stmt->getNumHandlers() << endl; 
  
  return true;
}
