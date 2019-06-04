////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZeldaWalker.cpp
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

#include <fstream>
#include <iostream>
#include "clang/AST/Mangle.h"
#include "ZeldaWalker.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <sstream>
#include "../Graph/ZeldaNode.h"

using namespace std;

static bool checkLibrary = true;

/**
 * Constructor
 * @param Context AST Context
 */
ZeldaWalker::ZeldaWalker(ASTContext *Context) : ParentWalker(Context) { }

/**
 * Destructor
 */
ZeldaWalker::~ZeldaWalker(){ }


bool ZeldaWalker::VisitStmt(Stmt *statement) {
    if (isInSystemHeader(statement) && !checkLibrary ) return true;

    return true;
}

bool ZeldaWalker::VisitCallExpr(CallExpr* expr){
    if (isInSystemHeader(expr) && !checkLibrary ) return true;
    recordCallExpr(expr);
    return true;
}

bool ZeldaWalker::VisitCXXConstructExpr(CXXConstructExpr* expr){
  if (isInSystemHeader(expr) && !checkLibrary ) return true;
  recordCXXConstructExpr(expr);

  return true;
}

bool ZeldaWalker::VisitCXXTryStmt(CXXTryStmt* stmt){
  // add to TA graph -> link to function
  if ( isInSystemHeader(stmt) ) return true;
   
  //Record the try declaration.
  recordCXXTryStmt(stmt);

  return true;
}

bool ZeldaWalker::VisitCXXCatchStmt(CXXCatchStmt* stmt){
  // add to TA graph -> link to function
  if (isInSystemHeader(stmt) ) return true;
   
  //Record the try declaration.
  recordCXXCatchStmt(stmt);

  return true;
}

bool ZeldaWalker::VisitCXXThrowExpr(CXXThrowExpr* expr){
  if (isInSystemHeader(expr) ) return true;
  
  if ( isInSystemHeader(expr) ){
    const Expr* subExpr = expr->getSubExpr();
    // if do not consider throws in a system header unless they are a rethrow
    if ( subExpr ){
      return true;
    } 
  }
  
  //Record the throw expression.
  recordCXXThrowExpr(expr);

  return true;
}


bool ZeldaWalker::VisitFunctionDecl(FunctionDecl* decl){
    if (isInSystemHeader(decl) && !checkLibrary ) return true;

    bool system = isInSystemHeader(decl);

    //Record the function declaration.
    recordFunctionDecl(decl, system);

    //Next, check for parent class information.
    recordParentClassLoc(decl);  

    return true;
}

bool ZeldaWalker::VisitCXXRecordDecl(CXXRecordDecl* decl){
    if (isInSystemHeader(decl) && !checkLibrary ) return true;

      recordClassDecl(decl);

    return true;
}


/**
 * Records a basic function declaration to the TA model.
 * @param decl The declaration to add.
 */
void ZeldaWalker::recordFunctionDecl(const FunctionDecl* decl, bool system){
    //Generates the fields.
    if ( ! decl->isThisDeclarationADefinition() ) return;
    string ID = generateID(decl);
    string name = generateName(decl);

    //Creates the node.
    ZeldaNode* node = new ZeldaNode(ID, name, ZeldaNode::FUNCTION);
    node->addSingleAttribute(CALLBACK_FLAG, "0");
    node->addCountAttribute(COUNT_TRY_FLAG);
    node->addCountAttribute(COUNT_THROW_FLAG);
    node->addCountAttribute(COUNT_CATCH_FLAG);
    node->addBoolAttribute("isRecursive", false); 
    if ( system ){
      node->addBoolAttribute("systemHeader", true);
    } else {
      node->addBoolAttribute("systemHeader", false);
    }
    string filename = generateFileName(decl);
    if ( isCFile(filename) )
      node->addSingleAttribute(FILENAME_ATTR, filename);
    
    auto exceptInfo = decl->getExceptionSpecSourceRange(); 

    if ( exceptInfo.isValid() ){
      node->addBoolAttribute("exceptDocumented", true);  
    } else {
      node->addBoolAttribute("exceptDocumented", false);
    } 
    graph->addNode(node);

    //Get the parent.
    addParentRelationship(decl, ID);
}

