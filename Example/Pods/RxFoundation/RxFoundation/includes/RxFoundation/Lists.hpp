//
//  Lists.hpp
//  RxFoundation
//
//  Created by closure on 11/30/16.
//  Copyright © 2016 closure. All rights reserved.
//

#ifndef List_h
#define List_h

namespace Rx {
    template <class ContainerType, class ElementType>
    class LinkedListIterator {
    public:
        explicit LinkedListIterator(ContainerType *FirstLink)
        : _currentLink(FirstLink)
        { }
        
        /**
          *Advances the iterator to the next element.
         */
        void next() {
            RxCheck(_currentLink);
            _currentLink = _currentLink->GetNextLink();
        }
        
        LinkedListIterator &operator++() {
            next();
            return *this;
        }
        
        LinkedListIterator operator++(int) {
            auto Tmp = *this;
            next();
            return Tmp;
        }
        
        /* *conversion to "bool" returning true if the iterator is valid. */
        RX_INLINE_VISIBILITY explicit operator bool() const {
            return _currentLink != nullptr;
        }
        
        // Accessors.
        ElementType &operator->() const {
            RxCheck(_currentLink);
            return **_currentLink;
        }
        
        ElementType &operator*() const {
            RxCheck(_currentLink);
            return **_currentLink;
        }
        
    private:
        
        ContainerType *_currentLink;
        
        friend bool operator==(const LinkedListIterator &Lhs, const LinkedListIterator &Rhs) { return Lhs._currentLink == Rhs._currentLink; }
        friend bool operator!=(const LinkedListIterator &Lhs, const LinkedListIterator &Rhs) { return Lhs._currentLink != Rhs._currentLink; }
    };
    
    
    /**
      *Encapsulates a link in a single linked list with constant access time.
     */
    template <class ElementType>
    class LinkedList {
    public:
        
        /**
          *Used to iterate over the elements of a linked list.
         */
        typedef LinkedListIterator<LinkedList,       ElementType> Iterator;
        typedef LinkedListIterator<LinkedList, const ElementType> ConstIterator;
        
        /* *Default constructor (empty list). */
        LinkedList()
        : _nextLink(nullptr)
        , _prevLink(nullptr)
        { }
        
        /**
          *Creates a new linked list with a single element.
         *
          *@param InElement The element to add to the list.
         */
        explicit LinkedList(const ElementType &InElement)
        : _element (InElement)
        , _nextLink(nullptr)
        , _prevLink(nullptr)
        { }
        
        /**
          *Removes this element from the list in constant time.
         *
          *This function is safe to call even if the element is not linked.
         */
        void Unlink() {
            if (_nextLink) {
                _nextLink->_prevLink = _prevLink;
            }
            if (_prevLink) {
                *_prevLink = _nextLink;
            }
            // Make it safe to call Unlink again.
            _nextLink = nullptr;
            _prevLink = nullptr;
        }
        
        /**
          *Adds this element to a list, before the given element.
         *
          *@param Before The link to insert this element before.
         */
        void Link(LinkedList *&Before) {
            if (Before) {
                Before->_prevLink = &_nextLink;
            }
            _nextLink = Before;
            _prevLink = &Before;
            Before = this;
        }
        
        /**
          *Returns whether element is currently linked.
         *
          *@return true if currently linked, false otherwise
         */
        bool IsLinked() {
            return _prevLink != nullptr;
        }
        
        LinkedList *GetPrevLink() const {
            return _prevLink;
        }
        
        LinkedList *GetNextLink() const {
            return _nextLink;
        }
        
        // Accessors.
        ElementType *operator->() {
            return &_element;
        }
        
        const ElementType *operator->() const {
            return &_element;
        }
        
        ElementType &operator*() {
            return _element;
        }
        
        const ElementType &operator*() const {
            return _element;
        }
        
        LinkedList *next() {
            return _nextLink;
        }
        
    private:
        ElementType  _element;
        LinkedList * _nextLink;
        LinkedList* *_prevLink;
        
        friend Iterator      begin(      LinkedList &List) { return Iterator     (&List); }
        friend ConstIterator begin(const LinkedList &List) { return ConstIterator(const_cast<LinkedList*>(&List)); }
        friend Iterator      end  (      LinkedList &List) { return Iterator     (nullptr); }
        friend ConstIterator end  (const LinkedList &List) { return ConstIterator(nullptr); }
    };
    
    
    template <class NodeType, class ElementType>
    class DoubleLinkedListIterator {
    public:
        
        explicit DoubleLinkedListIterator(NodeType *StartingNode)
        : _current(StartingNode)
        { }
        
        /* *conversion to "bool" returning true if the iterator is valid. */
        RX_INLINE_VISIBILITY explicit operator bool() const {
            return _current != nullptr;
        }
        
        DoubleLinkedListIterator &operator++() {
            RxCheck(_current);
            _current = _current->getNext();
            return *this;
        }
        
