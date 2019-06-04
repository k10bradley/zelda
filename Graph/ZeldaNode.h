/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZeldaNode.h
//
// Created By: Bryan J Muscedere
// Date: 10/04/17.
// Modified by Kirsten Bradley 2019 for Zelda
//
// Maintains an node structure. Basic
// system for storing information about
// edge information and for printing to
// TA.
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

#ifndef REX_REXNODE_H
#define REX_REXNODE_H

#include <map>
#include <set>
#include <string>

class ZeldaNode {
public:
    //Node Type Information
    enum NodeType {FUNCTION, VARIABLE, CLASS, FILE, TRY, CATCH, THROW, RETHROW};
    static std::string typeToString(NodeType type);

    //Constructor/Destructor
    ZeldaNode(std::string ID, NodeType type);
    ZeldaNode(std::string ID, std::string name, NodeType type);
    ~ZeldaNode();

    //Getters
    std::string getID();
    std::string getName();
    NodeType getType();
    int getCountAttribute(std::string key);
    bool getBoolAttribute(std::string key);
    std::string getSingleAttribute(std::string key);
    std::set<std::string> getMultiAttribute(std::string key);
    int getNumAttributes();

    //Setters
    void setID(std::string newID);
    void setName(std::string newName);
    void setType(NodeType newType);

    //Attribute Managers
    void addSingleAttribute(const std::string& key, std::string value);
    void addCountAttribute(const std::string& key, int value = 0);
    void addMultiAttribute(const std::string& key, std::string value);
    void addBoolAttribute(const std::string& key, bool value, bool cumulative = false, bool isAnd = true);

    //TA Generators
    std::string generateTANode();
    std::string generateTAAttribute();

private:
    std::string ID;
    std::string name;
    NodeType type;

    std::map<std::string, std::string> singleAttributes;
    std::map<std::string, bool> boolAttributes;
    std::map<std::string, int> countAttributes;
    std::map<std::string, std::set<std::string>> multiAttributes;

    const std::string INSTANCE_FLAG = "$INSTANCE";
    const std::string LABEL_FLAG = "label";
};

#endif //REX_REXNODE_H