void ZeldaWalker::recordCXXTryStmt(const CXXTryStmt* stmt){
    //Generates the fields.
    string ID = generateID(stmt);

    //Creates the node.
    ZeldaNode* node = new ZeldaNode(ID, ID, ZeldaNode::TRY);
    node->addCountAttribute(COUNT_TRY_FLAG);
    node->addCountAttribute(COUNT_THROW_FLAG);
    node->addCountAttribute(COUNT_CATCH_FLAG);
    graph->addNode(node);
    
    //Get the parent.
    addParentRelationship(stmt, ID);
}

void ZeldaWalker::updateType(std::string& s){
  if ( s == "NULL TYPE" ) s = "all";
}

void ZeldaWalker::recordCXXCatchStmt(const CXXCatchStmt* stmt){
    string ID = generateID(stmt);

    //Creates the node.
    ZeldaNode* node = new ZeldaNode(ID, ID, ZeldaNode::CATCH);
    node->addCountAttribute(COUNT_TRY_FLAG);
    node->addCountAttribute(COUNT_THROW_FLAG);
    node->addCountAttribute(COUNT_CATCH_FLAG);
    
    string catchType = stmt->getCaughtType().getAsString();
    updateType(catchType);
    node->addSingleAttribute(TYPE_FLAG, catchType);
    //Resolves the filename.
    string filename = generateFileName(stmt);
    if ( isCFile(filename) )
      node->addSingleAttribute(FILENAME_ATTR, filename);
    graph->addNode(node);
    
    //Get the parent.
    addParentRelationship(stmt, ID);

}

void ZeldaWalker::recordCXXThrowExpr(const CXXThrowExpr* expr){
    string ID = generateID(expr);

    string loc = StmtID(expr);
    if ( loc[0] == '-' ) return; 

    //Creates the node.
    const Expr* subExpr = expr->getSubExpr();
    string throwType = "rethrow";
    ZeldaNode::NodeType type = ZeldaNode::RETHROW;
    if ( subExpr ){
      throwType = expr->getSubExpr()->getType().getAsString();
      type = ZeldaNode::THROW;
    }
    updateType(throwType);

    ZeldaNode* node = new ZeldaNode(ID, ID, type);
    node->addSingleAttribute(TYPE_FLAG, throwType);
    node->addSingleAttribute("function", "");
    //Resolves the filename.
    string filename = generateFileName(expr);
    if ( isCFile(filename) )
      node->addSingleAttribute(FILENAME_ATTR, filename);
    graph->addNode(node);
    
    //Get the parent.
    addParentRelationship(expr, ID);
}

void ZeldaWalker::recordClassDecl(const CXXRecordDecl *decl) {
    //Generates some fields.
    string ID = generateID(decl);
    string name = generateName(decl);
    ZeldaNode *node = nullptr;
    //Creates the node.
    if (!graph->doesNodeExist(ID)) {
        ZeldaNode *node = new ZeldaNode(ID, name, ZeldaNode::CLASS);
        graph->addNode(node);
        //Resolves the filename.
        string filename = generateFileName(decl);
        node->addMultiAttribute(FILENAME_ATTR, filename);
    }
    
    addBaseClasses(decl);
}

void ZeldaWalker::addBaseClasses(const CXXRecordDecl* decl){
    string ID = generateID(decl);
    string name = generateName(decl);
    
    if ( !decl->hasDefinition() ) return;

    ZeldaNode *node = graph->findNode(ID);

    if ( node == nullptr ) return;

    for ( auto& base: decl->bases() ){
      string baseName = base.getType().getAsString();
  
      // clean class name for future matching
      if ( baseName.substr(0, 7) == "struct ") baseName = baseName.substr(7);
      if ( baseName.substr(0, 6) == "class ") baseName = baseName.substr(6);
      
      ZeldaNode* baseNode = graph->findNode(baseName);
      if ( baseNode == nullptr ) continue;
      string baseID = baseNode->getID();  
      
      if (graph->doesEdgeExist(baseID, ID, ZeldaEdge::INHERITS)){

        continue;
      }
      ZeldaEdge* edge = (!baseNode) ? new ZeldaEdge(baseID, node, ZeldaEdge::INHERITS) :
                      new ZeldaEdge(baseNode, node, ZeldaEdge::INHERITS);
      graph->addEdge(edge);
    }

}


