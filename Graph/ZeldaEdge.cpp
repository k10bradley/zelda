/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZeldaEdge.cpp
//
// Created By: Bryan J Muscedere
// Date: 10/04/17.
//
// Modified by Ten Bradley for Zelda
// Date: 2019
//
// Maintains an edge structure. Basic
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

#include <iostream>
#include "MD5.h"
#include <cstring>
#include "ZeldaEdge.h"
#include "ZeldaNode.h"

using namespace std;

static int const printLen = 200;


/**
 * Converts an edge type to a string representation.
 * @param type The edge type.
 * @return The string representation.
 */
string ZeldaEdge::typeToString(ZeldaEdge::EdgeType type){
    switch(type){
        case CONTAINS:
            return "contain";

        case CALLS:
            return "call";

        case HANDLE:
            return "handle";
      
        case INHERITS:
            return "inherits";
        
        case CONTEXT:
            return "context";

        case THROWS:
            return "throws";
        case FUNC_THROWS:
            return "funcThrows";

        case RETHROWS:
            return "rethrows";

        case CATCHES:
            return "catches";
        case THROWPATH:
            return "throwPath";
        case VIRTUAL_CALL: 
            return "virtualCall";
    }

    return "unknown";
}

/**
 * Creates an established edge based on two Zelda nodes.
 * @param src The pointer to the source.
 * @param dst The pointer to the destination.
 * @param type The edge type.
 */
ZeldaEdge::ZeldaEdge(ZeldaNode* src, ZeldaNode* dst, ZeldaEdge::EdgeType type){
    sourceNode = src;
    destNode = dst;
    sourceID = src->getID();
    destID = dst->getID();
    sourceName = src->getName();
    destName = dst->getName();
    this->type = type;
}

/**
 * Creates an unestablished edge based on two Zelda nodes.
 * @param src The string of the source ID.
 * @param dst The string of the destination ID.
 * @param type The edge type.
 */
ZeldaEdge::ZeldaEdge(string src, string dst, ZeldaEdge::EdgeType type){
    sourceNode = nullptr;
    destNode = nullptr;
    sourceID = src;
    destID = dst;
    sourceName = src;
    destName = dst;
    this->type = type;
}

/**
 * Creates an unestablished edge based on two Zelda nodes.
 * @param src The pointer to the source.
 * @param dst The string of the destination ID.
 * @param type The edge type.
 */
ZeldaEdge::ZeldaEdge(ZeldaNode* src, string dst, ZeldaEdge::EdgeType type){
    sourceNode = src;
    destNode = nullptr;
    sourceID = src->getID();
    destID = dst;
    sourceName = src->getName();
    destName = dst;
    this->type = type;
}

/**
 * Creates an unestablished edge based on two Zelda nodes.
 * @param src The string of the source ID.
 * @param dst The pointer to the destination.
 * @param type The edge type.
 */
ZeldaEdge::ZeldaEdge(string src, ZeldaNode* dst, ZeldaEdge::EdgeType type){
    sourceNode = nullptr;
    destNode = dst;
    sourceID = src;
    destID = dst->getID();
    sourceName = src;
    destName = dst->getName();
    this->type = type;
}

/**
 * Destructor
 */
ZeldaEdge::~ZeldaEdge(){ }

/**
 * Checks whether the edge is established.
 * @return Whether the edge is established.
 */
bool ZeldaEdge::isEstablished(){
    //Check to see if the nodes are in place.
    if (sourceNode && destNode) return true;
    return false;
}

/**
 * Sets the source node by pointer.
 * @param src The source node.
 */
void ZeldaEdge::setSource(ZeldaNode* src){
    sourceNode = src;
    if ( ! src ) return;
    sourceID = src->getID();
    sourceName = src->getName();
}

/**
 * Sets the destination node by pointer.
 * @param dst The destination node.
 */
void ZeldaEdge::setDestination(ZeldaNode* dst){
    destNode = dst;
    if ( ! dst ) return;
    destID = dst->getID();
    destName = dst->getName();
}

/**
 * Sets the ID of the source.
 * @param ID The ID of the source.
 */
void ZeldaEdge::setSourceID(string ID){
    if (!sourceNode){
        sourceID = ID;
    }
}

/**
 * Sets the name of the source.
 * @param name The new name.
 */
