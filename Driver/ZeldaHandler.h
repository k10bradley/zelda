/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZeldaHandler.h
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

#ifndef REX_REXHANDLER_H
#define REX_REXHANDLER_H

#include <string>
#include <vector>
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include <llvm/Support/CommandLine.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

class ZeldaHandler {
public:
    /** Constructors/Destructors */
    ZeldaHandler();
    ~ZeldaHandler();

    /** Getters */
    int getNumGraphs();
    int getNumFiles();
    std::vector<std::string> getFiles();

    /** Processing Systems */
    bool processClangToolCode(int argc, const char** argv);
    bool processAllFiles();


    /** Output Helpers */
    bool outputModel(std::string fileName);

    /** TA Helpers */
    bool resolveComponents(std::vector<path> databasePaths);

    /** Add and Remove Functions */
    int addByPath(path curPath);

private:
    /** Default Arguments */
    const std::string DEFAULT_EXT = ".ta";
    const std::string DEFAULT_FILENAME = "out";
    const std::string DEFAULT_START = "./Zelda";
    const std::string INCLUDE_DIR = CLANG_INCLUD_DIR;
    const std::string INCLUDE_DIR_LOC = "--extra-arg=-I" + INCLUDE_DIR;
    static const std::string DEFAULT_LOAD;
    const int BASE_LEN = 2;
    const std::string COMPILATION_DB_NAME = "compile_commands.json";


    /** Member Variables */
    std::vector<path> files;
    std::vector<std::string> ext;
    llvm::cl::OptionCategory Category;

    /** Arg Helper Methods */
    char** prepareArgs(int *argc);
    const std::vector<std::string> getFileList();

    /** Add/Remove Helper Methods */
    int addFile(path file);
    int addDirectory(path directory);

    /** Resolve Helper Methods */
    void addDirectory(path directory, std::map<std::string, path>& databases);
    std::map<std::string,std::vector<std::string>> resolveJSON(std::map<std::string, path>& databases);

};

#endif //REX_REXHANDLER_H
