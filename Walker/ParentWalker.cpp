/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ParentWalker.cpp
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

#include <fstream>
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>
#include "ParentWalker.h"

using namespace std;

TAGraph* ParentWalker::graph = new TAGraph();
TAGraph* ParentWalker::currentGraph = nullptr;
vector<TAGraph*> ParentWalker::graphList = vector<TAGraph*>();
vector<string> ParentWalker::headerExt = {"h","H","HPP","hpp","HXX","hxx","hh","HH","h++", "H++"};
vector<string> ParentWalker::ext = {"C","c","CPP","cpp","CXX","cxx","cc","CC","c++", "C++"};

std::string ParentWalker::CALLBACK_FLAG = "isCallbackFunc";

std::string catchFlag = "caughtBy";


static string replaceMap(string str){
  string s = str;
    for ( char& c: s ){
      switch (c){
        case '(':
        case ')': c= 'P'; continue;
        case '{':
        case '}': c= 'B'; continue;
        case '=': c= 'E'; continue;
        case ' ': c = '-'; continue;
        case '"': c = 'Q'; continue;
      }
    }
  return s;
}

bool ParentWalker::isCFile(string str){
  for ( auto header: ParentWalker::headerExt ){
    if ( boost::algorithm::ends_with(str, "." +  header) ) return true;
  }
  for ( auto file: ParentWalker::ext ){
    if ( boost::algorithm::ends_with(str, "." + file) ) return true;
  }
  return false;
}

/**
 * Constructor for the parent walker class.
 * @param Context The AST context.
 */
ParentWalker::ParentWalker(ASTContext *Context) : Context(Context) {
    ignoreLibraries.push_back(STANDARD_IGNORE);
}

/**
 * Destructor.
 */
ParentWalker::~ParentWalker() {}

/**
 * Adds graphs to the graph list.
 * @param graphs The graphs to add.
 */
void ParentWalker::addGraphs(vector<TAGraph *> graphs) {
    graphList.insert(graphList.end(), graphs.begin(), graphs.end());
}

/**
 * Gets the graph at the specified number.
 * @param num The specified number to get.
 * @return The graph at that location.
 */
TAGraph* ParentWalker::getGraph(int num) {
    return graphList.at(num);
}

/**
 * Deletes all TA graphs being maintained.
 */
void ParentWalker::deleteTAGraphs(){
    delete graph;
    for (int i = 0; i < graphList.size(); i++)
        delete graphList.at(i);

    graphList = vector<TAGraph*>();
}

/**
 * Deletes a TA graph by graph number.
 * @param num The number to delete.
 */
void ParentWalker::deleteTAGraph(int num){
    if (num < 0 || num >= graphList.size()) return;
    delete graphList.at(num);
    graphList.erase(graphList.begin() + num);
}

/**
 * Gets the number of graphs being maintained.
 * @return The number of graphs.
 */
int ParentWalker::getNumGraphs(){
    return (int) graphList.size();
}

/**
 * Stops processing the current graph.
 * @return The graph number of this graph.
 */
int ParentWalker::endCurrentGraph(){
    //Moves the current graph.
    graphList.push_back(graph);
    graph = new TAGraph();

    return (int) graphList.size() - 1;
}

/**
 * Outputs the current model.
 * @param fileName The filename to output.
 * @return Integer of graph number.
 */
int ParentWalker::generateCurrentTAModel(string fileName){
    return ParentWalker::generateTAModel(graph, fileName);
}

/**
 * Generates a TA model by number.
 * @param num The number to generate.
 * @param fileName The file to output it as.
 * @return Integer of graph number.
 */
int ParentWalker::generateTAModel(int num, string fileName){
    //Get the graph at the number.
    if (num >= graphList.size() || num < 0) return 0;
    return ParentWalker::generateTAModel(graphList.at(num), fileName);
}

/**
 * Outputs all graph models.
 * @param fileNames The file names to output as.
 * @return The return code.
 */