bool ZeldaWalker::VisitCXXMemberCallExpr(CXXMemberCallExpr* expr){
    if (isInSystemHeader(expr) && !checkLibrary ) return true;
    recordCXXMemberCallExpr(expr);
    return true;

}

void ZeldaWalker::recordCXXMemberCallExpr(const CXXMemberCallExpr* expr){
    // gets the method which is called
    auto mDecl = expr->getMethodDecl(); 
    if ( mDecl != nullptr ){

      // gets object method is called on
      auto member = expr->getImplicitObjectArgument();
      if ( member != nullptr ){
        
        // determines if type of this is known
        auto devirtual = mDecl->getDevirtualizedMethod(member, false);
        if ( devirtual ){
          if (isInSystemHeader(devirtual)) return;
          recordFunctionCall(expr, devirtual);
          recordFunctionCall(expr, devirtual, true);
          return;
        }
      }    
  }
}

void ZeldaWalker::recordCallExpr(const CallExpr* expr){
    //Get the sub-function.
    auto callee = expr->getDirectCallee();
    if (callee == nullptr) return;

    recordFunctionCall(expr, callee);
}

void ZeldaWalker::recordCXXConstructExpr(const CXXConstructExpr* expr){
    auto func = expr->getConstructor();
    //cout << func << endl;
    if (func == nullptr) return;
    
    string calleeID = generateID(func);
    ZeldaNode* calleeNode = graph->findNode(calleeID);

    //Gets the parent expression.
    auto parDecl = getParentFunction(expr);
    if (parDecl == nullptr) return;

    //Gets the ID for the parent.
    string callerID = generateID(parDecl);
    ZeldaNode* callerNode = graph->findNode(callerID);

    if (graph->doesEdgeExist(callerID, calleeID, ZeldaEdge::CALLS)) return;
   
    //cout << "creating edge" << endl;
    ZeldaEdge* edge = nullptr;
    if (calleeNode == nullptr) {
      if ( callerNode == nullptr ){
        edge = new ZeldaEdge(callerID, calleeID, ZeldaEdge::CALLS);
      } else {
        edge = new ZeldaEdge(callerNode, calleeID, ZeldaEdge::CALLS);
      }
    } else {
      if ( callerNode == nullptr ){
        edge = new ZeldaEdge(callerID, calleeNode, ZeldaEdge::CALLS);
      } else {
        edge = new ZeldaEdge(callerNode, calleeNode, ZeldaEdge::CALLS);
      }
    }
    //cerr << "edge created" << endl;
    if ( edge ){ 
      graph->addEdge(edge);
      //cerr << "parent relationship done" << endl;
    }
    addParentRelationship(expr, func, calleeID);
}

void ZeldaWalker::recordFunctionCall(const CallExpr* expr, const FunctionDecl* func, bool isVirtual){
    //Gets the ID for the cDecl.

    string calleeID = generateID(func);
    ZeldaNode* calleeNode = graph->findNode(calleeID);
    
    ZeldaEdge::EdgeType call;
    if ( isVirtual ) call = ZeldaEdge::VIRTUAL_CALL;
    else call = ZeldaEdge::CALLS;

    //Gets the parent expression.
    auto parDecl = getParentFunction(expr);
    if (parDecl == nullptr) return;

    //Gets the ID for the parent.
    string callerID = generateID(parDecl);
    ZeldaNode* callerNode = graph->findNode(callerID);
   

    //Adds the edge.
    if (graph->doesEdgeExist(callerID, calleeID, call) ) return;
    
    ZeldaEdge* edge = nullptr;
    if (calleeNode == nullptr) {
      if ( callerNode == nullptr ){
        edge = new ZeldaEdge(callerID, calleeID, call);
      } else {
        edge = new ZeldaEdge(callerNode, calleeID, call);
      }
    } else {
      if ( callerNode == nullptr ){
        edge = new ZeldaEdge(callerID, calleeNode, call);
      } else {
        edge = new ZeldaEdge(callerNode, calleeNode, call);
      }
    }
    if ( edge ) {
      graph->addEdge(edge);
    }
    addParentRelationship(expr, func, calleeID);
}


