//
//  MyMap.h
//  CS 32 Project 4
//
//  Created by Ashwin Vivek on 3/8/17.
//  Copyright © 2017 AshwinVivek. All rights reserved.
//

#ifndef MyMap_h
#define MyMap_h

#include <queue>
#include <iostream>
#include "support.h"

using namespace std;

template<typename KeyType, typename ValueType>
class MyMap
{
public:
    MyMap();
    ~MyMap();
    void clear();
    int size() const;
    void associate(const KeyType& key, const ValueType& value);
    
    // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const;
    
    // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const MyMap*>(this)->find(key));
    }
    
    // C++11 syntax for preventing copying and assignment
    MyMap(const MyMap&) = delete;
    MyMap& operator=(const MyMap&) = delete;
    
private:
    
    struct Node
    {
        Node(KeyType k, ValueType v)
        {
            m_key = k;
            m_value = v;
            right = nullptr;
            left = nullptr;
        }
        KeyType m_key;
        ValueType m_value;
        Node* right;
        Node* left;
    };
    Node* root;
    
};

template<typename KeyType, typename ValueType>
MyMap<KeyType, ValueType>::MyMap()
{
    root = nullptr;
}
template<typename KeyType, typename ValueType>
MyMap<KeyType, ValueType>::~MyMap()
{
    clear();
}

template<typename KeyType, typename ValueType>
void MyMap<KeyType, ValueType>::clear()
{
    if(root == nullptr)
        return;
    
    queue<Node*> q;
    
    q.push(root);
    while(!q.empty())
    {
        Node* curr = q.front();
        q.pop();
        if(curr->left != nullptr)
            q.push(curr->left);
        if(curr->right != nullptr)
            q.push(curr->right);
        delete curr;
    }
    root = nullptr;
}

template<typename KeyType, typename ValueType>
int MyMap<KeyType, ValueType>::size() const
{
    if(root == nullptr)
        return 0;
    
    int count = 0;
    
    queue<Node*> q;
    q.push(root);
    while(!q.empty())
    {
        Node* curr = q.front();
        q.pop();
        count++;
        if(curr->left != nullptr)
            q.push(curr->left);
        if(curr->right != nullptr)
            q.push(curr->right);
    }
    return count;
}

template<typename KeyType, typename ValueType>
void MyMap<KeyType, ValueType>::associate(const KeyType &key, const ValueType &value)
{
    bool foundKey = false;
    
    if(root == nullptr)
    {
        root = new Node(key, value);
        return;
    }
    
    queue<Node*> q;
    
    q.push(root);
    while(!q.empty())
    {
        Node* curr = q.front();
        q.pop();
        
        if(curr != nullptr)
        {
            if(curr->m_key == key){
                curr->m_value = value;
                foundKey = true;
                break;
            }
            if(key < curr->m_key)
                q.push(curr->left);
            else if(key > curr->m_key)
                q.push(curr->right);
        }
    }
    
    if(!foundKey)
    {
        while(!q.empty())
            q.pop();
        
        q.push(root);
        while(!q.empty())
        {
            Node* traverser = q.front();
            q.pop();
            
            if(traverser!=nullptr)
            {
                if(key < traverser->m_key)
                {
                    if(traverser->left!=nullptr)
                        q.push(traverser->left);
                    else{
                        traverser->left = new Node(key, value);
                        return;
                    }
                }
                if(key > traverser->m_key)
                {
                    if(traverser->right!=nullptr)
                        q.push(traverser->right);
                    else{
                        traverser->right = new Node(key, value);
                        return;
                    }
                }
            }
        }
    }
}

template<typename KeyType, typename ValueType>
const ValueType* MyMap<KeyType, ValueType>::find(const KeyType &key) const
{
    Node* ptr = root;
    while(ptr != nullptr)
    {
        if(key == ptr->m_key)
            return &(ptr->m_value);
        else if (key < ptr->m_key)
            ptr = ptr->left;
        else if(key > ptr->m_key)
            ptr = ptr->right;
    }
    return nullptr;
}



#endif /* MyMap_h */
