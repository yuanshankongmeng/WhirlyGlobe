/*
 *  QuadTree.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/11.
 *  Copyright 2012 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import "WhirlyVector.h"
#import <set>

@class WhirlyGlobeViewState;

namespace WhirlyKit
{
    
class SizeOnScreenCalculator;
    
/// Quad tree representation
class Quadtree
{
public:   
    Quadtree(Mbr mbr,int minLevel,int maxLevel,int maxNodes,float minImportance,SizeOnScreenCalculator *);
    ~Quadtree();

    /// Represents a single quad tree node
    class Identifier
    {
    public:
        Identifier() { }
        Identifier(int x,int y,int level) : x(x), y(y), level(level) { }
        
        // Comparison based on x,y,level.  Used for sorting
        bool operator < (const Identifier &that) const;
        
        int x,y,level;
    };

    /// Quad tree node with bounding box and projected size on the screen
    class NodeInfo
    {
    public:
        // Compare based on importance.  Used for sorting
        bool operator < (const NodeInfo &that) const;
        
        Identifier ident;
        Mbr mbr;
        float importance;
    };

    // Check if the given tile is already present
    bool isTileLoaded(Identifier ident);

    // Check if the quad tree will accept the given tile
    // This means either there's room or less important nodes loaded
    // It does not mean isn't already loaded.  Check that separately.
    bool willAcceptTile(NodeInfo nodeInfo);
    
    // Recalculate the importance of everything
    void reevaluateNodes();
    
    // Add the given tile, keeping track of what needed to be removed
    void addTile(NodeInfo nodeInfo,std::vector<Identifier> &tilesRemoved);
    
    // Remove the given tile
//    void removeTile(Identifier ident);

    // Given an identifier, fill out the node info
    // MBR, importance and such
    NodeInfo generateNode(Identifier ident);

    // Given the identifier of a parent, fill out the children
    void generateChildren(Identifier ident,std::vector<NodeInfo> &nodes);
    
    // Return the children from the given node
    // If the node isn't in the tree, return false
    bool childrenForNode(Identifier ident,std::vector<Identifier> &childIdents);
    
    // Check if the given node has a parent loaded
    // Return true if so, false if not.
    bool hasParent(Identifier ident,Identifier &parentIdent);
    
    // Dump out to the log for debugging
    void Print();
    
protected:
    class Node;

    // Sorter based on id
    typedef struct
    {
        bool operator() (const Node *a,const Node *b)
        {
            return a->nodeInfo.ident < b->nodeInfo.ident;
        }
    } NodeIdentSorter;

    // Sorter based on node importance
    typedef struct
    {
        bool operator() (const Node *a,const Node *b)
        {
            return a->nodeInfo.importance < b->nodeInfo.importance;
        }
    } NodeSizeSorter;

    typedef std::set<Node *,NodeIdentSorter> NodesByIdentType;
    typedef std::set<Node *,NodeSizeSorter> NodesBySizeType;

    /// Single quad tree node with pointer to parent and children
    class Node
    {
        friend class Quadtree;
    public:
        Node(Quadtree *tree);
        
        NodeInfo nodeInfo;
        
        void addChild(Quadtree *tree,Node *child);
        void removeChild(Quadtree *tree,Node *child);
        bool hasChildren();
        void Print();
        
    protected:
        NodesByIdentType::iterator identPos;
        NodesBySizeType::iterator sizePos;
        Node *parent;
        Node *children[4];
    };
        
    Node *getNode(Identifier ident);
    void removeNode(Node *);

    Mbr mbr;
    int minLevel,maxLevel;
    int maxNodes;
    float minImportance;
    SizeOnScreenCalculator *calc;
    
    // All nodes, sorted by ID
    NodesByIdentType nodesByIdent;
    // Child nodes, sorted by importance
    NodesBySizeType nodesBySize;
};
 
/// Calculate the max comparable size of a quad tree element on the screen
class SizeOnScreenCalculator
{
public:
    virtual ~SizeOnScreenCalculator() { }
    
    virtual float calcSize(Quadtree *quadtree,Quadtree::NodeInfo *node) = 0;
};

}