void ZeldaWalker::recordDeclRefExpr(const DeclRefExpr* expr){
    //Get the sub-function.
    const NamedDecl* callee = expr->getFoundDecl();
    if (callee == nullptr) return;
    const FunctionDecl* cDecl = dynamic_cast<const FunctionDecl*>(callee);
    if (cDecl == nullptr) return;

    //Gets the ID for the cDecl.
    string calleeID = generateID(cDecl);
    ZeldaNode* calleeNode = graph->findNode(calleeID);

    //Gets the parent expression.
    auto parDecl = getParentFunction(expr);
    if (parDecl == nullptr) return;

    //Gets the ID for the parent.
    string callerID = generateID(parDecl);

    ZeldaNode* callerNode = graph->findNode(callerID);

    //Adds the edge.
    if (graph->doesEdgeExist(callerID, calleeID, ZeldaEdge::CALLS)) return;
    ZeldaEdge* edge = (calleeNode == nullptr) ?
                    new ZeldaEdge(callerNode, calleeID, ZeldaEdge::CALLS) :
                    new ZeldaEdge(callerNode, calleeNode, ZeldaEdge::CALLS);
    graph->addEdge(edge);

}



/**
 * Finds an expression based on stored expressions.
 * @param expression The expression to find.
 * @return Whether the expression was found.
 */
bool ZeldaWalker::findExpression(const Expr* expression){
    for (Expr* cur : parentExpression){
        if (cur == expression) return true;
    }

    return false;
}



/**
 * Records the parent function in a statement.
 * @param statement The statement.
 * @param baseItem The baseItem of the parent.
 * @return The generated Zelda edge.
 */
ZeldaEdge* ZeldaWalker::recordParentFunction(const Stmt* statement, ZeldaNode* baseItem){
    //Gets the parent function.
    const FunctionDecl* decl = getParentFunction(statement);
    ZeldaNode* funcNode = graph->findNode(generateID(decl));

    //Checks if an edge already exists.
    if (graph->doesEdgeExist(generateID(decl), baseItem->getID(), ZeldaEdge::CALLS))
        return graph->findEdge(generateID(decl), baseItem->getID(), ZeldaEdge::CALLS);

    //Adds a call relation between the two.
    ZeldaEdge* edge;
    if (funcNode) {
        edge = new ZeldaEdge(funcNode, baseItem, ZeldaEdge::CALLS);
    } else {
        edge = new ZeldaEdge(generateID(decl), baseItem, ZeldaEdge::CALLS);
    }
    graph->addEdge(edge);
    return edge;
}


/**
 * Adds the parent relationship.
 * @param baseDecl The base declaration.
 * @param baseID The base ID.
 */
