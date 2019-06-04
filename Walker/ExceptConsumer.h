/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExceptConsumer.cpp
//
// Created By: Bryan J Muscedere
// Date: 08/07/17.
// Modified by Kirsten Bradley in 2019 for Zelda
//
// Sets up the AST walker components that
// Zelda uses to walk through Clang's AST.
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

#ifndef EXCEPT_CONSUMER_H
#define EXCEPT_CONSUMER_H

#include <string>
#include <vector>
#include "ExceptWalker.h"
#include "Classifier.h"
#include "ZeldaWalker.h"
#include "Counter.h"


class ExceptConsumer : public ASTConsumer {
public:
    //Constructor/Destructor
    explicit ExceptConsumer(ASTContext *Context);
    virtual void HandleTranslationUnit(ASTContext &Context);

    //Mode Functions
    enum Mode {EXCEPT, CLASSIFY};

    static void setClassifyFile(const std::string&);

    //Library Functions
    static void setLibrariesToIgnore(std::vector<std::string> libraries);

    static std::string classifyFile;
private:

    ExceptWalker exception;
    Classifier classify;
    ZeldaWalker walker;
    Counter counter;

    static std::vector<std::string> libraries;
};

class ZeldaAction : public ASTFrontendAction {
public:
    //Consumer Functions
    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler, StringRef InFile);
};

#endif
