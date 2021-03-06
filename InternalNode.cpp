#include <iostream>
#include <climits>
#include "InternalNode.h"

using namespace std;

InternalNode::InternalNode(int ISize, int LSize,
  InternalNode *p, BTreeNode *left, BTreeNode *right) :
  BTreeNode(LSize, p, left, right), internalSize(ISize)
{
  keys = new int[internalSize]; // keys[i] is the minimum of children[i]
  children = new BTreeNode* [ISize];
} // InternalNode::InternalNode()

BTreeNode* InternalNode::addPtr(BTreeNode *ptr, int pos)
{ // called when original was full, pos is where the node belongs.
  if(pos == internalSize)
    return ptr;

  BTreeNode *last = children[count - 1];

  for(int i = count - 2; i >= pos; i--)
  {
    children[i + 1] = children[i];
    keys[i + 1] = keys[i];
  } // shift things to right to make room for ptr, i can be -1!

  children[pos] = ptr;  // i will end up being the position that it is inserted
  keys[pos] = ptr->getMinimum();
  ptr->setParent(this);
  return last;
} // InternalNode:: addPtr()


void InternalNode::addToLeft(BTreeNode *last)
{
  ((InternalNode*)leftSibling)->insert(children[0]);

  for(int i = 0; i < count - 1; i++)
  {
    children[i] = children[i + 1];
    keys[i] = keys[i + 1];
  }

  children[count - 1] = last;
  keys[count - 1] = last->getMinimum();
  last->setParent(this);
  if(parent)
    parent->resetMinimum(this);
} // InternalNode::ToLeft()

void InternalNode::addToRight(BTreeNode *ptr, BTreeNode *last)
{
  ((InternalNode*) rightSibling)->insert(last);
  if(ptr == children[0] && parent)
    parent->resetMinimum(this);
} // InternalNode::addToRight()

void InternalNode::addToThis(BTreeNode *ptr, int pos)
{  // pos is where the ptr should go, guaranteed count < internalSize at start
  int i;

  for(i = count - 1; i >= pos; i--)
  {
      children[i + 1] = children[i];
      keys[i + 1] = keys[i];
  } // shift to the right to make room at pos

  children[pos] = ptr;
  keys[pos] = ptr->getMinimum();
  count++;
  ptr->setParent(this);

  if(pos == 0 && parent)
    parent->resetMinimum(this);
} // InternalNode::addToThis()


int InternalNode::getMaximum() const
{
  if(count > 0) // should always be the case
    return children[count - 1]->getMaximum();
  else
    return INT_MAX;
}  // getMaximum();


int InternalNode::getMinimum()const
{
  if(count > 0)   // should always be the case
    return children[0]->getMinimum();
  else
    return 0;
} // InternalNode::getMinimum()


InternalNode* InternalNode::insert(int value)
{  // count must always be >= 2 for an internal node
  int pos; // will be where value belongs

  for(pos = count - 1; pos > 0 && keys[pos] > value; pos--);

  BTreeNode *last, *ptr = children[pos]->insert(value);
  if(!ptr)  // child had room.
    return NULL;

  if(count < internalSize)
  {
    addToThis(ptr, pos + 1);
    return NULL;
  } // if room for value

  last = addPtr(ptr, pos + 1);

  if(leftSibling && leftSibling->getCount() < internalSize)
  {
    addToLeft(last);
    return NULL;
  }
  else // left sibling full or non-existent
    if(rightSibling && rightSibling->getCount() < internalSize)
    {
      addToRight(ptr, last);
      return NULL;
    }
    else // both siblings full or non-existent
      return split(last);
} // InternalNode::insert()

void InternalNode::insert(BTreeNode *oldRoot, BTreeNode *node2)
{ // Node must be the root, and node1
  children[0] = oldRoot;
  children[1] = node2;
  keys[0] = oldRoot->getMinimum();
  keys[1] = node2->getMinimum();
  count = 2;
  children[0]->setLeftSibling(NULL);
  children[0]->setRightSibling(children[1]);
  children[1]->setLeftSibling(children[0]);
  children[1]->setRightSibling(NULL);
  oldRoot->setParent(this);
  node2->setParent(this);
} // InternalNode::insert()

void InternalNode::insert(BTreeNode *newNode)
{ // called by sibling so either at beginning or end
  int pos;

  if(newNode->getMinimum() <= keys[0]) // from left sibling
    pos = 0;
  else // from right sibling
    pos = count;

  addToThis(newNode, pos);
} // InternalNode::insert(BTreeNode *newNode)

void InternalNode::print(Queue <BTreeNode*> &queue)
{
  int i;

  cout << "Internal: ";
  for (i = 0; i < count; i++)
    cout << keys[i] << ' ';
  cout << endl;

  for(i = 0; i < count; i++)
    queue.enqueue(children[i]);

} // InternalNode::print()


