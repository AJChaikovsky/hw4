#ifndef AVLBST_H
#define AVLBST_H

#include <iostream>
#include <exception>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include "bst.h"

struct KeyError { };

/**
 * A special kind of node for an AVL tree, which adds the balance as a data member.
 * (You do not need to change anything here.)
 */
template <typename Key, typename Value>
class AVLNode : public Node<Key, Value>
{
public:
    AVLNode(const Key& key, const Value& value, AVLNode<Key, Value>* parent);
    virtual ~AVLNode();

    int8_t getBalance() const;
    void setBalance(int8_t balance);
    void updateBalance(int8_t diff);

    // Getters for parent, left, and right (return AVLNode pointers)
    virtual AVLNode<Key, Value>* getParent() const override;
    virtual AVLNode<Key, Value>* getLeft() const override;
    virtual AVLNode<Key, Value>* getRight() const override;

    // For height-checking in tests. We assume the user may add a getHeight() method.
    // (Alternatively, getHeight() can be implemented in the tree.)
    int getHeight() const { return height_; }
    void setHeight(int h) { height_ = h; }

protected:
    int8_t balance_;    // Balance factor = (right subtree height - left subtree height)
    int height_ = 0;    // Height of the node (number of edges on longest path to a leaf)
};

/* ----------------- AVLNode implementations ----------------- */

template<class Key, class Value>
AVLNode<Key, Value>::AVLNode(const Key& key, const Value& value, AVLNode<Key, Value>* parent)
    : Node<Key, Value>(key, value, parent), balance_(0), height_(0)
{
}

template<class Key, class Value>
AVLNode<Key, Value>::~AVLNode() { }

template<class Key, class Value>
int8_t AVLNode<Key, Value>::getBalance() const
{
    return balance_;
}

template<class Key, class Value>
void AVLNode<Key, Value>::setBalance(int8_t balance)
{
    balance_ = balance;
}

template<class Key, class Value>
void AVLNode<Key, Value>::updateBalance(int8_t diff)
{
    balance_ += diff;
}

template<class Key, class Value>
AVLNode<Key, Value>* AVLNode<Key, Value>::getParent() const
{
    return static_cast<AVLNode<Key, Value>*>(this->parent_);
}

template<class Key, class Value>
AVLNode<Key, Value>* AVLNode<Key, Value>::getLeft() const
{
    return static_cast<AVLNode<Key, Value>*>(this->left_);
}

template<class Key, class Value>
AVLNode<Key, Value>* AVLNode<Key, Value>::getRight() const
{
    return static_cast<AVLNode<Key, Value>*>(this->right_);
}

/* ----------------- End AVLNode implementations ----------------- */

template <class Key, class Value>
class AVLTree : public BinarySearchTree<Key, Value>
{
public:
    virtual void insert(const std::pair<const Key, Value> &new_item);
    virtual void remove(const Key& key);
    
    // (Assumes clear() is inherited from BST.)
    
protected:
    virtual void nodeSwap(AVLNode<Key,Value>* n1, AVLNode<Key,Value>* n2);

    // Rotations
    AVLNode<Key, Value>* rotateLeft(AVLNode<Key, Value>* x);
    AVLNode<Key, Value>* rotateRight(AVLNode<Key, Value>* x);
    
    // Rebalancing logic
    AVLNode<Key, Value>* rebalance(AVLNode<Key, Value>* node);

    // Search helper (remove override because BST doesn't mark it virtual)
    Node<Key, Value>* internalFind(const Key& key) const;
    
    // Predecessor (for deletion)
    static AVLNode<Key, Value>* predecessor(Node<Key, Value>* current);
    
    // Height & Balance updating functions:
    void updateHeight(AVLNode<Key, Value>* node);
    int getHeight(AVLNode<Key, Value>* node);
    // updateBalance is used during insertion to propagate changes upward.
    void updateBalance(AVLNode<Key, Value>* node);
};

/* ----------------- AVLTree implementations ----------------- */

