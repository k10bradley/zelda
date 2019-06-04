/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZeldaMaster.cpp
//
// Created By: Bryan J Muscedere
// Date: 13/05/17.
//
// Updated By Kirsten Bradley 2019 for Zelda
//
// Driver source code for the Zelda program. Handles
// commands and passes it off to the ZeldaHandler to
// translate it into a Zelda action. Also handles
// errors gracefully.
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
#include <pwd.h>
#include <zconf.h>
#include <vector>
#include <boost/algorithm/string/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>
#include "ZeldaHandler.h"
#include "../Walker/Classifier.h"

using namespace std;
using namespace boost::filesystem;
namespace po = boost::program_options;

/** Zelda Command Handler */
static ZeldaHandler* masterHandle;

/**
 * Takes in a line and tokenizes it to
 * a vector by spaces.
 * @param line The line to tokenize.
 * @return A vector of words.
 */
vector<string> tokenizeBySpace(string line){
    vector<string> result;
    istringstream iss(line);
    for(std::string s; iss >> s;){
        cout << s << endl;
        result.push_back(s);
    }

    return result;
}


/**
 * Prints a simple about message.
 */
void handleAbout() {
    cout << "ZELDA - Zee Exception Length and Destionation Analyzer" << endl
         << "University of Waterloo, Copyright 2019" << endl
         << "Licensed under GNU Public License v3" << endl << endl
         << "This program is distributed in the hope that it will be useful," << endl
         << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << endl
         << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << endl
         << "GNU General Public License for more details." << endl << endl
         << "----------------------------------------------------" << endl << endl;
}

string setupOutputDir(path& location){
  string outputDir = location.string() + "/ZeldaAnalysis";
  string attempt = outputDir;
  int i = 2;
  while ( exists(attempt) ){
    ostringstream oss; oss << i;
    attempt = outputDir + oss.str();
    ++i;
  }
  outputDir = attempt;
  cout << "Creating Output Directory: " << outputDir << endl;
  create_directory(outputDir);    
  Classifier::setClassifyFile(outputDir);

  return outputDir;
}


/**
 * Driver method for the OUTPUT argument.
 * Allows users to specify what graphs to output.
 * Also performs basic sanity checking.
 * @param line The line to perform command line argument parsing.
 * @param desc The program options information for this command.
 */
void outputGraphs(path& startDir){
    // make a directory for output
    bool success;

    //Now, outputs the graphs
    success = masterHandle->outputModel(startDir.string());
    
    if (!success) {
        cerr << "There was an error outputting all graphs to TA models." << endl;
    } else {
        cout << "Contribution networks created successfully." << endl;
    }
}

/**
 * Driver method for the ADD command.
 * Allows users to specify files and folders to add.
 * @param line The line with all the command line arguments.
 */
void addFiles(string line){
    //Tokenize by space.
    vector<string> tokens = tokenizeBySpace(line);

    //Next, we check for errors.
    if (tokens.size() == 0) {
        cerr << "Error: You must include at least one file or directory to process." << endl;
        return;
    }

    //Next, we loop through to add these files.
    for (auto it = tokens.begin(); it != tokens.end(); ++it){
        path curPath = *it;

        //Check if the element exists.
        if (!exists(curPath)){
            return;
        }
        int numAdded = masterHandle->addByPath(curPath);

        //Checks whether the system is a directory.
        if (is_directory(curPath)){
           cout << numAdded << " source files were added from the directory " << curPath.filename() << "!" << endl;
        } else {
            cout << "Added C++ file " << curPath.filename() << "!" << endl;
        }
    }
}


/**
 * Simple method that prints the header for Zelda
 * to display to users what program they're running.
 */
void printHeader(){
    cout << "ZELDA: Exception Length and Destination Analyzer" << endl;
}


/**
 * Main method that drives the program. Prints the
 * header and then determines what mode to be in.
 * @param argc The number of arguments
 * @param argv Char** array of arguments
 * @return Return code denoting success.
 */
int main(int argc, const char** argv) {
    ZeldaHandler local;
    masterHandle = &local;

    bool simpleMode = argc != 1;

    if ( argc == 1 ){
      cerr << "Must include at least one file to analyze." << endl;
      return 1;
    }

    //Print the header first.
    printHeader();

    // runs the necessary steps for analysis
    vector<path> dirs;
    
    // determines files from args
    for ( int i = 1; i < argc; ++i ){
      string path = argv[i];
      //cout << path << endl;
      addFiles(path);
      if ( is_directory(path) ){
        dirs.emplace_back(path);
      }
    }

    string outputDir = setupOutputDir(dirs[0]);
    
    cout << "Processing file(s)..." << endl << "This may take some time!" << endl << endl;
    bool success = masterHandle->processAllFiles();

    //Checks the success of the operation.
    if (success) cout << "Zelda contribution graph was created successfully!" << endl
                << "Graph number is #" << masterHandle->getNumGraphs() - 1 << "." << endl;

    cout << "Searching for compilation databases..." << endl;
    success = masterHandle->resolveComponents(dirs);

    if (success){
        cout << "Components were resolved in all models!" << endl;
    } else {
        cerr << "There was an error resolving components in the models." << endl;
    }
    path out = outputDir; 
    outputGraphs(out);
    return 0;
}