void ZeldaEdge::setSourceName(string name){
    if (!sourceNode){
        sourceName = name;
    }
}

/**
 * Sets the name of the destination.
 * @param name The new name.
 */
void ZeldaEdge::setDestName(string name){
    if (!destNode){
        destName = name;
    }
}

/**
 * Sets the ID of the destination.
 * @param ID The ID of the destination.
 */
void ZeldaEdge::setDestinationID(string ID){
    if (!destNode){
        destID = ID;
    }
}

/**
 * Sets the edge type.
 * @param type The new edge type.
 */
void ZeldaEdge::setType(ZeldaEdge::EdgeType type){
    this->type = type;
}

/**
 * Gets the source node.
 * @return The node of the source.
 */
ZeldaNode* ZeldaEdge::getSource(){
    return sourceNode;
}

/**
 * Gets the destination node.
 * @return The node of the destination.
 */
ZeldaNode* ZeldaEdge::getDestination(){
    return destNode;
}

/**
 * Gets the edge type.
 * @return The edge type.
 */
ZeldaEdge::EdgeType ZeldaEdge::getType(){
    return type;
}

/**
 * Gets the source ID.
 * @return The source ID.
 */
string ZeldaEdge::getSourceID(){
    if (sourceNode) {
        return sourceNode->getID();
    }
    return sourceID;
}

/**
 * Gets the destination ID.
 * @return The destination ID.
 */
string ZeldaEdge::getDestinationID(){
    if (destNode) {
        return destNode->getID();
    }
    return destID;
}

/**
 * Gets the name of the source.
 * @return The name.
 */
string ZeldaEdge::getSourceName(){
    if (sourceNode) {
        return sourceNode->getName();
    }
    return sourceName;
}

/**
 * Gets the name of the destination.
 * @return The name.
 */
string ZeldaEdge::getDestinationName(){
    if (destNode) {
        return destNode->getName();
    }
    return destName;
}

/**
 * Gets the number of attributes.
 * @return The number of attributes.
 */
int ZeldaEdge::getNumAttributes(){
return (int) (singleAttributes.size() + multiAttributes.size());
}

/**
 * Adds an attribute that only has one value.
 * @param key The key of the attribute.
 * @param value The value of the attribute.
 */
void ZeldaEdge::addSingleAttribute(std::string key, std::string value){
    singleAttributes[key] += value;
}

/**
 * Adds an attribute that has multiple values.
 * @param key The key of the attribute.
 * @param value The value of the attribute.
 */
void ZeldaEdge::addMultiAttribute(string key, string value){
    multiAttributes[key].emplace(value);
}

/**
 * Get a single attribute by key.
 * @param key The key of the attribute.
 * @return A string of the value.
 */
string ZeldaEdge::getSingleAttribute(string key){
    //Check if the attribute exists.
    if (singleAttributes.find(key) == singleAttributes.end()) return string();
    return singleAttributes[key];
}

/**
 * Get a multi attribute by key.
 * @param key The key of the attribute.
 * @return A string of the value.
 */
set<string> ZeldaEdge::getMultiAttribute(string key){
    //Check if the attribute exists.
    if (multiAttributes.find(key) == multiAttributes.end()) return {};
    return multiAttributes[key];
}

static string shorten(const string& s){
  if ( s.length() < printLen ) return s;
  return s.substr(s.length() - printLen);
}
/**
 * Generates an edge in TA format.
 * @return A string of the TA representation.
 */
string ZeldaEdge::generateTAEdge(){
    //if ( ! generated ) {
    //  generated = true;
      return ZeldaEdge::typeToString(type) + " " + getMD5(getSourceID()) + " " + getMD5(getDestinationID());
    //}
    return "";
}

/**
 * Generates all attributes in TA format.
 * @return A string of the TA representation.
 */
string ZeldaEdge::generateTAAttribute(){
    string attributes = "(" + generateTAEdge() + ") { ";

    //Starts by generating all the single attributes.
    for (auto &entry : singleAttributes){
        attributes += entry.first + " = " + "\"" + entry.second + "\" ";
    }
    for (auto &entry : multiAttributes){
        attributes += entry.first + " = ( ";
        for (auto &vecEntry : entry.second){
            attributes += vecEntry + " ";
        }
        attributes += ") ";
    }
    attributes += "}";

    return attributes;
}