// Insert: standard BST insertion then update balance upward.
template<class Key, class Value>
void AVLTree<Key, Value>::insert (const std::pair<const Key, Value> &new_item)
{
    if(this->root_ == nullptr){
        this->root_ = new AVLNode<Key, Value>(new_item.first, new_item.second, nullptr);
        return;
    }

    Node<Key, Value>* curr = this->root_;
    Node<Key, Value>* parent = nullptr;

    // Standard BST insertion.
    while(curr != nullptr){
        parent = curr;
        if(new_item.first < curr->getKey()){
            curr = curr->getLeft();
        } else if(new_item.first > curr->getKey()){
            curr = curr->getRight();
        } else {
            curr->setValue(new_item.second); // Key already exists: update value.
            return;
        }
    }

    AVLNode<Key, Value>* newNode = new AVLNode<Key, Value>(new_item.first, new_item.second, static_cast<AVLNode<Key, Value>*>(parent));
    if(new_item.first < parent->getKey()){
        parent->setLeft(newNode);
    } else {
        parent->setRight(newNode);
    }

    // Propagate height and balance updates upward.
    AVLNode<Key, Value>* node = newNode->getParent();
    while(node != nullptr){
        updateHeight(node);
        int balance = node->getBalance();
        if(balance < -1 || balance > 1){
            // Attach the rebalanced subtree back.
            AVLNode<Key, Value>* newRoot = rebalance(node);
            // If node was root, update tree root.
            if(newRoot->getParent() == nullptr)
                this->root_ = newRoot;
        }
        // Continue upward.
        node = node->getParent();
    }
}

// updateBalance: propagates a change (if needed) upward.
// (This version is optional if updateHeight is called.)
template<class Key, class Value>
void AVLTree<Key, Value>::updateBalance(AVLNode<Key, Value>* node)
{
    AVLNode<Key, Value>* parent = node->getParent();
    if(parent == nullptr) return;
    
    if(node == parent->getLeft()){
        parent->updateBalance(-1);
    } else {
        parent->updateBalance(1);
    }
}

// updateHeight: recalculates height and balance factor for the node.
template<class Key, class Value>
void AVLTree<Key, Value>::updateHeight(AVLNode<Key, Value>* node)
{
    if(node == nullptr) return;
    int lh = getHeight(node->getLeft());
    int rh = getHeight(node->getRight());
    node->setHeight(std::max(lh, rh) + 1);
    node->setBalance(rh - lh);
}

// getHeight: recursively returns the height of the node.
// By definition, an empty tree has height -1.
template<class Key, class Value>
int AVLTree<Key, Value>::getHeight(AVLNode<Key, Value>* node)
{
    if(node == nullptr) return -1;
    return node->getHeight();
}

// rebalance: performs AVL rotations when a node is unbalanced.
template<class Key, class Value>
AVLNode<Key, Value>* AVLTree<Key, Value>::rebalance(AVLNode<Key, Value>* node)
{
    if(node == nullptr) return nullptr;
    int balance = node->getBalance();
    
    // Left-heavy case.
    if(balance == -2) {
        AVLNode<Key, Value>* leftChild = node->getLeft();
        if(leftChild->getBalance() > 0){
            // Left-Right case: perform left rotation on left child.
            node->setLeft(rotateLeft(leftChild));
            node->getLeft()->setParent(node);
        }
        return rotateRight(node);
    }
    // Right-heavy case.
    if(balance == 2) {
        AVLNode<Key, Value>* rightChild = node->getRight();
        if(rightChild->getBalance() < 0){
            // Right-Left case: perform right rotation on right child.
            node->setRight(rotateRight(rightChild));
            node->getRight()->setParent(node);
        }
        return rotateLeft(node);
    }
    return node; // Already balanced.
}

// rotateLeft: rotates the subtree left around x.
template<class Key, class Value>
AVLNode<Key, Value>* AVLTree<Key, Value>::rotateLeft(AVLNode<Key, Value>* x)
{
    AVLNode<Key, Value>* y = x->getRight();
    x->setRight(y->getLeft());
    if(y->getLeft() != nullptr){
        y->getLeft()->setParent(x);
    }
    y->setParent(x->getParent());
    if(x->getParent() == nullptr){
        this->root_ = y;
    } else if (x == x->getParent()->getLeft()){
        x->getParent()->setLeft(y);
    } else {
        x->getParent()->setRight(y);
    }
    y->setLeft(x);
    x->setParent(y);
    
    updateHeight(x);
    updateHeight(y);
    return y;
}