int ParentWalker::generateAllTAModels(vector<string> fileNames){
    if (fileNames.size() != graphList.size()) return 0;
    for (int i = 0; i < fileNames.size(); i++){
        int code = ParentWalker::generateTAModel(graphList.at(i), fileNames.at(i));
        if (code != 1) return 0;
    }
    return 1;
}


/**
 * Resolves all TA files based on a compile commands database map.
 * @param databaseMap The database map.
 * @return Whether the operation was successful.
 */
bool ParentWalker::resolveAllTAModels(map<string, vector<string>> databaseMap){
    //Goes through the graphs and forces them to resolve.
    for (TAGraph* curGraph : graphList){
        bool status  = curGraph->resolveComponents(databaseMap);
        if (!status) return false;
    }
    TAGraph* newGraph = new TAGraph;
    for (TAGraph* curGraph : graphList){
      newGraph->merge(curGraph);
    }
    graphList.clear();
    graphList.emplace_back(newGraph);

    cout << "Processing exceptions..." << endl;
    processExceptions();
    cout << "Done exceptions..." << endl;
    return true;
}



/*
 * Sets particular libraries to ignore when processing.
 * @param libraries The libraries to ignore.
 */
void ParentWalker::addLibrariesToIgnore(vector<string> libraries){
    ignoreLibraries = libraries;
}


/**
 * Gets the parent class of a decl.
 * @param decl The decl to find the parent class.
 * @return The class.
 */
const CXXRecordDecl* ParentWalker::getParentClass(const NamedDecl* decl){
    bool getParent = true;

    //Get the parent.
    auto parent = Context->getParents(*decl);
    while(getParent){
        //Check if it's empty.
        if (parent.empty()){
            getParent = false;
            continue;
        }

        //Get the current decl as named.
        auto currentDecl = parent[0].get<clang::NamedDecl>();
        if (currentDecl && isa<clang::CXXRecordDecl>(currentDecl)){
            return dyn_cast<CXXRecordDecl>(currentDecl);
        }

        parent = Context->getParents(parent[0]);
    }

    return nullptr;
}

/**
 * Checks whether an entity is a class.
 * @param ctor The class.
 * @param className The name of the class.
 * @return Whether the two items compare.
 */
bool ParentWalker::isClass(const CXXConstructExpr* ctor, string className){
    //Get the underlying class.
    QualType type = ctor->getBestDynamicClassTypeExpr()->getType();
    if (type->isArrayType()) return false;

    auto record = ctor->getBestDynamicClassType();
    if (record == nullptr) return false;

    //Check the qualified name.
    if (record->getQualifiedNameAsString().compare(className)) return false;
    return true;
}

/**
 * Checks whether an entity is a function..
 * @param expr The call expression.
 * @param functionName The name of the function.
 * @return Whether the two items compare.
 */
bool ParentWalker::isFunction(const CallExpr *expr, string functionName){
    //Gets the underlying callee.
    if (expr->getCalleeDecl() == nullptr) return false;
    auto callee = expr->getCalleeDecl()->getAsFunction();
    if (callee == nullptr) return false;

    //Checks the value of the callee.
    if (callee->getQualifiedNameAsString().compare(functionName)) return false;
    return true;
}

/**
 * Gets arguments of a call expression.
 * @param expr The expression to get arguments.
 * @return The vector of arguments.
 */
vector<string> ParentWalker::getArgs(const CallExpr* expr){
    string sBuffer;

    //Creates a vector.
    vector<string> args;

    //Get the arguments for the CallExpr.
    int numArgs = expr->getNumArgs();
    auto argList = expr->getArgs();
    for (int i = 0; i < numArgs; i++){
        sBuffer = "";
        auto curArg = argList[i];

        //Pushes the argument string into a vector.
        llvm::raw_string_ostream strStream(sBuffer);
        curArg->printPretty(strStream, nullptr, Context->getPrintingPolicy());
        sBuffer = strStream.str();
        args.push_back(sBuffer);
    }

    return args;
}

/**
 * Checks whether a statement is in the system header.
 * @param statement The statement.
 * @return Whether its in the system header.
 */