void ZeldaWalker::addParentRelationship(const Stmt* stmt, std::string baseID ){
    bool getParent = true;
    const NamedDecl* currentDecl = nullptr;
    const Stmt* currentStmt = nullptr;
    ZeldaNode* dst = graph->findNode(baseID);

    ZeldaEdge::EdgeType type = ZeldaEdge::CONTEXT;
    if ( isa<CXXThrowExpr>(stmt) ){
      if ( dst->getType() == ZeldaNode::RETHROW ) type = ZeldaEdge::RETHROWS;
      else type = ZeldaEdge::THROWS;
    }
    bool contextAdded = false;
    //Get the parent.
    auto parent = Context->getParents(*stmt);

    while(getParent){
        //Check if it's empty.
        if (parent.empty()){
          return;
        }

        //Get the current decl as named.
        currentDecl = parent[0].get<clang::NamedDecl>();
        currentStmt = parent[0].get<clang::Stmt>();
        if ( stmt == currentStmt ) {}
        else if (currentDecl) {
            //Get the parent as a NamedDecl.
            ZeldaNode* src = handleParent(currentDecl);
            if ( src ){
              updateNode( src, dst, stmt, type );
              return;
            }
        }
        else if ( currentStmt ){
            //Get the parent as a NamedDecl.
            ZeldaNode* src = handleParent(currentStmt);
            if ( src ) {
              updateNode(src, dst, stmt, type);
              return;
            }
        }

        parent = Context->getParents(parent[0]);
    }
}

void ZeldaWalker::addParentRelationship(const CXXConstructExpr* expr, const CXXConstructorDecl* decl, std::string funcID ){
    bool getParent = true;
    const NamedDecl* currentDecl = nullptr;
    const Stmt* currentStmt = nullptr;
    ZeldaNode* dest = graph->findNode(funcID);
    ZeldaEdge::EdgeType type = ZeldaEdge::CONTEXT;
    //Get the parent.
    //cerr << "finding construct expr parent" << endl;
    auto parent = Context->getParents(*expr);
    while(getParent){
        //Check if it's empty.
        if (parent.empty() ){
          return;
        }
        //Get the current decl as named.
        currentDecl = parent[0].get<clang::NamedDecl>();
        currentStmt = parent[0].get<clang::Stmt>();
        if ( expr == currentStmt ) {}
        else if (currentDecl) {

            ZeldaNode* parentN = handleParent(currentDecl);
            if ( parentN ) {
              //cout << "  called from " << parentN->getID() << endl;
              if ( dest ){
                updateNode(parentN, dest, nullptr, type); 

              } else {
                if ( graph->doesEdgeExist( parentN->getID(), funcID, type ) ) return;
                graph->addEdge( new ZeldaEdge( parentN, funcID, type ) );
              }
              return;
            }

        }
        else if ( currentStmt ){
            

            ZeldaNode* src = handleParent(currentStmt);
            if ( src ) {
              //cout << "  called from " << src->getName() << endl;
              if ( dest ){
                updateNode(src, dest, nullptr, type); 
              } else {
                if ( graph->doesEdgeExist( src->getID(), funcID, type ) ) return;
                graph->addEdge( new ZeldaEdge( src, funcID, type ) );
              }
              return;
            }
        }

        parent = Context->getParents(parent[0]);
    }


}

void ZeldaWalker::addParentRelationship(const CallExpr* call, const FunctionDecl* func, string funcID ){
    bool getParent = true;
    const NamedDecl* currentDecl = nullptr;
    const Stmt* currentStmt = nullptr;
    ZeldaNode* dest = graph->findNode(funcID);
    ZeldaEdge::EdgeType type = ZeldaEdge::CONTEXT;
    //Get the parent.

    auto parent = Context->getParents(*call);
    while(getParent){
        //Check if it's empty.
        if (parent.empty() ){
          return;
        }
        //Get the current decl as named.
        currentDecl = parent[0].get<clang::NamedDecl>();
        currentStmt = parent[0].get<clang::Stmt>();
        if ( call == currentStmt ) {}
        else if (currentDecl) {

            ZeldaNode* parentN = handleParent(currentDecl);
            if ( parentN ) {
              //cout << "  called from " << parentN->getID() << endl;
              if ( dest ){
                updateNode(parentN, dest, nullptr, type); 

              } else {
                if ( graph->doesEdgeExist( parentN->getID(), funcID, type ) ) return;
                graph->addEdge( new ZeldaEdge( parentN, funcID, type ) );
              }
              return;
            }

        }
        else if ( currentStmt ){
            

            ZeldaNode* src = handleParent(currentStmt);
            if ( src ) {
              //cout << "  called from " << src->getName() << endl;
              if ( dest ){
                updateNode(src, dest, nullptr, type); 
              } else {
                if ( graph->doesEdgeExist( src->getID(), funcID, type ) ) return;
                graph->addEdge( new ZeldaEdge( src, funcID, type ) );
              }
              return;
            }
        }

        parent = Context->getParents(parent[0]);
    }

}

