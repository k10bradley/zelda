/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TAGraph.cpp
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

#include <iostream>
#include "MD5.h"
#include <cstring>
#include <assert.h>
#include "../Walker/ZeldaWalker.h"
#include "TAGraph.h"
#include "ZeldaNode.h"
#include "ZeldaEdge.h"

using namespace std;

int const MD5_LENGTH = 33;

static int const printLength = 200;


/**
 * Constructor. Sets up all the member variables.
 * @param edgeType The edge type that forms the forest.
 */
TAGraph::TAGraph(){
    idList = std::unordered_map<std::string, ZeldaNode*>();
    edgeSrcList = std::unordered_map<std::string, std::vector<ZeldaEdge*>>();
    edgeDstList = std::unordered_map<std::string, std::vector<ZeldaEdge*>>();
}

/**
 * Destructor. Deletes all nodes and edges.
 */
TAGraph::~TAGraph(){
    clearGraph();
}

/**
 * Clears the graph from disk.
 */
void TAGraph::clearGraph(){
    for (auto &entry : idList){
        delete entry.second;
    }
    idList.clear();
    for (auto &entry : edgeSrcList){
        for (auto &vecEntry : entry.second){
//            delete vecEntry;
        }
    }
    edgeSrcList.clear();
    edgeDstList.clear();
}

void TAGraph::emptyGraph(){
    idList.clear();
    edgeSrcList.clear();
    edgeDstList.clear();
}

void TAGraph::merge(TAGraph* other){
  for ( auto elem : other->idList ){
    string ID = elem.first;
    ZeldaNode* node = elem.second;
    ZeldaNode* exists = findNode(ID);
    // fix edges in this graph which contain node in other
    if ( exists ){
      // node exists and must be added
    } else {
      // node does not exist
      // all edges in this graph are for a fake node
      addNode(node);
      vector<ZeldaEdge*> source = findEdgesBySrc(ID);
      for ( auto edge: source ){
        edge->setSource(node);
      }
      vector<ZeldaEdge*> dest = findEdgesByDst(ID);
      for ( auto edge: dest ){
        edge->setDestination(node);
      }
    }
    // add edges to this graph which have node as an end -> fix ends if possible
    vector<ZeldaEdge*> source = other->findEdgesBySrc(ID);
    for ( auto edge: source ){
      ZeldaNode* end = findNode(edge->getDestinationID());
      if ( end ) edge->setDestination(end);
      if ( exists ) edge->setSource(exists);
      addEdge(edge);
    }
    vector<ZeldaEdge*> dest = other->findEdgesByDst(ID);
    for ( auto edge: dest ){
      ZeldaNode* begin = findNode(edge->getSourceID());
      if ( begin ) edge->setSource(begin);
      if ( exists ) edge->setDestination(exists);
      addEdge(edge);
    }
    //if ( exists && node != exists ) delete node;
  }
  other->emptyGraph();
}

/**
 * Checks if the graph is empty.
 * @return Whether the graph is empty.
 */
bool TAGraph::isEmpty(){
    if (idList.size() == 0) return true;
    return false;
}

/**
 * Adds a new node.
 * @param node The node to add.
 */
void TAGraph::addNode(ZeldaNode* node){
    //Convert the node to a hash version.
    string newID = node->getID();
    node->setID(newID);

    //Add the node.
    if (idList.find(node->getID()) != idList.end())
        assert("Entry already exists.");
    idList[node->getID()] = node;
}

/**
 * Adds a new edge.
 * @param edge  The edge to add.
 */
void TAGraph::addEdge(ZeldaEdge* edge){
    //Convert the edge to a hash version.
    edge->setSourceID(edge->getSourceID());
    edge->setDestinationID(edge->getDestinationID());

    edgeSrcList[edge->getSourceID()].push_back(edge);
    edgeDstList[edge->getDestinationID()].push_back(edge);
}


/**
 * Removes a node.
 * @param nodeID The ID of the node to remove.
 */
void TAGraph::removeNode(std::string nodeID){
    ZeldaNode* node = idList[nodeID];

    //Erase the node.
    idList.erase(nodeID);
    delete node;

    //Erase all pertinent edges.
    vector<ZeldaEdge*> srcs = findEdgesBySrc(nodeID);
    vector<ZeldaEdge*> dsts = findEdgesByDst(nodeID);
    for (auto &item : srcs){
        removeEdge(item->getSourceID(), item->getDestinationID(), item->getType(), true);
    }
    for (auto &item : dsts){
        removeEdge(item->getSourceID(), item->getDestinationID(), item->getType(), true);
    }
}

