#ifndef LeafNodeH
#define LeafNodeH

#include "BTreeNode.h"

class LeafNode:public BTreeNode
{
  int *values;
public:
  LeafNode(int LSize, InternalNode *p, BTreeNode *left,
    BTreeNode *right);
  void addToLeft(int value, int last);
  void addToRight(int value, int last);
  void addToThis(int value);
  void addValue(int value, int &last);
  int getMaximum() const;
  int getMinimum() const;
  LeafNode* insert(int value); // returns pointer to new Leaf if splits
  // else NULL
  LeafNode* remove(int value); // returns pointer to removed Leaf if merge
  //NULL == no merge
  bool removeDriver(int value); //returns 1 if value not found
  void mergeRight();
  void mergeLeft();
  void borrowRight();
  void borrowLeft();
  void print(Queue <BTreeNode*> &queue);
  LeafNode* split(int value, int last);

}; //LeafNode class

#endif