bool ParentWalker::isInSystemHeader(const Stmt *statement) {
    if (statement == nullptr) return false;

    //Get the system header.
    bool isIn;
    try {
        //Get the source manager.
        auto &manager = Context->getSourceManager();

#ifdef CLANG_VER_LTE
        //Check if in header.
        statement->getLocStart();
        isIn = isInSystemHeader(manager, statement->getLocStart());
#else
        isIn = isInSystemHeader(manager, statement->getBeginLoc());
#endif
    } catch (...) {
        return false;
    }

    return isIn;
}

/**
 * Checks whether a decl is in the system header.
 * @param decl The declaration.
 * @return Whether its in the system header.
 */
bool ParentWalker::isInSystemHeader(const Decl *decl){
    if (decl == nullptr) return false;

    //Get the system header.
    bool isIn;
    try {
        //Get the source manager.
        auto &manager = Context->getSourceManager();

        //Check if in header.
#ifdef CLANG_VER_LTE
        isIn = isInSystemHeader(manager, decl->getLocStart());
#else
        isIn = isInSystemHeader(manager, decl->getBeginLoc());
#endif
    } catch (...) {
        return false;
    }

    return isIn;
}
/**
 * Generates a unique ID based on a decl.
 * @param decl The decl to generate the ID.
 * @return A string of the ID.
 */
string ParentWalker::generateID(const NamedDecl* decl){
    //Gets the canonical decl.
    decl = dyn_cast<NamedDecl>(decl->getCanonicalDecl());
    string name = "";

    //Generates a special name for function overloading.
    if (isa<FunctionDecl>(decl) || isa<CXXMethodDecl>(decl)){
        const FunctionDecl* cur = decl->getAsFunction();
        name = cur->getReturnType().getAsString() + "-" + decl->getNameAsString();
        for (int i = 0; i < cur->getNumParams(); i++){
            name += "-" + cur->parameters().data()[i]->getType().getAsString();
        }
    } else {
        name = decl->getNameAsString();
    }


    bool getParent = true;
    bool recurse = false;
    const NamedDecl* originalDecl = decl;

    //Get the parent.
    auto parent = Context->getParents(*decl);
    while(getParent){
        //Check if it's empty.
        if (parent.empty()){
            getParent = false;
            continue;
        }

        //Get the current decl as named.
        decl = parent[0].get<clang::NamedDecl>();
        if (decl) {
            name = generateID(decl) + "::" + name;
            recurse = true;
            getParent = false;
            continue;
        }

        parent = Context->getParents(parent[0]);
    }

    //Sees if no true qualified name was used.
    Decl::Kind kind = originalDecl->getKind();
    if (!recurse) {
        if (kind != Decl::Function && kind == Decl::CXXMethod){
            //We need to get the parent function.
            const DeclContext *parentContext = originalDecl->getParentFunctionOrMethod();

            //If we have nullptr, get the parent function.
            if (parentContext != nullptr) {
                string parentQualName = generateID(static_cast<const FunctionDecl *>(parentContext));
                name = parentQualName + "::" + originalDecl->getNameAsString();
            }
        }
    }

    //Finally, check if we have a main method.
    if (name.compare("int-main-int-char-**") == 0 || name.compare("int-main") == 0 ){
#if CLANG_VER_LTE
        name = Context->getSourceManager().getFilename(originalDecl->getLocStart()).str() + "--" + name;
#else
        name = Context->getSourceManager().getFilename(originalDecl->getBeginLoc()).str() + "--" + name;
#endif
    }

    return replaceMap(name);
}

string ParentWalker::StmtID(const Stmt* stmt){
  string name;
  int beginloc = stmt->getBeginLoc().getRawEncoding();
  ostringstream oss;
  oss << beginloc;
  return replaceMap(oss.str());
}