        DoubleLinkedListIterator operator++(int) {
            auto Tmp = *this;
            ++(*this);
            return Tmp;
        }
        
        DoubleLinkedListIterator &operator--() {
            RxCheck(_current);
            _current = _current->getPrev();
            return *this;
        }
        
        DoubleLinkedListIterator operator--(int) {
            auto Tmp = *this;
            --(*this);
            return Tmp;
        }
        
        // Accessors.
        ElementType &operator->() const {
            RxCheck(_current);
            return _current->getValue();
        }
        
        ElementType &operator*() const {
            RxCheck(_current);
            return _current->getValue();
        }
        
        NodeType *get() const {
            RxCheck(_current);
            return _current;
        }
        
    private:
        NodeType *_current;
        
        friend bool operator==(const DoubleLinkedListIterator &Lhs, const DoubleLinkedListIterator &Rhs) { return Lhs._current == Rhs._current; }
        friend bool operator!=(const DoubleLinkedListIterator &Lhs, const DoubleLinkedListIterator &Rhs) { return Lhs._current != Rhs._current; }
    };
    
    
    /**
      *Double linked list.
     */
    template <class ElementType>
    class DoubleLinkedList {
    public:
        class Node {
        public:
            friend class DoubleLinkedList;
            
            /* *Constructor */
            Node(const ElementType &InValue)
            : _value(InValue), _next(nullptr), _prev(nullptr)
            { }
            
            ~Node() {
                _next = nullptr;
                _prev = nullptr;
            }
            
            const ElementType &getValue() const {
                return _value;
            }
            
            ElementType &getValue() {
                return _value;
            }
            
            Node *getNext() {
                return _next;
            }
            
            Node *getPrev() {
                return _prev;
            }
            
        protected:
            ElementType _value;
            Node *_next;
            Node *_prev;
        };
        
        /**
          *Used to iterate over the elements of a linked list.
         */
        typedef DoubleLinkedListIterator<Node,       ElementType> Iterator;
        typedef DoubleLinkedListIterator<Node, const ElementType> ConstIterator;
        
        /* *Constructors. */
        DoubleLinkedList()
        : _headNode(nullptr)
        , _tailNode(nullptr)
        , _size(0)
        { }
        
        /* *Destructor */
        virtual ~DoubleLinkedList() {
            clear();
        }
        
        // Adding/Removing methods
        
        /**
          *Add the specified value to the beginning of the list, making that value the new head of the list.
         *
          *@param	InElement	the value to add to the list.
          *@return	whether the node was successfully added into the list.
          *@see getHead, insert, RemoveNode
         */
        bool push_front(const ElementType &InElement) {
            return push_front(new Node(InElement));
        }
        
        bool push_front(Node *NewNode) {
            if (NewNode == nullptr) {
                return false;
            }
            
            // have an existing head node - change the head node to point to this one
            if (_headNode != nullptr) {
                NewNode->_next = _headNode;
                _headNode->_prev = NewNode;
                _headNode = NewNode;
            } else {
                _headNode = _tailNode = NewNode;
            }
            
            setListSize(_size + 1);
            return true;
        }
        
        /**
          *Append the specified value to the end of the list
         *
          *@param	InElement	the value to add to the list.
          *@return	whether the node was successfully added into the list
          *@see getTail, insert, RemoveNode
         */
        Node *addObject(const ElementType &InElement) {
            Node *node = new Node(InElement);
            if (addObject(node)) {
                return node;
            }
            if (node) {
                delete node;
            }
            return nullptr;
        }
        
        bool addObject(Node *NewNode) {
            if (NewNode == nullptr) {
                return false;
            }
            
            if (_tailNode != nullptr) {
                _tailNode->_next = NewNode;
                NewNode->_prev = _tailNode;
                _tailNode = NewNode;
            } else {
                _headNode = _tailNode = NewNode;
            }
            
            setListSize(_size + 1);
            return true;
        }
        
        /**
          *Insert the specified value into the list at an arbitrary point.
         *
          *@param	InElement			the value to insert into the list
          *@param	NodeToInsertBefore	the new node will be inserted into the list at the current location of this node
         *								if nullptr, the new node will become the new head of the list
          *@return	whether the node was successfully added into the list
          *@see clear, RemoveNode
         */
        bool insert(const ElementType &InElement, Node *NodeToInsertBefore = nullptr) {
            return insert(new Node(InElement), NodeToInsertBefore);
        }
        
        bool insert(Node *NewNode, Node *NodeToInsertBefore = nullptr) {
            if (NewNode == nullptr) {
                return false;
            }
            
            if (NodeToInsertBefore == nullptr || NodeToInsertBefore == _headNode) {
                return push_front(NewNode);
            }
            
            NewNode->_prev = NodeToInsertBefore->_prev;
            NewNode->_next = NodeToInsertBefore;
            
            NodeToInsertBefore->_prev->_next = NewNode;
            NodeToInsertBefore->_prev = NewNode;
            
            setListSize(_size + 1);
            return true;
        }
        
