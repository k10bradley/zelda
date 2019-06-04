////////////////////////////////////////////////////////////////////////////////////////////////////////
// Classifier.cpp
//
// Created By: Ten Bradley
// Date: 2019.
//
// Walks Clang's AST to do simple classification of expressions
// within catch and try statements. 
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
#include "Classifier.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../Graph/ZeldaNode.h"
#include "clang/AST/DeclCXX.h"
#include <fstream>
#include <sstream>

using namespace std;

int Classifier::functionCode = 0;
int Classifier::exceptionCode = 0;
int Classifier::catchCode = 0;

ofstream Classifier::output;

int Classifier::getFunctionCode(){ return functionCode; }
int Classifier::getExceptionCode(){ return exceptionCode; }
int Classifier::getCatchCode(){ return catchCode; }

/**
 * Constructor
 * @param Context AST Context
 */
Classifier::Classifier(ASTContext *Context) : ParentWalker(Context) {}

/**
 * Destructor
 */
Classifier::~Classifier(){ }

void Classifier::setClassifyFile(string& loc){
  if ( ! output.is_open() ){
    string file = loc;
    file += "/classification.txt";
    output.open(file);
  }
}

bool Classifier::inCatch(){
  return !catches.empty();
}

bool Classifier::inTry(){
  return !tries.empty();
}

ostream* Classifier::getStream(){
  if ( inCatch() ) return &(catches.back());
  if ( inTry() ) return &(tries.back());
  return nullptr;
}

void Classifier::updateType(std::string& s){
  if ( s == "NULL TYPE" ) s = "all";
  if ( s.find("operator<<") != string::npos ) s = "print";
}


bool Classifier::VisitFunctionDecl(FunctionDecl* func){
  if ( ! isInSystemHeader(func) ) {
    currFunction = func;
    if ( func->isThisDeclarationADefinition() ){
      string temp;
      string code;
      raw_string_ostream oss(temp);
      auto body = func->getBody();
      if ( body ) body->printPretty(oss, nullptr, PrintingPolicy(Context->getLangOpts()));
      code = oss.str();
      functionCode += countLines(code);
      return true;
    }
  }
  currFunction = nullptr;
  return true;
}

bool Classifier::TraverseCXXCatchStmt(CXXCatchStmt* stmt){

  if ( currFunction ){
    string catchType = stmt->getCaughtType().getAsString();
    updateType(catchType);
    catches.emplace_back();
    catchTypes.emplace_back(catchType);

    if ( currFunction ) functionInfo(currFunction);
    catches.back().clear();
    bool hasChildren = false;

    for( auto child: stmt->children() ){
      hasChildren = true;
      TraverseStmt(child);
      statements.clear();
    }

    string results = catches.back().str();
    if ( !hasChildren ) results =  "empty empty;";
    else if ( results.empty() ) results =  "unclassified unclassified;";
    output << "catch " << catchType << ";" << results << endl;
    catches.pop_back();
    catchTypes.pop_back();
  
    if ( catches.empty() ){
      string temp;
      string code;
      raw_string_ostream oss(temp);
      stmt->printPretty(oss, nullptr, PrintingPolicy(Context->getLangOpts()));
      code = oss.str();
      catchCode += countLines(code);
      exceptionCode += 1; // to account for prettyPrint placing } and catch on the same line 
    }
  }
  return true;
}

bool Classifier::TraverseCXXTryStmt(CXXTryStmt* stmt){
  if ( currFunction ){
    tries.emplace_back();
    bool hasChildren = false;
    for( auto child: stmt->children() ){
      hasChildren = true;
      TraverseStmt(child);
      statements.clear();
    }
    string results = tries.back().str();

    if ( !hasChildren ) results =  "empty empty;";
    else if ( results.empty() ) results =  "unclassified unclassified;";

    output << "try;" << results << endl;
    tries.pop_back();

    if ( tries.empty() ){
      string temp;
      string code;
      raw_string_ostream oss(temp);
      stmt->printPretty(oss, nullptr, PrintingPolicy(Context->getLangOpts()));
      code = oss.str();
      exceptionCode += countLines(code); 
    }

  }
  return true;

}

bool Classifier::VisitCXXThrowExpr(CXXThrowExpr* expr){
  ostream* out = getStream();
  if ( out ){
    string throwType;
    Expr* sub = expr->getSubExpr();
    if ( sub ){
      throwType = sub->getType().getAsString();
      updateType(throwType);
    } else {
      *out << "rethrow " << catchTypes.back() << ";";
      return true;
    }
    *out << "throw " << throwType << ";"; 
  }
  return true;
}

bool Classifier::VisitReturnStmt(ReturnStmt*){
  ostream* out = getStream();
  if ( out ){
    *out << "return;";
  } 
  return true;
}

bool Classifier::VisitContinueStmt(ContinueStmt*){
  ostream* out = getStream();
  if ( out ){
    *out << "continue;";
  } 
  return true;
}

bool Classifier::VisitBreakStmt(BreakStmt*){
  ostream* out = getStream();
  if ( out ){
    *out << "break;";
  } 
  return true;
}

bool Classifier::VisitCXXDeleteExpr(CXXDeleteExpr* expr){
  ostream* out = getStream();
  if ( out ){
    *out << "delete" << ( expr->isArrayForm() ? "[] " : " " ) << expr->getDestroyedType().getAsString() << ";";
  } 
  return true;
}

void Classifier::functionInfo(FunctionDecl* func){
  string function;
  ostream* out = getStream();
  // if member, print class -> this is bad style
  CXXMethodDecl* member = dynamic_cast<CXXMethodDecl*>(func);
  if ( member ){
    // get Class type and then function info
    auto parent = member->getParent();
    if ( parent ) {
      auto decl = parent->getCanonicalDecl();
      if ( decl ){
        auto name = decl->getDeclName();
        *out << name.getAsString() << "::" ;
      }
    }

  }
  // function name
  function +=  func->getNameInfo().getAsString() + "(";
  // function params
  if ( func->getNumParams() != 0 ) {
    bool first = true;
    for ( auto param: func->parameters() ){
      if ( first ) first = false;
      else function += ", ";
      function += param->getOriginalType().getAsString();
    } 
  }

  function += ");";
  *out << function;

}

bool Classifier::VisitCXXMemberCallExpr(CXXMemberCallExpr* member){
  ostream* out = getStream();
  if ( out ){
    string called;
    CXXMethodDecl* func = member->getMethodDecl();
    *out << "call "; 
    if ( func ){
      functionInfo(func);
    } else {
      *out << "unknown;";
    }
  }
  return true;
}

bool Classifier::VisitCallExpr(CallExpr* call){
  ostream* out = getStream();
  if ( out ){
    string called;
    FunctionDecl* func = call->getDirectCallee();
    if ( func ){
      string name = func->getNameInfo().getAsString();
      updateType(name);
      if ( name == "print" ){
        *out << "print print;";
        return true;
      }
      *out << "call ";
      functionInfo(func);
    } else {
      *out << "call unknown;";
    }
  }
  return true;
}

int Classifier::countLines(const std::string& s){
  int count = 0;
  istringstream iss{s};
  string line;
  while ( getline(iss, line)) {
    ++count;
  }
  return count;
}
