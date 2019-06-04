/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZeldaEdge.h
//
// Created By: Bryan J Muscedere
// Date: 10/04/17.
// Modified by Kirsten Bradley 2019 for Zelda
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

#ifndef REX_REXEDGE_H
#define REX_REXEDGE_H

class ZeldaNode;
#include <string>
#include <map>
#include <set>

class ZeldaEdge {
public:
    //Node Type Information
    enum EdgeType {CONTAINS, CALLS, HANDLE, INHERITS, CONTEXT,
        THROWS, FUNC_THROWS, RETHROWS, CATCHES, THROWPATH, VIRTUAL_CALL};
    static std::string typeToString(EdgeType type);

    //Constructor/Destructor
    ZeldaEdge(ZeldaNode* src, ZeldaNode* dst, EdgeType type);
    ZeldaEdge(std::string src, std::string dst, EdgeType type);
    ZeldaEdge(ZeldaNode* src, std::string dst, EdgeType type);
    ZeldaEdge(std::string src, ZeldaNode* dst, EdgeType type);
    ~ZeldaEdge();

    //Information Methods
    bool isEstablished();

    //Setters
    void setSource(ZeldaNode* src);
    void setDestination(ZeldaNode* dst);
    void setSourceID(std::string ID);
    void setDestinationID(std::string ID);
    void setSourceName(std::string name);
    void setDestName(std::string name);
    void setType(EdgeType type);

    //Getters
    ZeldaNode* getSource();
    ZeldaNode* getDestination();
    EdgeType getType();
    std::string getSourceID();
    std::string getDestinationID();
    std::string getSourceName();
    std::string getDestinationName();
    int getNumAttributes();

    //Attribute Manager
    void addSingleAttribute(std::string key, std::string value);
    void addMultiAttribute(std::string key, std::string value);
    std::string getSingleAttribute(std::string key);
    std::set<std::string> getMultiAttribute(std::string key);

    //TA Generator
    std::string generateTAEdge();
    std::string generateTAAttribute();

private:
    ZeldaNode* sourceNode;
    ZeldaNode* destNode;

    std::string sourceID;
    std::string destID;
    std::string sourceName;
    std::string destName;

    EdgeType type;

    //std::string getMD5(std::string ID);

    std::map<std::string, std::string> singleAttributes;
    std::map<std::string, std::set<std::string>> multiAttributes;
    bool generated = false;
};


#endif //REX_REXEDGE_H
