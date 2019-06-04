/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZeldaHandler.cpp
//
// Created By: Bryan J Muscedere
// Date: 15/05/17.
// Updated By Kirsten Bradley 2019 for ZELDA
//
//
// Driver that connects to all the backend functions
// and coordinates them based on a user's commands
// from the master driver.
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

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <regex>
#include <fstream>
#include <boost/foreach.hpp>
#include "ZeldaHandler.h"
#include "../Walker/ExceptConsumer.h"
#include "../JSON/json.h"
//#include "../Configuration/ScenarioWalker.h"

using namespace std;
using namespace clang::tooling;


/**
 * Constructor that prepares the ZeldaHandler.
 */
ZeldaHandler::ZeldaHandler():Category{"Zelda"}{
  //Sets the C, C++ extensions.
  ext.push_back(".C");
  ext.push_back(".c");
  ext.push_back(".cpp");
  ext.push_back(".CPP");
  ext.push_back(".CXX");
  ext.push_back(".cxx");
  ext.push_back(".cc");
  ext.push_back(".CC");
  ext.push_back(".c++");
  ext.push_back(".C++");
}

/** Deletes all the TA graphs */
ZeldaHandler::~ZeldaHandler(){
  ParentWalker::deleteTAGraphs();
}

/**
 * Gets the number of already generated graphs.
 * @return Number of generated graphs.
 */
int ZeldaHandler::getNumGraphs(){
  return ParentWalker::getNumGraphs();
}

/**
 * Number of files currently in the queue.
 * @return The size of the queue.
 */
int ZeldaHandler::getNumFiles(){
  return (int) files.size();
}

/**
 * Gets a list of all the files.
 * @return A vector of all the files.
 */
vector<string> ZeldaHandler::getFiles(){
  vector<string> strFiles;
  for (path file : files) strFiles.push_back(canonical(file).string());

  return strFiles;
}


/**
 * Runs through all files in the queue and generates a graph.
 * @param minimalWalk Boolean that indicates what ZeldaWalker to use.
 * @param loadLoc The location to load the libraries to ignore (OPTIONAL).
 * @return Boolean indicating success.
 */
bool ZeldaHandler::processAllFiles(){
  bool success = true;

  //Creates the command line arguments.
  int argc = 0;
  char** argv = prepareArgs(&argc);

  //Gets the list of files.
  const vector<string> fileList = getFileList();

  //Sets up the processor.
  CommonOptionsParser OptionsParser(argc, (const char**) argv, Category);
  ExceptConsumer::setClassifyFile(current_path().string());
  cerr << current_path().string() << endl;

  int fileSplit = getNumFiles();
  for (int i = 0; i < getNumFiles(); i += fileSplit){

    vector<string> toProcess;
    toProcess = fileList;


    ClangTool* Tool = new ClangTool(OptionsParser.getCompilations(), toProcess);
    int code = Tool->run(newFrontendActionFactory<ZeldaAction>().get());

    //Gets the code and checks for warnings.
    if (code != 0) {
      cerr << "Warning: Compilation errors were detected." << endl;
      success = false;
    }

    delete Tool;
  }
  

  //Shifts the graphs.
  ParentWalker::endCurrentGraph();

  //Clears the graph.
  files.clear();

  //Cleans up memory.
  for (int i = 0; i  < argc; i++) delete argv[i];
  delete[] argv;

  //Returns the success code.
  return success;
}


/**
 * Outputs an individual TA model to TA format.
 * @param modelNum The number of the model to output.
 * @param fileName The filename to output as.
 * @return Boolean indicating success.
 */
bool ZeldaHandler::outputModel(std::string fileName){
  string lines = fileName;
  lines += "/lineCounts.csv";
  std::ofstream ofs{lines};
  ofs << "total," << Classifier::getFunctionCode() << endl << "exception," << Classifier::getExceptionCode()  << endl
      << "catch," << Classifier::getCatchCode() << endl;;

  string tryFile = fileName;
  string catchFile = fileName;
  string neitherFile = fileName;
  tryFile += "/tries.csv";
  catchFile += "/catches.csv";
  neitherFile += "/generic.csv";
  std::ofstream tries{tryFile};
  std::ofstream catches{catchFile};
  std::ofstream nonExcept{neitherFile};

  Counter::printData(0, tries);
  Counter::printData(1, catches);
  Counter::printData(2, nonExcept);

  string taFile = fileName + "/" + DEFAULT_FILENAME + DEFAULT_EXT;

  //First, check if the number if valid.
  //if (modelNum < 0 || modelNum > getNumGraphs() - 1) return false;

  int succ = ParentWalker::generateTAModel(0, taFile);
  if (succ == 0) {
    cerr << "Error writing to " << fileName << "!" << endl
      << "Check the file and retry!" << endl;
    return false;
  }
  //ParentWalker::deleteTAGraph(modelNum);
  return true;
}


/**
 * Resolves the components in the model.
 * @param databasePaths The paths to potential compilation databases.
 * @return A boolean indicating success.
 */