string ParentWalker::generateID(const Stmt* stmt){
    const NamedDecl* decl = nullptr;
    string name; 
    // name try stmt function-tryNum
    bool getParent = true;
    bool recurse = false;

    string type;
    if ( isa<CXXThrowExpr>(stmt) ) type = "throw";
    if ( isa<CXXCatchStmt>(stmt) ) type = "catch";
    if ( isa<CXXTryStmt>(stmt) ) type = "try";

    //Get the parent.
    auto parent = Context->getParents(*stmt);
    while(getParent){
        //Check if it's empty.
        if (parent.empty()){
            getParent = false;
            continue;
        }

        //Get the current decl as named.
        decl = parent[0].get<clang::NamedDecl>();
        if (decl) {
            name = generateName(decl) + "::" + name;
            recurse = true;       
            getParent = false;
            continue;
        }

        parent = Context->getParents(parent[0]);
    }
    string ret = name + type + StmtID(stmt);
    return replaceMap(ret);
}

/**
 * Generates a name based on a decl.
 * @param decl The declaration.
 * @return The string of the decl.
 */
string ParentWalker::generateName(const NamedDecl* decl){
    string name = decl->getQualifiedNameAsString();
    //Check if we have a main function.

    if ( name == "main" ){
#if CLANG_VER_LTE
        name = Context->getSourceManager().getFilename(decl->getLocStart()).str() + "-" + name;
#else
        name = Context->getSourceManager().getFilename(decl->getBeginLoc()).str() + "-" + name;
#endif
    }

    // add params
    const FunctionDecl* func = dynamic_cast<const FunctionDecl*>(decl);
    if ( func ){
      name += "-";
      bool first = true;
      for ( auto it = func->param_begin(); it != func->param_end(); ++it ){
        if ( first ) first = false;
        else name += "-";
        name += (*it)->getOriginalType().getAsString() ; 
      }
      name += "-";
    }

    return replaceMap(name);
}

/**
 * Gets the filename of the decl.
 * @param decl The declaration.
 * @return The filename the declaration is in.
 */
string ParentWalker::generateFileName(const NamedDecl* decl){
    //Gets the file name.
    SourceManager& SrcMgr = Context->getSourceManager();
#if CLANG_VER_LTE
    auto fullLoc = Context->getFullLoc(decl->getLocStart());
#else
    auto fullLoc = Context->getFullLoc(decl->getBeginLoc());
#endif
    if (!fullLoc.isValid()) return string();

    string fileName = SrcMgr.getFilename(fullLoc).str();

    //Use boost to get the absolute path.
    boost::filesystem::path fN = boost::filesystem::path(fileName);
    string newPath = canonical(fN.normalize()).string();
    return replaceMap(newPath);
}

string ParentWalker::generateFileName(const Stmt* stmt){
    //Gets the file name.
    SourceManager& SrcMgr = Context->getSourceManager();
#if CLANG_VER_LTE
    auto fullLoc = Context->getFullLoc(stmt->getLocStart());
#else
    auto fullLoc = Context->getFullLoc(stmt->getBeginLoc());
#endif
    if (!fullLoc.isValid()) return string();

    string fileName = SrcMgr.getFilename(fullLoc).str();

    //Use boost to get the absolute path.
    boost::filesystem::path fN = boost::filesystem::path(fileName);
    string newPath = canonical(fN.normalize()).string();
    return replaceMap(newPath);
}

/**
 * Gets the location of the function decl.
 * @param decl The decl to add.
 */
void ParentWalker::recordParentClassLoc(const FunctionDecl* decl){
    if (!decl) return;
    auto funcDec = decl->getDefinition();
    if (!funcDec) return;

    //Gets the parent class.
    const CXXRecordDecl* classDec = getParentClass(decl);
    if (!classDec) return;
    ZeldaNode* parentClass = graph->findNode(generateID(classDec));
    if (!parentClass) return;

    //Generates the filename.
    string baseFN = generateFileName(funcDec);
    parentClass->addMultiAttribute(FILENAME_ATTR, baseFN);
}

/**
 * Validates a string argument.
 * @param name The argument to validate.
 * @return The cleaned up string.
 */