/**
 * Removes an edge.
 * @param srcID The source node ID
 * @param dstID The destination node ID
 * @param type The node type.
 * @param hashed Whether the IDs we're searching for are already hashed.
 */
void TAGraph::removeEdge(std::string srcID, std::string dstID, ZeldaEdge::EdgeType type, bool hashed){
    if (!hashed){
        srcID = srcID;
        dstID = dstID;
    }

    ZeldaEdge* edgeToRemove = nullptr;

    //Starts by finding the node in the source list.
    vector<ZeldaEdge*> srcEdges = edgeSrcList[srcID];
    for (int i = 0; srcEdges.size(); i++){
        ZeldaEdge* edge = srcEdges.at(i);
        if (edge->getSourceID().compare(srcID) == 0 && edge->getDestinationID().compare(dstID) == 0 &&
                edge->getType() == type){
            edgeToRemove = edge;
            edgeSrcList[srcID].erase(edgeSrcList[srcID].begin() + i);
            break;
        }
    }

    //Removes the node in the destination list.
    vector<ZeldaEdge*> dstEdges = edgeDstList[dstID];
    for (int i = 0; dstEdges.size(); i++){
        ZeldaEdge* edge = dstEdges.at(i);
        if (edge->getSourceID().compare(srcID) == 0 && edge->getDestinationID().compare(dstID) == 0 &&
            edge->getType() == type){
            edgeDstList[dstID].erase(edgeDstList[dstID].begin() + i);
            break;
        }
    }

    //Removes the edge.
    if (edgeToRemove) delete edgeToRemove;
}

/**
 * Finds a node by ID
 * @param nodeID The ID to check.
 * @return The pointer to the node.
 */
ZeldaNode* TAGraph::findNode(std::string nodeID){
    //Check to see if the node exists.
    if (!doesNodeExist(nodeID)) return nullptr;
    return idList[nodeID];
}

/**
 * Finds node by name
 * @param nodeName The name of the node.
 * @param MD5Check Whether we want to hash by MD5
 * @return The pointer to the node.
 */
ZeldaNode* TAGraph::findNodeByName(string nodeName, bool MD5Check) {
    //Loop through the map.
    for (auto entry : idList){
        if (!entry.second) continue;

        string entryName = entry.second->getName();
        if (entryName.compare(nodeName) == 0)
            return entry.second;
    }

    return nullptr;
}

/**
 * Finds a node by the end of its name.
 * @param endName The end name string to search for.
 * @param MD5Check Whether we want to hash by MD5
 * @return The pointer to the node.
 */
ZeldaNode* TAGraph::findNodeByEndName(string endName, bool MD5Check) {
    for (auto entry: idList){
        if (!entry.second) continue;

        string entryName = entry.second->getName();
        if (hasEnding(entryName, endName)) {
            return entry.second;
        }
    }

    return nullptr;
}

/**
 * Finds nodes by a particular type.
 * @param type The type to search for.
 * @return A collection of nodes of that type.
 */
vector<ZeldaNode*> TAGraph::findNodesByType(ZeldaNode::NodeType type){
    vector<ZeldaNode*> nodes;

    for (auto entry : idList){
        if (entry.second->getType() == type) nodes.push_back(entry.second);
    }

    return nodes;
}

/**
 * Find edge bashed on its IDs.
 * @param srcID The source node ID.
 * @param dstID The destination node ID.
 * @param type The node type.
 * @return The edge that was found.
 */
ZeldaEdge* TAGraph::findEdge(std::string srcID, std::string dstID, ZeldaEdge::EdgeType type){
    //Gets the edges for a source.
    vector<ZeldaEdge*> edges = edgeSrcList[srcID];
    for (auto &edge : edges){
        if (edge->getSourceID().compare(srcID) == 0 && edge->getDestinationID().compare(dstID) == 0 &&
                edge->getType() == type){
            return edge;
        }
    }

    return nullptr;
}

/**
 * Finds edge by source ID and edge type.
 * @param src ZeldaNode used as source of edge
 * @param md5 type The type of edges being searched for
 * @return A vector of matched nodes.
 */
vector<ZeldaEdge*> TAGraph::findEdgesByTypeAndSrc(ZeldaNode* src, ZeldaEdge::EdgeType type){
  vector<ZeldaEdge*> edges = edgeSrcList[src->getID()];
  vector<ZeldaEdge*> matching;
  for( auto edge: edges ){
    if ( edge->getType() == type ){
      matching.emplace_back(edge);
    }
  }
  return matching;
}