BTreeNode* InternalNode::remove(int value)
{  // to be written by students
  int pos = getPosition(value);

  BTreeNode* killed = children[pos]->remove(value);

  if(killed != NULL) //if != NULL, we had a leafnode merge
  {
    removeDriver(pos); //clean up the keys and children
    delete killed;

    if(parent)
      parent->resetMinimum(this);
  }

  InternalNode* leftSib = (InternalNode*) getLeftSibling();
  InternalNode* rightSib = (InternalNode*) getRightSibling();
  if(count < internalSize/2 + internalSize%2)
    {
      if(leftSib != NULL && parent) //go left
	{
	  if(leftSib->count <= internalSize/2 + internalSize % 2) //can't borrow, so merge!
	    {
	      mergeLeft();
	      return this;
	    }
	  else //borrow
	    {
	      borrowLeft();
	      return NULL;
	    }
	}
      else if(rightSib != NULL && parent) //go right
	{
	  if(rightSib->count <= internalSize/2 + internalSize % 2) //can't borrow, so merge!
	    {
	      mergeRight();
	      return this;
	      }
	  else //borrow
	    {
	      borrowRight();
	      return NULL;
	    }
	}
    }

  // if the function gets to here without returning, this condition should be enough
  if (children[1] == NULL) // only child, return new root (??)
  {
    children[0]->setParent(NULL);
    return children[0];
  } // if

  return NULL; // filler for stub
} // InternalNode::remove()

int InternalNode::getPosition(int value)
{
  int i;
  for(i = 0; (value >= keys[i]) && (i < count); i++){/*empty for loop*/}
  return (i-1);
} //InternalNode::getPosition()

void InternalNode::removeDriver(int i)
{
  int j;
  for(j=i; j < count-1; j++) //write over old value (shift down)
    {
      keys[j] = keys[j+1];
      children[j] = children[j+1];
    }
  keys[j] = NULL;
  children[j] = NULL;
  count--;

  if(i==0 && parent)
    parent->resetMinimum(this);
} //InternalNode::removeDriver()

void InternalNode::mergeLeft()
{
  InternalNode * leftSib = (InternalNode *) getLeftSibling();
  while(count != 0)
    {
      leftSib->insert(children[count-1]); //minimize shifting
      removeDriver(count-1); //delete shifted value
    }
  InternalNode * rightSib = (InternalNode *) getRightSibling();
  leftSib->rightSibling = rightSib;
  if(rightSib != NULL)
    rightSib->leftSibling = leftSib;
  if(parent)
    parent->resetMinimum(this);
} //InternalNode::mergeLeft()

void InternalNode::mergeRight()
{
 
  InternalNode * rightSib = (InternalNode *) getRightSibling();
  while(count != 0)
    {
      rightSib->insert(children[count-1]); //minimize shifting
      removeDriver(count-1); //delete shifted value
    }
  InternalNode * leftSib = (InternalNode *) getLeftSibling();
  rightSib->leftSibling = leftSib;
  if(leftSib != NULL)
    leftSib->rightSibling = rightSib;
  if(parent)
    parent->resetMinimum(rightSib);
} //InternalNode::mergeRight()

void InternalNode::borrowLeft()
{
  InternalNode * leftSib = (InternalNode *) getLeftSibling();
  int max = leftSib->getMaximum();
  insert(leftSib->children[count-1]); //add max value from sibling to front of borrower
  leftSib->removeDriver(getPosition(max)); //remove the max value from sibling
  if(parent)
    parent->resetMinimum(this);
} //InternalNode::borrowLeft()

void InternalNode::borrowRight()
{
  InternalNode * rightSib = (InternalNode *) getRightSibling();
  insert(rightSib->children[0]); //add min value from sibling to borrower
  rightSib->removeDriver(0); //remove min value from sibling
  if(parent)
    parent->resetMinimum(rightSib);
} //InternalNode::borrowRight()

void InternalNode::resetMinimum(const BTreeNode* child)
{
  for(int i = 0; i < count; i++)
    if(children[i] == child)
    {
      keys[i] = children[i]->getMinimum();
      if(i == 0 && parent)
        parent->resetMinimum(this);
      break;
    }
} // InternalNode::resetMinimum()


InternalNode* InternalNode::split(BTreeNode *last)
{
  InternalNode *newptr = new InternalNode(internalSize, leafSize,
    parent, this, rightSibling);


  if(rightSibling)
    rightSibling->setLeftSibling(newptr);

  rightSibling = newptr;

  for(int i = (internalSize + 1) / 2; i < internalSize; i++)
  {
    newptr->children[newptr->count] = children[i];
    newptr->keys[newptr->count++] = keys[i];
    children[i]->setParent(newptr);
  }

  newptr->children[newptr->count] = last;
  newptr->keys[newptr->count++] = last->getMinimum();
  last->setParent(newptr);
  count = (internalSize + 1) / 2;
  return newptr;
} // split()

