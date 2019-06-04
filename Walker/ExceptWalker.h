////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExceptWalker.h
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

#ifndef EXCEPT_WALKER_H
#define EXCEPT_WALKER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"
#include "../Graph/TAGraph.h"
#include "ParentWalker.h"
#include <map>
#include "Classifier.h"

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

class ExceptWalker : public RecursiveASTVisitor<ExceptWalker>, public ParentWalker {
public:
    //Constructor/Destructor
    explicit ExceptWalker(ASTContext *Context);
    ~ExceptWalker();


    //ASTWalker Functions
    bool VisitFunctionDecl(FunctionDecl*);
    bool VisitCXXMethodDecl(CXXMethodDecl*);
    bool VisitCXXThrowExpr(CXXThrowExpr*);
    bool VisitCXXCatchStmt(CXXCatchStmt*);
    bool VisitCXXTryStmt(CXXTryStmt*);
private:
    FunctionDecl* currFunction = nullptr;

    std::string functionName;

    std::string getCurrFunctionName();

    std::string QualTypeString(const QualType&);

    std::map<std::string, int> catchTypes;
    std::map<std::string, int> throwTypes;

    std::ofstream catchOut; 
    std::ofstream throwOut;
    std::ofstream tryOut;

};

#endif