bool ZeldaHandler::resolveComponents(std::vector<path> databasePaths){
  map<string, path> databaseMap;

  //First, from the path we resolve all compilation databases.
  for (path curPath : databasePaths){
    addDirectory(curPath, databaseMap);
  }

  //Count the number of keys.
  cout << databaseMap.size() << " compilation databases were detected!" << endl;
  if (databaseMap.size() == 0) return false;
  cout << "Now resolving..." << endl;

  //Go through each JSON file.
  map<string, vector<string>> results = resolveJSON(databaseMap);

 

  //Goes through each of the graphs.
  bool res = ParentWalker::resolveAllTAModels(results);
  return res;
}


/**
 * Adds a file/directory by path.
 * @param curPath The path to add.
 * @return Integer indicating the number of files added.
 */
int ZeldaHandler::addByPath(path curPath){
  int num = 0;

  //Determines what the path is.
  if (is_directory(curPath)){
    num = addDirectory(curPath);
  } else {
    num = addFile(curPath);
  }

  return num;
}



/**
 * Generates an argv array based on the files in the queue and Clang's input format.
 * @param argc The number of tokens.
 * @return The new argv command.
 */
char** ZeldaHandler::prepareArgs(int *argc){
  int size = BASE_LEN + (int) files.size();

  //Sets argc.
  *argc = size;

  //Next, argv.
  char** argv = new char*[size];

  //Copies the base start.
  argv[0] = new char[DEFAULT_START.size() + 1];
  argv[1] = new char[INCLUDE_DIR_LOC.size() + 1];

  //Next, moves them over.
  strcpy(argv[0], DEFAULT_START.c_str());
  strcpy(argv[1], INCLUDE_DIR_LOC.c_str());

  //Next, loops through the files and copies.
  for (int i = 0; i < files.size(); i++){
    string curFile = canonical(files.at(i)).string();
    argv[i + BASE_LEN] = new char[curFile.size() + 1];
    strcpy(argv[i + BASE_LEN], curFile.c_str());
  }

  return argv;
}

/**
 * Simply converts the path vector into a string
 * vector.
 * @return A string vector of files.
 */
const vector<string> ZeldaHandler::getFileList(){
  vector<string> results;
  for (path curItem : files){
    results.push_back(canonical(curItem).string());
  }

  return results;
}

/**
 * Adds a file to the queue.
 * @param file The file to add.
 * @return Returns 1.
 */
int ZeldaHandler::addFile(path file){
  //Gets the string that is added.
  files.push_back(file);
  return 1;
}

/**
 * Recursively adds a directory to the queue.
 * @param directory The directory to add.
 * @return The number of files added.
 */
int ZeldaHandler::addDirectory(path directory){
  int numAdded = 0;
  vector<path> interiorDir = vector<path>();
  directory_iterator endIter;

  //Start by iterating through and inspecting each file.
  for (directory_iterator iter(directory); iter != endIter; iter++){
    //Check what the current file is.
    if (is_regular_file(iter->path())){
      //Check the extension.
      string extFile = extension(iter->path());
 
      //Iterates through the extension vector.
      for (auto end: ext){
        //Checks the file.
        if (extFile == end){
          cout << iter->path() << endl;
          numAdded += addFile(iter->path());
          break;
        }
      }
    } else if (is_directory(iter->path())){
      //Add the directory to the search system.
      interiorDir.push_back(iter->path());
    }
  }

  //Next, goes to all the internal directories.
  for (path cur : interiorDir){
    numAdded += addDirectory(cur);
  }

  return numAdded;
}


/**
 * Adds a directory to the queue.
 * @param directory The directory to add.
 * @param databases A collection of databases.
 * @return A map of databases per file.
 */
void ZeldaHandler::addDirectory(path directory, map<string, path>& databases){
  vector<path> interiorDir = vector<path>();
  directory_iterator endIter;

  //Start by iterating through and inspecting each file.
  for (directory_iterator iter(directory); iter != endIter; iter++){
    //Check what the current file is.
    if (is_regular_file(iter->path())){
      //Check the file name.
      string fn = iter->path().filename().string();

      //Adds if its a compilation database.
      if (fn == COMPILATION_DB_NAME ){
        databases[directory.string()] = iter->path();
      }
    } else if (is_directory(iter->path())){
      //Add the directory to the search system.
      interiorDir.push_back(iter->path());
    }
  }

  //Next, goes to all the internal directories.
  for (path cur : interiorDir){
    addDirectory(cur, databases);
  }

}

/**
 * Generates a collection of file to components.
 * @param databases The list of JSON databases.
 * @return A mapping from file to component name.
 */
map<string, vector<string>> ZeldaHandler::resolveJSON(map<string, path>& databases){
  map<string, vector<string>> resultMap;

  //Loop through the list of databases.
  for (auto entry : databases){
    //Gets the JSON objects.
    Json::Value root;
    path copy = entry.second;


    //Load the JSON file.
    std::ifstream jsonFile(entry.second.string(), std::ifstream::binary);
    jsonFile >> root;
    if (root.isNull()) continue;

    //Now, we loop through each entry.
    bool loop = true;
    int i = 0;
    while (loop){
      Json::Value node = root[i];
      if (node.isNull()){
        loop = false;
        continue;
      }

      //Get the file.
      string filename = copy.string() + "/" + node["file"].asString();
      resultMap[filename].push_back(entry.first);

      i++;
    }
    jsonFile.close();
  }

  return resultMap;
};

