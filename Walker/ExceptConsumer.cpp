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

#include "ExceptConsumer.h"

using namespace std;

vector<string> ExceptConsumer::libraries = vector<string>();
string ExceptConsumer::classifyFile = "";

/**
 * Creates a Zelda consumer.
 * @param Context The AST context.
 */
ExceptConsumer::ExceptConsumer(ASTContext *Context) : exception{Context}, counter{Context}, classify{Context}, walker{Context} {}

/**
 * Handles the AST context's translation unit. Tells Clang to traverse AST.
 * @param Context The AST context.
 */
void ExceptConsumer::HandleTranslationUnit(ASTContext &Context) {
//        walker.addLibrariesToIgnore(ExceptConsumer::libraries);
//        walker.TraverseDecl(Context.getTranslationUnitDecl());
        
        counter.addLibrariesToIgnore(ExceptConsumer::libraries);
        counter.TraverseDecl(Context.getTranslationUnitDecl());

//        exception.addLibrariesToIgnore(ExceptConsumer::libraries);
//        exception.TraverseDecl(Context.getTranslationUnitDecl());

//        classify.addLibrariesToIgnore(ExceptConsumer::libraries);
//        classify.TraverseDecl(Context.getTranslationUnitDecl());
      
}
void ExceptConsumer::setClassifyFile(const std::string& file){
  classifyFile = file;
}

/**
 * Sets the libraries to ignore.
 * @param libraries The libraries to ignore.
 */
void ExceptConsumer::setLibrariesToIgnore(vector<string> libraries){
    ExceptConsumer::libraries = libraries;
}

/**
 * Creates an AST consumer to "eat" the AST.
 * @param Compiler The compiler instance to process.
 * @param InFile The input file.
 * @return A pointer to the AST consumer.
 */
std::unique_ptr<ASTConsumer> ZeldaAction::CreateASTConsumer(CompilerInstance &Compiler, StringRef InFile) {
    return std::unique_ptr<ASTConsumer>(new ExceptConsumer(&Compiler.getASTContext()));
}
