/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZeldaNode.cpp
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

#include "MD5.h"
#include <cstring>
#include <algorithm>
#include <sstream>
#include "ZeldaNode.h"

using namespace std;

static const int printLen = 200;

/**
 * Converts an node type to a string representation.
 * @param type The node type.
 * @return The string representation.
 */
string ZeldaNode::typeToString(ZeldaNode::NodeType type){
    switch (type){
        case FUNCTION:
            return "cFunction";

        case VARIABLE:
            return "cVariable";

        case CLASS:
            return "cClass";

        case FILE:
            return "cFile";

        case TRY:
            return "cTry";
        case CATCH:
            return "cCatch";
        case THROW:
            return "cThrow";
        case RETHROW:
            return "cRethrow";
    }
    return "cRoot";
}

/**
 * Creates a Zelda node with only an ID.
 * @param ID The ID.
 * @param type The node type.
 */
ZeldaNode::ZeldaNode(std::string ID, NodeType type){
    this->ID = ID;
    this->name = ID;
    this->type = type;
}

/**
 * Creates a Zelda node with an ID and name.
 * @param ID The ID.
 * @param name The name.
 * @param type The node type.
 */
ZeldaNode::ZeldaNode(std::string ID, std::string name, NodeType type){
    this->ID = ID;
    this->name = name;
    this->type = type;

    //Add the label.
    addSingleAttribute(LABEL_FLAG, name);
}

/**
 * Destructor
 */
ZeldaNode::~ZeldaNode(){ }

/**
 * Gets the ID.
 * @return The node ID.
 */
std::string ZeldaNode::getID(){
    return ID;
}

/**
 * Gets the name.
 * @return The node name.
 */
std::string ZeldaNode::getName(){
    return name;
}

/**
 * Gets the type.
 * @return The node type.
 */
ZeldaNode::NodeType ZeldaNode::getType(){
    return type;
}

/**
 * Gets a single attribute by key.
 * @param key The key.
 * @return A string of the value.
 */
std::string ZeldaNode::getSingleAttribute(std::string key){
    //Check if the item exists.
    if (singleAttributes.find(key) == singleAttributes.end()) return string();
    return singleAttributes[key];
}

int ZeldaNode::getCountAttribute(std::string key){
    //Check if the item exists.
    if (countAttributes.find(key) == countAttributes.end()) return 0;
    return countAttributes[key];
}

bool ZeldaNode::getBoolAttribute(std::string key){
    //Check if the item exists.
    if (boolAttributes.find(key) == boolAttributes.end()) return false;
    return boolAttributes[key];
}

/**
 * Gets a multi attribute by key.
 * @param key The key.
 * @return A vector of values.
 */
std::set<std::string> ZeldaNode::getMultiAttribute(std::string key){
    //Check if the item exists.
    if (multiAttributes.find(key) == multiAttributes.end()) return set<string>();
    return multiAttributes[key];
}

/**
 * Gets the number of attributes.
 * @return The number of attributes.
 */
int ZeldaNode::getNumAttributes(){
    return (int) (singleAttributes.size() + multiAttributes.size() + countAttributes.size());
}

/**
 * Sets the ID.
 * @param newID The new ID to add.
 */
void ZeldaNode::setID(std::string newID){
    ID = newID;
}

/**
 * Sets the name.
 * @param newName The new name to add.
 */
void ZeldaNode::setName(std::string newName){
    name = newName;

    //Add the new label.
    addSingleAttribute(LABEL_FLAG, name);
}

/**
 * Sets the node type.
 * @param newType The new type.
 */
void ZeldaNode::setType(NodeType newType){
    type = newType;
}

/**
 * Adds a single attribute.
 * @param key The key.
 * @param value The value.
 */
void ZeldaNode::addSingleAttribute(const std::string& key, std::string value){
    //Add the KV pair in.
    singleAttributes[key] = value;
}

void ZeldaNode::addCountAttribute(const std::string& key, int value){
    //Add the KV pair in.
    countAttributes[key] += value;
}

void ZeldaNode::addBoolAttribute(const std::string& key, bool value, bool cumulative, bool isAnd){
  // if it's not cummulative, or it's not stored, just store value
  if ( ! cumulative || boolAttributes.find(key) == boolAttributes.end() ){
    boolAttributes[key] = value;
  } else {
    if ( isAnd ){
      boolAttributes[key] &= value;
    } else { // isOr
      boolAttributes[key] |= value;
    }
  }
}

/**
 * Adds a multi attribute.
 * @param key The key.
 * @param value The value.
 */
void ZeldaNode::addMultiAttribute(const std::string& key, std::string value){
    //Get the vector for the key.
    set<string> kV = multiAttributes[key];
    if (std::find(kV.begin(), kV.end(), value) != kV.end()) return;

    //Add the pair to the list.
    multiAttributes[key].emplace(value);
}

static string shorten(const string& s){
  if ( s.length() < printLen ) return s;
  return s.substr(s.length() - printLen);
}

/**
 * Generates the TA node string.
 * @return The string TA representation.
 */
string ZeldaNode::generateTANode(){
    return INSTANCE_FLAG + " " + getMD5(ID) + " " + ZeldaNode::typeToString(type);
}

/**
 * Generates the TA attribute string.
 * @return The string TA representation.
 */
string ZeldaNode::generateTAAttribute(){
    string attributes = getMD5(ID) + " { ";
    //Starts by generating all the single attributes.
    for (auto &entry : singleAttributes){
        if ( !entry.second.empty() )
          attributes += entry.first + " = " + "\"" + entry.second + "\" ";
    }
    for (auto &entry : boolAttributes){
        ostringstream oss;
        oss << boolalpha << entry.second;
        attributes += entry.first + " = " + "\"" + oss.str() + "\" ";
    }
    for (auto &entry : multiAttributes){
        attributes += entry.first + " = ( ";
        for (auto &vecEntry : entry.second){
            attributes += vecEntry + " ";
        }
        attributes += ") ";
        ostringstream oss;
        oss << entry.second.size();
        attributes += entry.first + "Count = \"" + oss.str() + "\"";
    }
    for (auto &entry : countAttributes){
        int count = entry.second;
        ostringstream oss;
        oss << count;
        attributes += entry.first + " = " + "\"" + oss.str() + "\" ";
    }
    attributes += "}";

    return attributes;
}
