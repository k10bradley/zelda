////////////////////////////////////////////////////////////////////////////////////////////////////////
// Counter.h
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

#ifndef COUNTER_WALKER_H
#define COUNTER_WALKER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"
#include "../Graph/TAGraph.h"
#include "ParentWalker.h"
#include <vector>
#include <map>
#include <iostream>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;


class CountTypes{
  std::map<std::string, int> occurs;
 public:
  int& get(const std::string&);
  CountTypes();
  const std::map<std::string,int> getMap() const;

  friend std::ostream& operator<<(std::ostream&, const CountTypes&);
};

std::ostream& operator<<(std::ostream&, const CountTypes&);



class Counter: public RecursiveASTVisitor<Counter>, public ParentWalker{
public:
    //Constructor/Destructor
    explicit Counter(ASTContext *Context);
 
    bool TraverseFunctionDecl(FunctionDecl*);
    bool TraverseCXXTryStmt(CXXTryStmt*);
    bool TraverseCXXCatchStmt(CXXCatchStmt* stmt);

    //ASTWalker Functions
    bool VisitCXXTryStmt(CXXTryStmt*);
    bool VisitCXXCatchStmt(CXXCatchStmt*);
    bool VisitCXXThrowExpr(CXXThrowExpr*);

    bool VisitSwitchCase(SwitchCase*);
    bool VisitIfStmt(IfStmt*);
    bool VisitWhileStmt(WhileStmt*);
    bool VisitDoStmt(DoStmt*);
    bool VisitForStmt(ForStmt*);
    bool VisitReturnStmt(ReturnStmt*);
    bool VisitContinueStmt(ContinueStmt*);
    bool VisitBreakStmt(BreakStmt*);
    bool VisitCXXDeleteExpr(CXXDeleteExpr*);

    static void printData(int, std::ostream& out = std::cout );

private:

    bool addCount(const std::string& s);

    static CountTypes tries;
    static CountTypes catches;
    static CountTypes nonExceptional;

    std::vector<CountTypes*> current;
};

#endif //COUNTER_WALKER_H