string ParentWalker::validateStringArg(string name){
    //Checks the topic name.
    std::string prefix("\"");

    //Checks if we have a variable.
    if (name.at(0) != '"') {
        cerr << "Zelda Warning: Variable is used to record topic name. Topic name is " << name << "!" << endl;
    }

    //Removes the quotes if present.
    if (!name.compare(0, prefix.size(), prefix)){
        name = name.substr(1);
    }
    if (!name.compare(name.size() - 1, prefix.size(), prefix)){
        name = name.substr(0, name.size() - 1);
    }

    return name;
}

/**
 * Generates a TA model.
 * @param graph The graph to generate.
 * @param fileName The filename to save.
 * @return Status code.
 */
int ParentWalker::generateTAModel(TAGraph* graph, string fileName){
    //Purge the edges.
    graph->purgeUnestablishedEdges(true);

    //Gets the string for the model.
    bool ret = graph->getTAModel(fileName);

    return 0;
}

/**
 * Checks whether an item is in a system header file.
 * @param manager The source manager.
 * @param loc The source location.
 * @return Whether its in a system header file.
 */
bool ParentWalker::isInSystemHeader(const SourceManager& manager, SourceLocation loc) {
    //Get the expansion location.
    auto expansionLoc = manager.getExpansionLoc(loc);

    //Check if we have a valid location.
    if (expansionLoc.isInvalid()) {
        return false;
    }

    //Get if we have a system header.
    bool sysHeader = manager.isInSystemHeader(loc);
    if (sysHeader) return sysHeader;

    //Now, check to see if we have a Zelda library.
    string libLoc = expansionLoc.printToString(manager);
    libLoc = libLoc.substr(0, libLoc.find(":"));
    for ( string ext: headerExt ){
      if (boost::algorithm::ends_with(libLoc, "." + ext) ){
          return true;
      }
    }

    return false;
}

void ParentWalker::updateNodes(ZeldaNode* src, ZeldaNode* dest, const string& FLAG){
  int count = src->getCountAttribute(FLAG);
  src->addCountAttribute(FLAG, 1);
  dest->addCountAttribute("order", count);
}

void ParentWalker::processExceptions(){
  // contains all throw nodes
  for ( auto curGraph : graphList ){
    currentGraph = curGraph;
    vector<ZeldaNode*> throws = curGraph->findNodesByType(ZeldaNode::THROW);

    // for each node determine where it is thrown to
    for ( auto throwNode: throws ){
      throwNode->addBoolAttribute("intermodual",false,true,false);
      throwNode->addBoolAttribute("intermodualCatch",false,true,false);
      vector<ZeldaEdge*> throwEdges = currentGraph->findEdgesByDst(throwNode->getName());

      for ( int i = 0; i < throwEdges.size() ; ++i ){

        ZeldaEdge* edge = throwEdges.at(i);
        if ( ! edge )  continue;
        ZeldaNode* contains = edge->getSource();
        if ( ! contains ) continue;

        ZeldaEdge* ret = nullptr;
        vector<ZeldaEdge*> newThrows;

        switch ( contains->getType() ){
          case ZeldaNode::FUNCTION:
            // throw is in a function
            newThrows = processFunction(contains, throwNode);
            for ( ZeldaEdge* rets : newThrows ){
              throwEdges.emplace_back(rets);
            }
            continue;
          case ZeldaNode::TRY:
            // throw from a try block
            ret = processTry(contains, throwNode, edge);
            if ( ret && ret != edge ) {
              throwEdges.emplace_back(ret);
            }
            continue;
          case ZeldaNode::CATCH:
            // throw from a catch block
            ret = processCatch(contains, throwNode, edge);
            if ( ret && ret != edge ){
              throwEdges.emplace_back(ret);
            }
            continue;
        }
      }
    }
  }

}