// rotateRight: rotates the subtree right around y.
template<class Key, class Value>
AVLNode<Key, Value>* AVLTree<Key, Value>::rotateRight(AVLNode<Key, Value>* y)
{
    AVLNode<Key, Value>* x = y->getLeft();
    y->setLeft(x->getRight());
    if(x->getRight() != nullptr){
        x->getRight()->setParent(y);
    }
    x->setParent(y->getParent());
    if(y->getParent() == nullptr){
        this->root_ = x;
    } else if (y == y->getParent()->getRight()){
        y->getParent()->setRight(x);
    } else {
        y->getParent()->setLeft(x);
    }
    x->setRight(y);
    y->setParent(x);
    
    updateHeight(y);
    updateHeight(x);
    return x;
}

// internalFind: standard BST search (do not use override)
template<class Key, class Value>
Node<Key, Value>* AVLTree<Key, Value>::internalFind(const Key& key) const
{
    Node<Key, Value>* current = this->root_;
    while(current != nullptr){
        if(key == current->getKey()){
            return current;
        } else if(key < current->getKey()){
            current = current->getLeft();
        } else {
            current = current->getRight();
        }
    }
    return nullptr;
}

// predecessor: returns the in-order predecessor of the given node.
template<class Key, class Value>
AVLNode<Key, Value>* AVLTree<Key, Value>::predecessor(Node<Key, Value>* current)
{
    if(current == nullptr) return nullptr;
    AVLNode<Key, Value>* node = static_cast<AVLNode<Key, Value>*>(current);
    if(node->getLeft() != nullptr){
        node = node->getLeft();
        while(node->getRight() != nullptr){
            node = node->getRight();
        }
        return node;
    }
    AVLNode<Key, Value>* parent = node->getParent();
    while(parent != nullptr && node == parent->getLeft()){
        node = parent;
        parent = parent->getParent();
    }
    return parent;
}

// remove: deletes a node from the tree and rebalances upward.
template<class Key, class Value>
void AVLTree<Key, Value>::remove(const Key& key)
{
    AVLNode<Key, Value>* node = static_cast<AVLNode<Key, Value>*>(this->internalFind(key));
    if(node == nullptr) return;
    
    // If node has two children, swap with predecessor.
    if(node->getLeft() != nullptr && node->getRight() != nullptr){
        AVLNode<Key, Value>* pred = static_cast<AVLNode<Key, Value>*>(this->predecessor(node));
        this->nodeSwap(node, pred);
    }
    
    // Now node has at most one child.
    AVLNode<Key, Value>* parent = node->getParent();
    AVLNode<Key, Value>* child = nullptr;
    if(node->getLeft() != nullptr)
        child = node->getLeft();
    else if(node->getRight() != nullptr)
        child = node->getRight();
    
    if(child != nullptr)
        child->setParent(parent);
    
    if(parent == nullptr){
        this->root_ = child;
    } else {
        if(parent->getLeft() == node)
            parent->setLeft(child);
        else
            parent->setRight(child);
    }
    
    delete node;
    
    // Rebalance upward from parent.
    AVLNode<Key, Value>* current = parent;
    while(current != nullptr){
        updateHeight(current);
        AVLNode<Key, Value>* newRoot = rebalance(current);
        // If the new root's parent is null, it's the overall tree root.
        if(newRoot->getParent() == nullptr)
            this->root_ = newRoot;
        current = newRoot->getParent();
    }
}

// nodeSwap: swaps two nodes using the BST helper, then swaps balances.
template<class Key, class Value>
void AVLTree<Key, Value>::nodeSwap(AVLNode<Key,Value>* n1, AVLNode<Key,Value>* n2)
{
    BinarySearchTree<Key, Value>::nodeSwap(n1, n2);
    int8_t tempB = n1->getBalance();
    n1->setBalance(n2->getBalance());
    n2->setBalance(tempB);
}

#endif