std::vector<ZeldaEdge*> TAGraph::findEdgesByTypeAndDst(ZeldaNode* dst, ZeldaEdge::EdgeType type){
  vector<ZeldaEdge*> edges = findEdgesByDst(dst->getID());
  vector<ZeldaEdge*> matching;
  for( auto edge: edges ){
    if ( edge->getType() == type ){
      matching.emplace_back(edge);
    }
  }
  return matching;
}

/**
 * Finds edge by source ID.
 * @param srcID The source ID.
 * @param md5 Whether we want to hash the ID.
 * @return A vector of matched nodes.
 */
std::vector<ZeldaEdge*> TAGraph::findEdgesBySrc(std::string srcID, bool md5){
    if (md5) return edgeSrcList[srcID];
    return edgeSrcList[srcID];
}

/**
 * Finds edge by destination ID.
 * @param srcID The destination ID.
 * @param md5 Whether we want to hash the ID.
 * @return A vector of matched nodes.
 */
std::vector<ZeldaEdge*> TAGraph::findEdgesByDst(std::string dstID, bool md5){
    if (md5) return edgeDstList[dstID];
    return edgeDstList[dstID];
}

/**
 * Checks whether a node exists.
 * @param nodeID The ID of the node.
 * @return Whether the node exists.
 */
bool TAGraph::doesNodeExist(std::string nodeID){
    if (idList.find(nodeID) == idList.end()) return false;
    return true;
}

/**
 * Checks whether an edge exists.
 * @param srcID The source ID of the node.
 * @param dstID The destination ID of the node.
 * @type type The edge type.
 * @return Whether the edge exists.
 */
bool TAGraph::doesEdgeExist(std::string srcID, std::string dstID, ZeldaEdge::EdgeType type){
    //Gets the edges for a source.
    vector<ZeldaEdge*> edges = edgeSrcList[srcID];
    for (auto &edge : edges){
        if (edge->getSourceID().compare(srcID) == 0 && edge->getDestinationID().compare(dstID) == 0 &&
            edge->getType() == type){
            return true;
        }
    }

    return false;
}

/**
 * Resloves components based on a map of databases to files.
 * @param databaseMap The map of files to compile commands databases.
 * @return Whether the operation was successful.
 */
bool TAGraph::resolveComponents(map<string, vector<string>> databaseMap){
    //Go through the list of nodes.
    for (auto entry : idList){
        ZeldaNode* curNode = entry.second;
        if (curNode->getType() != ZeldaNode::CLASS) {
            //Check if the node has a contains relationship already.
            auto edges = findEdgesByDst(curNode->getID());
            bool hasContains = false;

            //Get the relationships.
            for ( ZeldaEdge* edge : edges){
                if (edge->getType() == ZeldaEdge::CONTAINS) {
                    hasContains = true;
                    break;
                }
            }
            if (hasContains) continue;
        }
    }
    return true;
}

/**
 * Resolves edges that have not been established.
 */
void TAGraph::resolveUnestablishedEdges(){
    //We go through our edges.
    for(auto it = edgeSrcList.begin(); it != edgeSrcList.end(); it++) {
        vector<ZeldaEdge*> edges = it->second;

        //Go through the list of subedges.
        for (int i = 0; i < edges.size(); i++){
            ZeldaEdge* curEdge = edges.at(i);

            //Check if the edge is established.
            if (!curEdge->isEstablished()){
                bool res = resolveEdge(curEdge);

                if (!res && curEdge->getType() == ZeldaEdge::CALLS && curEdge->getDestination() == nullptr){
                    resolveEdgeByName(curEdge);
                }
            }
        }
    }
}

/**
 * Removes unestablished edges.
 * @param resolveFirst Whether we resolve them first.
 */
void TAGraph::purgeUnestablishedEdges(bool resolveFirst){
    //We iterate through our edges.
    for(auto it = edgeSrcList.begin(); it != edgeSrcList.end(); it++) {
        vector<ZeldaEdge*> edges = it->second;

        //Go through the list of subedges.
        for (int i = 0; i < edges.size(); i++){
            ZeldaEdge* curEdge = edges.at(i);

            //Check if the edge is established
            if (!curEdge->isEstablished()){
                bool remove = true;
                if (resolveFirst) {
                    remove = !resolveEdge(curEdge);
                    if (remove && curEdge->getType() == ZeldaEdge::CALLS && curEdge->getDestination() == nullptr){
                        remove = !resolveEdgeByName(curEdge);
                    }
                }
                if (remove) removeEdge(curEdge->getSourceID(), curEdge->getDestinationID(), curEdge->getType(), true);
            }
        }
    }
}