vector<ZeldaEdge*> ParentWalker::processFunction(ZeldaNode* func, ZeldaNode* thrown){
  // this function throws -> find all the calls to this function and add a throw edge
  //     from thrown to the context of the call
  if ( ! currentGraph->doesEdgeExist( func->getID(), thrown->getID(), ZeldaEdge::FUNC_THROWS ) ){
        ZeldaEdge* newEdge = new ZeldaEdge( func, thrown, ZeldaEdge::FUNC_THROWS );
        currentGraph->addEdge(newEdge);
  }
  vector<ZeldaEdge*> calls = currentGraph->findEdgesByTypeAndDst(func, ZeldaEdge::CONTEXT);
  set<string> paths = thrown->getMultiAttribute("functions");
  vector<ZeldaEdge*> rets;
  string functionName = func->getName();

  if ( paths.empty() ) { thrown->addSingleAttribute("function", functionName); }
  if ( func->getBoolAttribute("isRecursive") ) return rets;

  for ( auto context: paths ){
    if ( context == functionName ){
      func->addBoolAttribute("isRecursive", true); 
      return rets;
    }
  }
  thrown->addMultiAttribute("functions", functionName );
  thrown->addCountAttribute("funcCount", 1);
  thrown->addMultiAttribute("path", functionName );
  for ( auto edge: calls ){
    ZeldaNode* callScope = edge->getSource();
    if ( callScope ){
      if ( ! currentGraph->doesEdgeExist( callScope->getID(), thrown->getID(), ZeldaEdge::THROWS ) ){
        ZeldaEdge* newEdge = new ZeldaEdge( callScope, thrown, ZeldaEdge::THROWS );
        currentGraph->addEdge(newEdge);
        rets.emplace_back(newEdge);
        string file1 = callScope->getSingleAttribute("filename"); 
        string file2 = thrown->getSingleAttribute("filename");
        // determine if catch and throw are in the same file
        if ( ! file1.empty() && file1 != file2 && isCFile(file1) ) {
          thrown->addBoolAttribute("intermodual",true);
        } 
      }
    }
  }

  return rets;
}

ZeldaEdge* ParentWalker::processTry(ZeldaNode* tryNode, ZeldaNode* thrown, ZeldaEdge* edge){
  vector<ZeldaEdge*> edges = currentGraph->findEdgesBySrc(tryNode->getName());
  string thrownType = thrown->getSingleAttribute("type");
  int numCatches = tryNode->getCountAttribute("countCatch");
  vector<ZeldaEdge*> catches;
  int foundCatch = 0;
  // find catch edges in order
  for ( auto it = edges.begin(); it != edges.end() && foundCatch < numCatches; ){
    ZeldaEdge* edge = *it;
    ZeldaNode* dest = edge->getDestination();
    if ( !dest ){ ++it; continue; }
    if ( edge->getType() == ZeldaEdge::CONTEXT && dest->getType() == ZeldaNode::CATCH 
        && dest->getCountAttribute("order") == foundCatch ){
          catches.emplace_back(edge);
          it = edges.begin();
          ++foundCatch;
    } else {
      ++it;
    }
  }

  // go through catches in order
  // if the type matches catch type, change edge to catch
  // otherwise check next edge
  // if no edge match, exception is thrown to tryNodes's context

  for ( auto it = catches.begin(); it != catches.end(); ++it ){
    auto cEdge = *it;
    ZeldaNode* catchNode = cEdge->getDestination();
    if ( ! catchNode) continue;
    if ( matchesType( thrown, catchNode )){

      string file1 = catchNode->getSingleAttribute("filename"); 
      string file2 = thrown->getSingleAttribute("filename");
      // determine if catch and throw are in the same file
      if ( ! file1.empty() && file1 != file2 && isCFile(file1) ) {
        thrown->addBoolAttribute("intermodualCatch",true);
        thrown->addBoolAttribute("intermodual",true);
      }
 
      thrown->addMultiAttribute("caughtBy", catchNode->getName() );
      return replaceEdge( edge, catchNode, thrown, ZeldaEdge::CATCHES );
    } else {
      thrown->addMultiAttribute("seenBy", catchNode->getName() );
    } 
  }
  
  ZeldaNode* outerNode = currentGraph->findNodeByName(tryNode->getSingleAttribute("context"));
  if ( outerNode == nullptr ){
    return nullptr; 
  }
   
  return replaceEdge( edge, outerNode, thrown, ZeldaEdge::THROWS );
  
}

