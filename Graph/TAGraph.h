/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TAGraph.h
//
// Created By: Bryan J Muscedere
// Date: 10/04/17.
// Updated by Kirsten Bradley 2019 for Zelda
//
// Master system for maintaining the in-memory
// digraph system. This is important to store
// C++ entities and then output to TA.
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

#ifndef ZELDA_TAGRAPH_H
#define ZELDA_TAGRAPH_H

#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include "ZeldaEdge.h"
#include "ZeldaNode.h"

class TAGraph {
public:
    //Constructor/Destructor
    TAGraph();
    virtual ~TAGraph();

    void clearGraph();

    bool isEmpty();

    //Node/Edge Adders
    virtual void addNode(ZeldaNode* node);
    virtual void addEdge(ZeldaEdge* edge);

    //Node/Edge Removers
    //void hierarchyRemove(ZeldaNode* toRemove);
    void removeNode(std::string nodeID);
    void removeEdge(std::string srcID, std::string dstID, ZeldaEdge::EdgeType type, bool hashed = false);

    //Find Methods
    ZeldaNode* findNode(std::string nodeID);
    ZeldaNode* findNodeByName(std::string nodeName, bool MD5Check = false);
    ZeldaNode* findNodeByEndName(std::string endName, bool MD5Check = false);
    std::vector<ZeldaNode*> findNodesByType(ZeldaNode::NodeType type);
    ZeldaEdge* findEdge(std::string srcID, std::string dstID, ZeldaEdge::EdgeType type);
    std::vector<ZeldaEdge*> findEdgesBySrc(std::string srcID, bool md5 = true);
    std::vector<ZeldaEdge*> findEdgesByDst(std::string dstID, bool md5 = true);
    std::vector<ZeldaEdge*> findEdgesByTypeAndSrc(ZeldaNode* node, ZeldaEdge::EdgeType type);
    std::vector<ZeldaEdge*> findEdgesByTypeAndDst(ZeldaNode* node, ZeldaEdge::EdgeType type);

    //Element Exist Methods
    bool doesNodeExist(std::string nodeID);
    bool doesEdgeExist(std::string srcID, std::string dstID, ZeldaEdge::EdgeType type);

    //Graph Clean Methods
    virtual bool resolveComponents(std::map<std::string, std::vector<std::string>> databaseMap);
    void resolveUnestablishedEdges();
    virtual void purgeUnestablishedEdges(bool resolveFirst = true);

    void merge(TAGraph* other);

    //TA Generators
    virtual bool getTAModel(const std::string&);

protected:
    bool minMode;

    //Member variables
    std::unordered_map<std::string, ZeldaNode*> idList;
    std::unordered_map<std::string, std::vector<ZeldaEdge*>> edgeSrcList;
    std::unordered_map<std::string, std::vector<ZeldaEdge*>> edgeDstList;

    bool generateInstances();
    bool generateRelations();
    bool generateAttributes();

    //Helper Function
    bool hasEnding(std::string const &fullString, std::string const &ending);
private:
    //Zelda Functionality
    std::string const FILENAME_ATTR = "filename";

    int const MD5_LENGTH = 33;


    //Edge Resolvers
    bool resolveEdge(ZeldaEdge* edge);
    bool resolveEdgeByName(ZeldaEdge* edge);

    void emptyGraph();
    std::ofstream out;
};


#endif //REX_TAGRAPH_H