        /**
          *Remove the node corresponding to InElement.
         *
          *@param InElement The value to remove from the list.
          *@see clear, insert
         */
        void erase(const ElementType &InElement) {
            Node *ExistingNode = findNode(InElement);
            erase(ExistingNode);
        }
        
        /**
          *Removes the node specified.
         *
          *@param nodeToRemove The node to remove.
          *@see clear, insert
         */
        void erase(Node *nodeToRemove, bool shouldDeleteNode = true) {
            if (nodeToRemove != nullptr) {
                // if we only have one node, just call Clear() so that we don't have to do lots of extra checks in the code below
                if (size() == 1) {
                    RxCheck(nodeToRemove == _headNode);
                    if (shouldDeleteNode) {
                        clear();
                    } else {
                        nodeToRemove->_next = nodeToRemove->_prev = nullptr;
                        _headNode = _tailNode = nullptr;
                        setListSize(0);
                    }
                    return;
                }
                
                if (nodeToRemove == _headNode) {
                    _headNode = _headNode->_next;
                    _headNode->_prev = nullptr;
                } else if (nodeToRemove == _tailNode) {
                    _tailNode = _tailNode->_prev;
                    _tailNode->_next = nullptr;
                } else {
                    nodeToRemove->_next->_prev = nodeToRemove->_prev;
                    nodeToRemove->_prev->_next = nodeToRemove->_next;
                }
                
                if (shouldDeleteNode) {
                    delete nodeToRemove;
                } else {
                    nodeToRemove->_next = nodeToRemove->_prev = nullptr;
                }
                setListSize(_size - 1);
            }
        }
        
        /* *Removes all nodes from the list. */
        void clear() {
            Node *Node;
            while (_headNode != nullptr) {
                Node = _headNode->_next;
                delete _headNode;
                _headNode = Node;
            }
            
            _headNode = _tailNode = nullptr;
            setListSize(0);
        }
        
        // Accessors.
        
        /**
          *Returns the node at the head of the list.
         *
          *@return Pointer to the first node.
          *@see getTail
         */
        Node *getHead() const {
            return _headNode;
        }
        
        /**
          *Returns the node at the end of the list.
         *
          *@return Pointer to the last node.
          *@see getHead
         */
        Node *getTail() const {
            return _tailNode;
        }
        
        /**
          *Finds the node corresponding to the value specified
         *
          *@param	InElement	the value to find
          *@return	a pointer to the node that contains the value specified, or nullptr of the value couldn't be found
         */
        Node *findNode(const ElementType &InElement) {
            Node *Node = _headNode;
            while (Node != nullptr) {
                if (Node->getValue() == InElement) {
                    break;
                }
                
                Node = Node->_next;
            }
            
            return Node;
        }
        
        bool contains(const ElementType &InElement) {
            return (findNode(InElement) != nullptr);
        }
        
        /**
          *Returns the number of items in the list.
         *
          *@return Item count.
         */
        int32_t size() const {
            return _size;
        }
        
    protected:
        
        /**
          *Updates the size reported by size().  Child classes can use this function to conveniently
          *hook into list additions/removals.
         *
          *@param	NewListSize		the new size for this list
         */
        virtual void setListSize(int32_t NewListSize) {
            _size = NewListSize;
        }
        
    private:
        Node *_headNode;
        Node *_tailNode;
        int32_t _size;
        
        DoubleLinkedList(const DoubleLinkedList&);
        DoubleLinkedList &operator=(const DoubleLinkedList&);
        
        friend Iterator      begin(      DoubleLinkedList &List) { return Iterator     (List.getHead()); }
        friend ConstIterator begin(const DoubleLinkedList &List) { return ConstIterator(List.getHead()); }
        friend Iterator      end  (      DoubleLinkedList &List) { return Iterator     (nullptr); }
        friend ConstIterator end  (const DoubleLinkedList &List) { return ConstIterator(nullptr); }
    };
    
    
    /*----------------------------------------------------------------------------
     List.
     ----------------------------------------------------------------------------*/
    
    //
    // Simple single-linked list template.
    //
    template <class ElementType>
    class List {
    public:
        // Constructor.
        
        List(const ElementType &InElement, List<ElementType> *InNext = nullptr) RX_NOEXCEPT {
            _element = InElement;
            _next = InNext;
        }
        
    public:
        const ElementType &getElement() const RX_NOEXCEPT { return _element; }
        ElementType getElement() RX_NOEXCEPT { return _element; }
        void setElement(const ElementType &value) RX_NOEXCEPT { _element = value; }
        
        const List<ElementType> *&getNext() const RX_NOEXCEPT { return _next; }
        List<ElementType> *getNext() RX_NOEXCEPT { return _next; }
        void setNext(const List<ElementType> *&value) RX_NOEXCEPT { _next = value; }
        
    private:
        ElementType			_element;
        List<ElementType>*	_next;
    };
}

#endif /// *List_h */