/**
 * Only keep features of a particular name. Remove all descendants.
 * @param features The features to keep.
 * @return Boolean indicating success.
 */
/*
bool TAGraph::keepFeatures(vector<string> features){
    //Find features by name.
    vector<ZeldaNode*> featureNodes = findNodesByType(ZeldaNode::COMPONENT);
    for (ZeldaNode* curNode : featureNodes){
        if (find(features.begin(), features.end(), curNode->getName()) == features.end()){
            hierarchyRemove(curNode);
        }
    }
}
*/

/**
 * Generates the TA model of the graph.
 * @return The string representation of the model.
 */
bool TAGraph::getTAModel(const string& filename){
    if ( ! out.is_open() ) out.open(filename);
    out << "// Zelda Exception Extraction \n//Author: Kirsten Bradley \n";
    
    out << "SCHEME TUPLE :\n\n";
    
    out << "SCHEME ATTRIBUTE :\n\n";

    out << "FACT TUPLE :\n";
    generateInstances();
    generateRelations();
    out << "\nFACT ATTRIBUTE :\n";
    generateAttributes();

    return true;
}

/**
 * Generates instances of the TA graph.
 * @return The instances in string form.
 */
bool TAGraph::generateInstances(){
    //Writes the nodes.
    string model = "";
    for (auto &entry : idList){
        if (entry.second == nullptr) continue;
        out << entry.second->generateTANode() << "\n";
    }

    return true;
}

/**
 * Generates relations of the TA graph.
 */
bool TAGraph::generateRelations(){
    string model = "";

    //Writes the edges.
    for (auto &entry : edgeSrcList){
        for (auto &vecEntry : entry.second){
            out << vecEntry->generateTAEdge() << "\n";
        }
    }

    return true;
}

/**
 * Generates attributes of the TA graph.
 */
bool TAGraph::generateAttributes(){
    //Writes the attributes.
    for (auto &entry : idList){
        if (entry.second == nullptr) continue;
        if (entry.second->getNumAttributes() == 0) continue;
        out << entry.second->generateTAAttribute() << "\n";
    }
    for (auto &entry : edgeSrcList){
        for (auto &vecEntry : entry.second){
            if (vecEntry->getNumAttributes() == 0) continue;
            out << vecEntry->generateTAAttribute() << "\n";
        }
    }

    return true;
}


/**
 * Resolves an edge. If the nodes exist, the edge will be properly resolved.
 * @param edge The edge to resolve.
 * @return Whether it was resolved successfully.
 */
bool TAGraph::resolveEdge(ZeldaEdge *edge) {
    if (edge->isEstablished()) return true;

    //Look for the source and destination.
    if (edge->getSource() == nullptr){
        //Resolves the source ID.
        string sourceID = edge->getSourceID();
        ZeldaNode* srcNode = idList[sourceID];
        if (srcNode == nullptr) return false;

        edge->setSource(srcNode);
    }
    if (edge->getDestination() == nullptr){
        //Resolves the source ID.
        string destID = edge->getDestinationID();
        ZeldaNode* destNode = idList[destID];
        if (destNode == nullptr) return false;

        edge->setDestination(destNode);
    }

    return true;
}

/**
 * Resolves an edge based on the names of the nodes.
 * @param edge The edge to resolve.
 * @return Whether it was resolved successfully.
 */
bool TAGraph::resolveEdgeByName(ZeldaEdge* edge){
    if (edge->isEstablished()) return true;

    //Look for the source and destination.
    if (edge->getSource() == nullptr){
        //Resolves the source ID.
        string sourceName = edge->getSourceName();
        ZeldaNode* srcNode = findNodeByEndName(sourceName);
        if (srcNode == nullptr) return false;

        edge->setSource(srcNode);
    }
    if (edge->getDestination() == nullptr){
        //Resolves the source ID.
        string destName = edge->getDestinationName();
        ZeldaNode* destNode = findNodeByEndName(destName);
        if (destNode == nullptr) return false;

        edge->setDestination(destNode);
    }


    return true;
}

/**
 * Checks if a string has an ending.
 * @param fullString The string to check.
 * @param ending The ending to apply.
 * @return Whether that string has that ending.
 */
bool TAGraph::hasEnding(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}
