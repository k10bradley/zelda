////////////////////////////////////////////////////////////////////////////////////////////////////////
// Classifier.h
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

#ifndef CLASSIFIER_WALKER_H
#define CLASSIFIER_WALKER_H

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
#include <sstream>
#include <fstream>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

class ClassifyWorker: public RecursiveASTVisitor<ClassifyWorker>, public ParentWalker {
  public:

    ClassifyWorker(ASTContext *Context);
};


class Classifier : public RecursiveASTVisitor<Classifier>, public ParentWalker {
public:
    //Constructor/Destructor
    explicit Classifier(ASTContext *Context);
    ~Classifier();
 

    //ASTWalker Functions
    bool TraverseCXXCatchStmt(CXXCatchStmt*);
    bool TraverseCXXTryStmt(CXXTryStmt* stmt);
    bool VisitCXXThrowExpr(CXXThrowExpr*);
    bool VisitCallExpr(CallExpr*);
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr*);
    bool VisitCXXDeleteExpr(CXXDeleteExpr*);
    bool VisitFunctionDecl(FunctionDecl*);
    bool VisitReturnStmt(ReturnStmt*);
    bool VisitContinueStmt(ContinueStmt*);
    bool VisitBreakStmt(BreakStmt*);

    static void setClassifyFile(std::string&);

    static int getFunctionCode();
    static int getExceptionCode();
    static int getCatchCode();
private:
    static int functionCode;
    static int exceptionCode;
    static int catchCode;
    static std::ofstream output;

    FunctionDecl* currFunction = nullptr;

    enum class Statments { PRINT, DELETE, THROW, RETHROW, OTHER };
    std::vector<std::ostringstream> catches; 
    std::vector<std::ostringstream> tries; 
    std::vector<std::string> catchTypes;
    std::vector<Statments> statements;
    bool inCatch();
    bool inTry();
    std::ostream* getStream();
    void updateType(std::string& s);
    void functionInfo(FunctionDecl*);

    int countLines(const std::string& s);

    void recordFunctionDecl(const FunctionDecl* );
};

#endif //CLASSIFIER_WALKER_H
