////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZeldaWalker.h
//
// Created By: Kirsten Bradley 2019
//
// Walks the Clang AST to recover information for exception
// flow analysis.
//
//
// Code is greatly motified from Rex:
// Rex was Created By: Bryan J Muscedere
// Date: 07/04/17.
// Copyright (C) 2017, Bryan J. Muscedere
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

#ifndef REX_ZeldaWALKER_H
#define REX_ZeldaWALKER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"
#include "../Graph/TAGraph.h"
#include "ParentWalker.h"

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

class ZeldaWalker : public RecursiveASTVisitor<ZeldaWalker>, public ParentWalker {
public:
    //Constructor/Destructor
    explicit ZeldaWalker(ASTContext *Context);
    ~ZeldaWalker();

    //ASTWalker Functions
    bool VisitStmt(Stmt* statement);
    bool VisitCallExpr(CallExpr*);
    bool VisitCXXConstructExpr(CXXConstructExpr*);
    bool VisitFunctionDecl(FunctionDecl* decl);
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr* expr);
    bool VisitCXXRecordDecl(CXXRecordDecl* decl);
    bool VisitCXXTryStmt(CXXTryStmt* stmt);
    bool VisitCXXCatchStmt(CXXCatchStmt* stmt);
    bool VisitCXXThrowExpr(CXXThrowExpr* expr);

private:

    std::vector<clang::Expr*> parentExpression;

    //C++ Detectors
    void recordFunctionDecl(const FunctionDecl* decl, bool);
    void recordClassDecl(const CXXRecordDecl* decl);
    void recordCXXTryStmt(const CXXTryStmt* stmt);
    void recordCXXCatchStmt(const CXXCatchStmt* stmt);
    void recordCXXThrowExpr(const CXXThrowExpr* expr);
    void recordCXXConstructExpr(const CXXConstructExpr* expr);

    void addBaseClasses(const CXXRecordDecl* decl);

    bool findExpression(const Expr* expression);

    //Expr Recorders
    void recordCXXMemberCallExpr(const CXXMemberCallExpr* decl);
    void recordFunctionCall(const CallExpr* exor, const FunctionDecl* func, bool isVirtual = false);
    void recordCallExpr(const CallExpr* expr);
    void recordDeclRefExpr(const DeclRefExpr* expr);
    ZeldaEdge* recordParentFunction(const Stmt* statement, ZeldaNode* baseItem);


    //Secondary Helper Functions
    void addParentRelationship(const Stmt* stmt, std::string baseID);
    void addParentRelationship(const NamedDecl* decl, std::string baseID);
    void addParentRelationship(const CallExpr* call, const FunctionDecl* func, std::string funcID );
    void addParentRelationship(const CXXConstructExpr* expr, const CXXConstructorDecl* decl, std::string funcID );
    const FunctionDecl* getParentFunction(const Stmt* expr);
    void updateType(std::string&);
    ZeldaNode* handleParent(const NamedDecl* parent);
    ZeldaNode* handleParent(const Stmt* parent);
    void updateNode(ZeldaNode* src, ZeldaNode* dst, const Stmt* stmt, ZeldaEdge::EdgeType type);
    std::string walkerID(const Stmt* stmt);
};

#endif //REX_ZeldaWALKER_H
