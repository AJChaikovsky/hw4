#ifndef RECCHECK
//if you want to add any #includes like <iostream> you must do them here (before the next endif)

#endif

#include "equal-paths.h"
using namespace std;


// You may add any prototypes of helper functions here
bool checkPaths(Node* node, int currentDepth, int& leafDepth);

bool equalPaths(Node * root)
{
    // Add your code below
    int leafDepth = -1;
    return checkPaths(root, 0, leafDepth);


}

bool checkPaths(Node* node, int currentDepth, int& leafDepth){
    if(node == nullptr){
        return true; //base case
    }

    if(node->left == nullptr && node->right == nullptr){
        if(leafDepth == -1){ //leaf node
            leafDepth = currentDepth; //first leaf found
            return true;
        }else {
            return currentDepth == leafDepth; //compare to other leaf node
        }
    }
    return checkPaths(node->left, currentDepth + 1, leafDepth) 
    && checkPaths(node->right, currentDepth + 1, leafDepth); //recursively checks both subtrees
}