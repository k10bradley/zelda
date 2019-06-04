/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ParentWalker.h
//
// Created By: Bryan J Muscedere
// Date: 06/07/17.
//
// Contains the majority of the logic for adding
// nodes and relations to the graph. Many of these
// are helper functions like ID generation or
// class resolution.
//
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

#ifndef REX_PARENTWALKER_H
#define REX_PARENTWALKER_H

#include <map>
#include <boost/filesystem.hpp>
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/CommandLine.h"
#include "../Graph/TAGraph.h"

class ZeldaWalker;
class MinimalZeldaWalker;

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

namespace bs = boost::filesystem;

class ParentWalker {
public:
    //Constructor/Destructor
    explicit ParentWalker(ASTContext *Context);
    virtual ~ParentWalker();

    //Graph Operations
    static void addGraphs(std::vector<TAGraph*> graphs);
    static TAGraph* getGraph(int num);
    static void deleteTAGraphs();
    static void deleteTAGraph(int num);
    static int getNumGraphs();
    static int endCurrentGraph();
    static int generateCurrentTAModel(std::string fileName);
    static int generateTAModel(int num, std::string fileName);
    static int generateAllTAModels(std::vector<std::string> fileName);
    static void setCurrentGraphMinMode(bool minMode);
    static bool resolveAllTAModels(std::map<std::string, std::vector<std::string>> databaseMap);
//    static bool dumpCurrentFile(int fileNum, std::string fileName);
//    static bool dumpCurrentSettings(std::vector<bs::path> files, bool minMode);
    static void processExceptions();
    static bool isCFile(std::string str);

    //Processing Operations
    void addLibrariesToIgnore(std::vector<std::string> libraries);
    static std::string CALLBACK_FLAG;
    static std::vector<std::string> headerExt;
    static std::vector<std::string> ext;

protected:
    const std::string COUNT_TRY_FLAG = "countTry";
    const std::string COUNT_CATCH_FLAG = "countCatch";
    const std::string COUNT_THROW_FLAG = "countThrow";
    const std::string CONTEXT_FLAG = "context";
    const std::string TYPE_FLAG = "type";
    const std::string PARAM_FLAG = "isParam";
    
    const std::string FILENAME_ATTR = "filename";

    static TAGraph* graph;
    static std::vector<TAGraph*> graphList;
    ASTContext *Context;


    //System Headers
    bool isInSystemHeader(const Stmt* statement);
    bool isInSystemHeader(const Decl* decl);

    //Zelda Recorders
    void updateNodes(ZeldaNode* dst, ZeldaNode* src, const std::string& FLAG); 

    const NamedDecl* generateZeldaNode(const CXXConstructExpr* expr, ZeldaNode::NodeType type);

    //Name Helper Functions
    std::string generateID(const NamedDecl* decl);
    std::string generateID(const Stmt* stmt);
    std::string generateName(const NamedDecl* decl);
    std::string validateStringArg(std::string name);
    std::string generateFileName(const NamedDecl* decl);
    std::string generateFileName(const Stmt* stmt);
    void recordParentClassLoc(const FunctionDecl* decl);
    static std::string StmtID(const Stmt*);

private:

    //Header Libraries
    const std::string STANDARD_IGNORE = "/usr/local/include/";
    std::vector<std::string> ignoreLibraries;

    //Graph Operations - Helpers
    static int generateTAModel(TAGraph* graph, std::string fileName);

    //System Headers - Helpers
    bool isInSystemHeader(const SourceManager& manager, SourceLocation loc);

    const CXXRecordDecl* getParentClass(const NamedDecl* decl);

    //ZeldaHandlers - Helpers
    bool isClass(const CXXConstructExpr* ctor, std::string className);
    bool isFunction(const CallExpr* expr, std::string functionName);
    const NamedDecl* getParentAssign(const CXXConstructExpr* expr);
    ZeldaNode* findCallbackFunction(std::string callbackQualified);
    std::vector<std::string> getArgs(const CallExpr* expr);
    NamedDecl* getParentVariable(const Expr* callExpr);

    static std::vector<ZeldaEdge*> processFunction(ZeldaNode* func, ZeldaNode* thrown); 
    static ZeldaEdge* processTry(ZeldaNode* tryNode, ZeldaNode* thrown, ZeldaEdge* edge); 
    static ZeldaEdge* processCatch(ZeldaNode* catchNode, ZeldaNode* thrown, ZeldaEdge* edge); 
    static bool matchesType(ZeldaNode* thrown, ZeldaNode* match, bool subType = true);
    static ZeldaEdge* replaceEdge(ZeldaEdge* oldEdge, ZeldaNode* newSrc, ZeldaNode* newDst, ZeldaEdge::EdgeType type);

    static TAGraph* currentGraph;  

};


#endif //REX_PARENTWALKER_H