static void cleanType(string &s){
  bool cont = true;
  while ( cont ){
    if ( s.length() > 6 && s.substr(0,6) == "const " ){
      s = s.substr(6);
    }
    else if ( s.length() > 2 && s.substr(s.length()-2) == " &" ){
      s = s.substr(0,s.length()-2);
    }
    else {
      cont = false;
    }
  }
}

bool ParentWalker::matchesType(ZeldaNode* thrown, ZeldaNode* match, bool subType){
  string thrownType = thrown->getSingleAttribute("type");
  string matchType  = match->getSingleAttribute("type");
  if ( matchType == "all" ) return true;

  cleanType(thrownType);
  cleanType(matchType);

  if ( thrownType == matchType ){
    return true;
  }
  if ( ! subType ) return false;
  vector<ZeldaEdge*> subclasses = currentGraph->findEdgesByTypeAndSrc(match, ZeldaEdge::INHERITS );
  
  for ( auto edge: subclasses ){
    ZeldaNode* subclass = edge->getDestination();
    string subType = subclass->getSingleAttribute("type");
    cleanType(subType);
    if ( subType == thrownType ) return true;
  }
  
  return false;
}

ZeldaEdge* ParentWalker::replaceEdge(ZeldaEdge* oldEdge, ZeldaNode* newSrc, ZeldaNode* newDst, ZeldaEdge::EdgeType type){
  if ( ! currentGraph->doesEdgeExist( newSrc->getID(), newDst->getID(), type )){
    ZeldaEdge* newEdge = new ZeldaEdge( newSrc, newDst, type );
    newDst->addMultiAttribute("path", oldEdge->getSource()->getName() );
    oldEdge->setType(ZeldaEdge::THROWPATH);
    //currentGraph->removeEdge(oldEdge->getSourceID(), oldEdge->getDestinationID(), oldEdge->getType(), true);
    currentGraph->addEdge(newEdge);
    return newEdge;
  }
  return nullptr;
}

ZeldaEdge* ParentWalker::processCatch(ZeldaNode* catchNode, ZeldaNode* thrown, ZeldaEdge* edge){
  if ( edge->getType() == ZeldaEdge::CATCHES ) {
    // either the catch doesn't throw, throws a different object, or rethrows
    ZeldaNode* catchNode = edge->getSource();
    if ( ! catchNode ) {
      return nullptr;
    }
    string thrownType = thrown->getSingleAttribute("type");


    vector<ZeldaEdge*> edges = currentGraph->findEdgesBySrc(catchNode->getID());
    // find catch edges in order
    for ( auto it = edges.begin(); it != edges.end() ; ++it ){
      ZeldaEdge* edge = *it;
      ZeldaNode* dest = edge->getDestination();
      if ( !dest ) continue;
      if ( edge->getType() == ZeldaEdge::RETHROWS && dest->getType() == ZeldaNode::RETHROW ){
          // a throw is being rethrown -> can add an edge with the type thrown
          ZeldaEdge* newEdge = new ZeldaEdge(catchNode, thrown, ZeldaEdge::THROWS);
          currentGraph->addEdge(newEdge);
          return newEdge;
       }
      }
      thrown->addMultiAttribute("path", catchNode->getName() );
      thrown->addMultiAttribute(catchFlag, catchNode->getName() );
    
  } 
  // find outer stmt associated with this catch
  else if ( edge->getType() == ZeldaEdge::THROWS ){
    ZeldaNode* tryNode = currentGraph->findNodeByName(catchNode->getSingleAttribute("context"));
      
    ZeldaNode* outerNode = currentGraph->findNodeByName(tryNode->getSingleAttribute("context"));

    if ( outerNode == nullptr ){

      return nullptr; 
    }
    return replaceEdge( edge, outerNode, thrown, edge->getType() );
  }
  return nullptr;
}