void ZeldaWalker::addParentRelationship(const NamedDecl* decl, string baseID){

    bool getParent = true;
    const NamedDecl* currentDecl = decl;
    const Stmt* currentStmt = nullptr;
    ZeldaNode* dest = graph->findNode(baseID);
    //Get the parent.

    auto parent = Context->getParents(*decl);
    while(getParent){
        //Check if it's empty.
        if (parent.empty()){
          return;
        }
        //Get the current decl as named.
        currentDecl = parent[0].get<clang::NamedDecl>();
        currentStmt = parent[0].get<clang::Stmt>();
        if ( decl == currentDecl ) {}
        else if (currentDecl) {

            ZeldaNode* parentN = handleParent(currentDecl);
            if ( parentN ) {
              updateNode(parentN, dest, nullptr, ZeldaEdge::CONTAINS); 
              return;
            }

        }
        else if ( currentStmt ){
            
            ZeldaNode* src = handleParent(currentStmt);
            if ( src ) {
              updateNode(src, dest, nullptr, ZeldaEdge::CONTAINS);
              return;
            }
        }

        parent = Context->getParents(parent[0]);
    }
}



ZeldaNode* ZeldaWalker::handleParent(const NamedDecl* parent){
    string parentID;
    ZeldaNode* pNode = nullptr;
    if (isa<FunctionDecl>(parent) || isa<CXXMethodDecl>(parent) ){
        parentID = generateID(parent);
        pNode = graph->findNode(parentID); 

    }
    return pNode;
}

ZeldaNode* ZeldaWalker::handleParent(const Stmt* parent){
    //Now, check to see what our current ID is.
    string parentID;
    ZeldaNode* pNode = nullptr;
    if ( parent == nullptr ) return nullptr;
    if (isa<CXXTryStmt>(parent) || isa<CXXCatchStmt>(parent) ) {

      parentID = generateID(parent);      

      if ( parentID.empty() ) return nullptr;

      pNode = graph->findNode(parentID); 
    }
    return pNode;
}

void ZeldaWalker::updateNode(ZeldaNode* src, ZeldaNode* dst, const Stmt* stmt, ZeldaEdge::EdgeType type){
  //if ( graph->doesEdgeExist( src->getID(), dst->getID(), type ) ) return;
  if ( stmt ){
    if ( isa<CXXTryStmt>(stmt) ){
      updateNodes(src, dst, COUNT_TRY_FLAG);
    }
    if ( isa<CXXThrowExpr>(stmt) ){
      updateNodes(src, dst, COUNT_THROW_FLAG);
    }
    if ( isa<CXXCatchStmt>(stmt) ){
      updateNodes(src, dst, COUNT_CATCH_FLAG);
    }
  }

  if ( type == ZeldaEdge::CONTEXT && dst && dst->getType() != ZeldaNode::FUNCTION ) dst->addSingleAttribute(CONTEXT_FLAG, src->getName());

  graph->addEdge(new ZeldaEdge(src, dst, type));
}


const FunctionDecl* ZeldaWalker::getParentFunction(const Stmt* baseFunc){
    bool getParent = true;

    //Get the parent.
    auto parent = Context->getParents(*baseFunc);
    while(getParent){
        //Check if it's empty.
        if (parent.empty()){
            getParent = false;
            continue;
        }

        //Get the current decl as named.
        auto currentDecl = parent[0].get<clang::NamedDecl>();
        if (currentDecl && isa<clang::FunctionDecl>(currentDecl)){
            return dyn_cast<FunctionDecl>(currentDecl);
        }

        parent = Context->getParents(parent[0]);
    }

    return nullptr;
}

