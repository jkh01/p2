#include <iostream>
#include <climits>
#include "LeafNode.h"
#include "InternalNode.h"
#include "QueueAr.h"

using namespace std;


LeafNode::LeafNode(int LSize, InternalNode *p,
  BTreeNode *left, BTreeNode *right) : BTreeNode(LSize, p, left, right)
{
  values = new int[LSize];
}  // LeafNode()

void LeafNode::addToLeft(int value, int last)
{
  leftSibling->insert(values[0]);

  for(int i = 0; i < count - 1; i++)
    values[i] = values[i + 1];

  values[count - 1] = last;
  if(parent)
    parent->resetMinimum(this);
} // LeafNode::ToLeft()

void LeafNode::addToRight(int value, int last)
{
  rightSibling->insert(last);

  if(value == values[0] && parent)
    parent->resetMinimum(this);
} // LeafNode::addToRight()

void LeafNode::addToThis(int value)
{
  int i;

  for(i = count - 1; i >= 0 && values[i] > value; i--)
      values[i + 1] = values[i];

  values[i + 1] = value;
  count++;

  if(value == values[0] && parent)
    parent->resetMinimum(this);
} // LeafNode::addToThis()


void LeafNode::addValue(int value, int &last)
{
  int i;

  if(value > values[count - 1])
    last = value;
  else
  {
    last = values[count - 1];

    for(i = count - 2; i >= 0 && values[i] > value; i--)
      values[i + 1] = values[i];
    // i may end up at -1
    values[i + 1] = value;
  }
} // LeafNode:: addValue()


int LeafNode::getMaximum()const
{
  if(count > 0)  // should always be the case
    return values[count - 1];
  else
    return INT_MAX;
} // getMaximum()


int LeafNode::getMinimum()const
{
  if(count > 0)  // should always be the case
    return values[0];
  else
    return 0;

} // LeafNode::getMinimum()


LeafNode* LeafNode::insert(int value)
{
  int last;

  if(count < leafSize)
  {
    addToThis(value);
    return NULL;
  } // if room for value

  addValue(value, last);

  if(leftSibling && leftSibling->getCount() < leafSize)
  {
    addToLeft(value, last);
    return NULL;
  }
  else // left sibling full or non-existent
    if(rightSibling && rightSibling->getCount() < leafSize)
    {
      addToRight(value, last);
      return NULL;
    }
    else // both siblings full or non-existent
      return split(value, last);
}  // LeafNode::insert()

void LeafNode::print(Queue <BTreeNode*> &queue)
{
  cout << "Leaf: ";
  for (int i = 0; i < count; i++)
    cout << values[i] << ' ';
  cout << endl;
} // LeafNode::print()

LeafNode* LeafNode::remove(int value)
{   // To be written by students
  
  if(removeDriver(value)) //if not found, return null
    return NULL;
  
  if(count < (leafSize/2 + leafSize%2) && parent != NULL) //we need to borrow or merge!
    {
      LeafNode * leftSib = (LeafNode *) getLeftSibling();
      if(leftSib != 0) //go left
	{
	  if(leftSib->getCount() <= leafSize/2 + leafSize%2) //leftSib doesn't have enough to donate, merge!
	    {
	      mergeLeft();
	      return this;
	    }
	  else //borrowing works
	    {
	      borrowLeft();
	    }
	}
      else if (parent != NULL)//go right
	{
	  LeafNode * rightSib = (LeafNode *) getRightSibling(); //we always have one sibling per Sean
	  if(rightSib->getCount() <= leafSize/2 + leafSize%2) //rightSib doesn't have enough either
	    {
	      mergeRight();
	      return this;
	    }
	  else //borrowing
	    {
	      borrowRight();
	    }
	}
    }  
  return NULL;  
} // LeafNode::remove()

bool LeafNode::removeDriver(int value)
{
  int min = getMinimum();
  int i; //removing
  for (i = 0; i < count && values[i] != value; i++);

  if (i == count) // value not in leaf
    return 1;

  int j;
  for (j = i; j < (count-1); j++) //shift values down
    values[j] = values[j+1];
  values[j+1] = NULL; //node #count-1  must be null after remove
  count--;

  if(value == min && parent) //if it has a parent and is the min
    parent->resetMinimum(this);

  return 0;
}

void LeafNode::mergeLeft()
{
  LeafNode * leftSib = (LeafNode *) getLeftSibling();
  while(count != 0)
    {
      int value = getMinimum(); //always add to end to save array shifting
      removeDriver(value);
      leftSib->addToThis(value);
    }

  /* potentially faster
  int sibCount = leftSib->getCount();
  int dif = sibCount - count; //difference in size
  for(int i = sibCount; i < leafSize; i++) //add 
    {
      leftSib->values[sibCount] = values[sibCount-dif]; //merge inorder, min to max
      leftSib->count++;
      values[sibCount-dif] = NULL; //delete old value
    }
  count = 0;
  */
}

void LeafNode::mergeRight()
{
  LeafNode * rightSib = (LeafNode *) getRightSibling();
  while(count != 0)
    {
      int value = getMinimum(); //so we move less
      removeDriver(value);
      rightSib->addToThis(value);
    }
  
  /* this code is potentially faster but also more perilous (maybe add after everything works for speed?)

    int sibCount = rightSib->getCount();
  for(int i = sibCount-1; i >= 0; i--) //shift values up so we can merge in front  
    {
      rightSib->values[i+count] = rightSib->values[i];
      rightSib->count++;
    }
  for(int i = 0; i < count; i++) //install in front min to max
    {
      rightSib->values[i] = values[i];
      values[i] = NULL;
    }
  count = 0;*/
}

void LeafNode::borrowLeft()
{
  LeafNode * leftSib = (LeafNode *) getLeftSibling();
  int borrow = leftSib->getMaximum(); //we want the max value from sibling
  leftSib->removeDriver(borrow); //remove it from leftSib, ours now!
  addToThis(borrow); //install the new value
}

void LeafNode::borrowRight()
{
  LeafNode * rightSib = (LeafNode *) getRightSibling();
  int borrow = rightSib->getMinimum(); //we want the min value from sibling
  rightSib->removeDriver(borrow); //remove it from rightSib, ours now!
  addToThis(borrow); //install the new value
}

LeafNode* LeafNode::split(int value, int last)
{
  LeafNode *ptr = new LeafNode(leafSize, parent, this, rightSibling);


  if(rightSibling)
    rightSibling->setLeftSibling(ptr);

  rightSibling = ptr;

  for(int i = (leafSize + 1) / 2; i < leafSize; i++)
    ptr->values[ptr->count++] = values[i];

  ptr->values[ptr->count++] = last;
  count = (leafSize + 1) / 2;

  if(value == values[0] && parent)
    parent->resetMinimum(this);
  return ptr;
} // LeafNode::split()

