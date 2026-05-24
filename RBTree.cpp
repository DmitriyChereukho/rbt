#include "RBTree.h"
#include <algorithm>
#include <iostream>
#include <utility>

constexpr bool RED = true;
constexpr bool BLACK = false;

// ============================================================
// Node methods
// ============================================================

BinarySearchTree::Node::Node(Key key, Value value,
                              Node *parent, Node *left, Node *right)
    : keyValuePair(key, value), parent(parent), left(left), right(right), color(RED) {}

BinarySearchTree::Node::Node(const Node &other)
    : keyValuePair(other.keyValuePair)
    , parent(nullptr)
    , left(nullptr)
    , right(nullptr)
    , color(other.color)
{
    if (other.left != nullptr) {
        left = new Node(*other.left);
        left->parent = this;
    }
    if (other.right != nullptr) {
        right = new Node(*other.right);
        right->parent = this;
    }
}

bool BinarySearchTree::Node::operator==(const Node &other) const {
    return keyValuePair.first == other.keyValuePair.first &&
           keyValuePair.second == other.keyValuePair.second;
}

void BinarySearchTree::Node::output_node_tree() const {
    if (left != nullptr) {
        left->output_node_tree();
    }
    std::cout << "Key: " << keyValuePair.first
              << " Value: " << keyValuePair.second
              << " Color: " << (color == RED ? "RED" : "BLACK") << std::endl;
    if (right != nullptr) {
        right->output_node_tree();
    }
}

void BinarySearchTree::Node::insert(const Key &key, const Value &value) {
    if (key < keyValuePair.first) {
        if (left == nullptr) {
            left = new Node(key, value, this);
        } else {
            left->insert(key, value);
        }
    } else {
        if (right == nullptr) {
            right = new Node(key, value, this);
        } else {
            right->insert(key, value);
        }
    }
}

void BinarySearchTree::Node::erase(const Key &key) {
    // Handled at tree level with fixup; this method exists for the ABI
    if (key < keyValuePair.first) {
        if (left != nullptr) {
            left->erase(key);
        }
    } else if (key > keyValuePair.first) {
        if (right != nullptr) {
            right->erase(key);
        }
    }
}

void BinarySearchTree::Node::clear() {
    if (left != nullptr) {
        left->clear();
        delete left;
        left = nullptr;
    }
    if (right != nullptr) {
        right->clear();
        delete right;
        right = nullptr;
    }
}

// ============================================================
// Private static helpers for tree navigation
// ============================================================

BinarySearchTree::Node *BinarySearchTree::treeMin(Node *node) {
    while (node != nullptr && node->left != nullptr) {
        node = node->left;
    }
    return node;
}

BinarySearchTree::Node *BinarySearchTree::treeMax(Node *node) {
    while (node != nullptr && node->right != nullptr) {
        node = node->right;
    }
    return node;
}

// ============================================================
// Tree: Rotations
// ============================================================

void BinarySearchTree::LeftRotate(Node *x) {
    Node *y = x->right;
    if (y == nullptr) return;

    x->right = y->left;
    if (y->left != nullptr) {
        y->left->parent = x;
    }

    y->parent = x->parent;
    if (x->parent == nullptr) {
        _root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

void BinarySearchTree::RightRotate(Node *y) {
    Node *x = y->left;
    if (x == nullptr) return;

    y->left = x->right;
    if (x->right != nullptr) {
        x->right->parent = y;
    }

    x->parent = y->parent;
    if (y->parent == nullptr) {
        _root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }

    x->right = y;
    y->parent = x;
}

// ============================================================
// Tree: Insertion
// ============================================================

void BinarySearchTree::insert(const Key &key, const Value &value) {
    // BST insert (iterative)
    Node *parent = nullptr;
    Node *cur = _root;
    while (cur != nullptr) {
        parent = cur;
        if (key < cur->keyValuePair.first) {
            cur = cur->left;
        } else {
            cur = cur->right;
        }
    }

    Node *z = new Node(key, value, parent);
    if (parent == nullptr) {
        _root = z;
    } else if (key < parent->keyValuePair.first) {
        parent->left = z;
    } else {
        parent->right = z;
    }

    _size++;
    RBInsertFixup(z);
}

void BinarySearchTree::RBInsertFixup(Node *z) {
    while (z != _root && z->parent != nullptr && z->parent->color == RED) {
        Node *gp = z->parent->parent;
        if (gp == nullptr) break;

        if (z->parent == gp->left) {
            Node *uncle = gp->right;
            if (uncle != nullptr && uncle->color == RED) {
                // Case 1: uncle is red
                z->parent->color = BLACK;
                uncle->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if (z == z->parent->right) {
                    // Case 2: z is right child
                    z = z->parent;
                    LeftRotate(z);
                }
                // Case 3: z is left child
                z->parent->color = BLACK;
                gp->color = RED;
                RightRotate(gp);
            }
        } else {
            Node *uncle = gp->left;
            if (uncle != nullptr && uncle->color == RED) {
                // Case 1: uncle is red
                z->parent->color = BLACK;
                uncle->color = BLACK;
                gp->color = RED;
                z = gp;
            } else {
                if (z == z->parent->left) {
                    // Case 2: z is left child
                    z = z->parent;
                    RightRotate(z);
                }
                // Case 3: z is right child
                z->parent->color = BLACK;
                gp->color = RED;
                LeftRotate(gp);
            }
        }
    }
    _root->color = BLACK;
}

// ============================================================
// Tree: Deletion
// ============================================================

void BinarySearchTree::RBEraseFixup(Node *x) {
    while (x != _root && (x == nullptr || x->color == BLACK)) {
        Node *parent;
        bool isLeft;

        if (x != nullptr) {
            parent = x->parent;
            isLeft = (x == parent->left);
        } else {
            parent = _efParent;
            isLeft = _efIsLeft;
        }

        if (parent == nullptr) break;

        if (isLeft) {
            Node *w = parent->right;
            if (w == nullptr) break;

            if (w->color == RED) {
                w->color = BLACK;
                parent->color = RED;
                LeftRotate(parent);
                w = parent->right;
                if (w == nullptr) break;
            }
            if ((w->left == nullptr || w->left->color == BLACK) &&
                (w->right == nullptr || w->right->color == BLACK)) {
                w->color = RED;
                x = parent;
            } else {
                if (w->right == nullptr || w->right->color == BLACK) {
                    if (w->left != nullptr) w->left->color = BLACK;
                    w->color = RED;
                    RightRotate(w);
                    w = parent->right;
                    if (w == nullptr) break;
                }
                w->color = parent->color;
                parent->color = BLACK;
                if (w->right != nullptr) w->right->color = BLACK;
                LeftRotate(parent);
                x = _root;
            }
        } else {
            Node *w = parent->left;
            if (w == nullptr) break;

            if (w->color == RED) {
                w->color = BLACK;
                parent->color = RED;
                RightRotate(parent);
                w = parent->left;
                if (w == nullptr) break;
            }
            if ((w->right == nullptr || w->right->color == BLACK) &&
                (w->left == nullptr || w->left->color == BLACK)) {
                w->color = RED;
                x = parent;
            } else {
                if (w->left == nullptr || w->left->color == BLACK) {
                    if (w->right != nullptr) w->right->color = BLACK;
                    w->color = RED;
                    LeftRotate(w);
                    w = parent->left;
                    if (w == nullptr) break;
                }
                w->color = parent->color;
                parent->color = BLACK;
                if (w->left != nullptr) w->left->color = BLACK;
                RightRotate(parent);
                x = _root;
            }
        }
    }
    if (x != nullptr) {
        x->color = BLACK;
    }
}

void BinarySearchTree::erase(const Key &key) {
    // Find node to delete (first/leftmost with this key)
    Node *z = _root;
    while (z != nullptr) {
        if (key < z->keyValuePair.first) {
            z = z->left;
        } else if (key > z->keyValuePair.first) {
            z = z->right;
        } else {
            // Found one — check for leftmost with same key
            Node *leftmost = z;
            while (leftmost->left != nullptr &&
                   leftmost->left->keyValuePair.first == key) {
                leftmost = leftmost->left;
            }
            z = leftmost;
            break;
        }
    }

    if (z == nullptr) return;
    _size--;

    Node *y = z;
    bool y_original_color = y->color;
    Node *x = nullptr;

    // Save double-black tracking info in case x ends up nullptr
    _efParent = nullptr;
    _efIsLeft = false;

    if (z->left == nullptr) {
        x = z->right;
        if (x == nullptr) {
            _efParent = z->parent;
            _efIsLeft = (z->parent != nullptr && z == z->parent->left);
        }
        // Transplant z with z->right
        if (z->parent == nullptr) {
            _root = z->right;
        } else if (z == z->parent->left) {
            z->parent->left = z->right;
        } else {
            z->parent->right = z->right;
        }
        if (z->right != nullptr) {
            z->right->parent = z->parent;
        }
    } else if (z->right == nullptr) {
        x = z->left;
        if (x == nullptr) {
            _efParent = z->parent;
            _efIsLeft = (z->parent != nullptr && z == z->parent->left);
        }
        // Transplant z with z->left
        if (z->parent == nullptr) {
            _root = z->left;
        } else if (z == z->parent->left) {
            z->parent->left = z->left;
        } else {
            z->parent->right = z->left;
        }
        z->left->parent = z->parent;
    } else {
        y = treeMin(z->right);
        y_original_color = y->color;
        x = y->right;

        if (x == nullptr) {
            _efParent = (y->parent != z) ? y->parent : y;
            _efIsLeft = (y->parent != z) ? (y == y->parent->left) : false;
        }

        if (y->parent != z) {
            // Transplant y with y->right
            if (y == y->parent->left) {
                y->parent->left = y->right;
            } else {
                y->parent->right = y->right;
            }
            if (y->right != nullptr) {
                y->right->parent = y->parent;
            }
            y->right = z->right;
            if (y->right != nullptr) {
                y->right->parent = y;
            }
        }

        // Transplant z with y
        if (z->parent == nullptr) {
            _root = y;
        } else if (z == z->parent->left) {
            z->parent->left = y;
        } else {
            z->parent->right = y;
        }
        y->parent = z->parent;
        y->left = z->left;
        if (y->left != nullptr) {
            y->left->parent = y;
        }
        y->color = z->color;
    }

    delete z;

    if (y_original_color == BLACK) {
        RBEraseFixup(x);
    }
}

// ============================================================
// Tree: TreeSuccessor
// ============================================================

BinarySearchTree::Node *BinarySearchTree::TreeSuccessor(Node *node) const {
    if (node == nullptr) return nullptr;
    if (node->right != nullptr) {
        return treeMin(node->right);
    }
    Node *parent = node->parent;
    while (parent != nullptr && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

// ============================================================
// Tree: find / rfind
// ============================================================

BinarySearchTree::Iterator BinarySearchTree::find(const Key &key) {
    Node *cur = _root;
    Node *result = nullptr;
    while (cur != nullptr) {
        if (key < cur->keyValuePair.first) {
            cur = cur->left;
        } else if (key > cur->keyValuePair.first) {
            cur = cur->right;
        } else {
            result = cur;
            cur = cur->left;
        }
    }
    return Iterator(result);
}

BinarySearchTree::ConstIterator BinarySearchTree::find(const Key &key) const {
    const Node *cur = _root;
    const Node *result = nullptr;
    while (cur != nullptr) {
        if (key < cur->keyValuePair.first) {
            cur = cur->left;
        } else if (key > cur->keyValuePair.first) {
            cur = cur->right;
        } else {
            result = cur;
            cur = cur->left;
        }
    }
    return ConstIterator(result);
}

BinarySearchTree::Iterator BinarySearchTree::rfind(const Key &key) {
    Node *cur = _root;
    Node *result = nullptr;
    while (cur != nullptr) {
        if (key < cur->keyValuePair.first) {
            cur = cur->left;
        } else if (key > cur->keyValuePair.first) {
            cur = cur->right;
        } else {
            result = cur;
            cur = cur->right;
        }
    }
    return Iterator(result);
}

// ============================================================
// Tree: equalRange
// ============================================================

std::pair<BinarySearchTree::Iterator, BinarySearchTree::Iterator>
BinarySearchTree::equalRange(const Key &key) {
    Iterator first = find(key);
    if (first == end()) {
        return {first, first};
    }
    Node *cur = _root;
    Node *upper = nullptr;
    while (cur != nullptr) {
        if (key < cur->keyValuePair.first) {
            upper = cur;
            cur = cur->left;
        } else {
            cur = cur->right;
        }
    }
    return {first, Iterator(upper)};
}

std::pair<BinarySearchTree::ConstIterator, BinarySearchTree::ConstIterator>
BinarySearchTree::equalRange(const Key &key) const {
    ConstIterator first = find(key);
    if (first == cend()) {
        return {first, first};
    }
    const Node *cur = _root;
    const Node *upper = nullptr;
    while (cur != nullptr) {
        if (key < cur->keyValuePair.first) {
            upper = cur;
            cur = cur->left;
        } else {
            cur = cur->right;
        }
    }
    return {first, ConstIterator(upper)};
}

// ============================================================
// Tree: min / max
// ============================================================

BinarySearchTree::ConstIterator BinarySearchTree::min() const {
    return ConstIterator(treeMin(_root));
}

BinarySearchTree::ConstIterator BinarySearchTree::max() const {
    return ConstIterator(treeMax(_root));
}

BinarySearchTree::ConstIterator BinarySearchTree::min(const Key &key) const {
    return find(key);
}

BinarySearchTree::ConstIterator BinarySearchTree::max(const Key &key) const {
    const Node *cur = _root;
    const Node *result = nullptr;
    while (cur != nullptr) {
        if (key < cur->keyValuePair.first) {
            cur = cur->left;
        } else if (key > cur->keyValuePair.first) {
            cur = cur->right;
        } else {
            result = cur;
            cur = cur->right;
        }
    }
    return ConstIterator(result);
}

// ============================================================
// Tree: Iterators
// ============================================================

BinarySearchTree::Iterator::Iterator(Node *node) : _node(node) {}

std::pair<Key, Value> &BinarySearchTree::Iterator::operator*() {
    return _node->keyValuePair;
}

const std::pair<Key, Value> &BinarySearchTree::Iterator::operator*() const {
    return _node->keyValuePair;
}

std::pair<Key, Value> *BinarySearchTree::Iterator::operator->() {
    return &_node->keyValuePair;
}

const std::pair<Key, Value> *BinarySearchTree::Iterator::operator->() const {
    return &_node->keyValuePair;
}

BinarySearchTree::Iterator BinarySearchTree::Iterator::operator++() {
    if (_node == nullptr) return *this;
    if (_node->right != nullptr) {
        _node = treeMin(_node->right);
    } else {
        Node *parent = _node->parent;
        while (parent != nullptr && _node == parent->right) {
            _node = parent;
            parent = parent->parent;
        }
        _node = parent;
    }
    return *this;
}

BinarySearchTree::Iterator BinarySearchTree::Iterator::operator++(int) {
    Iterator tmp(*this);
    ++(*this);
    return tmp;
}

BinarySearchTree::Iterator BinarySearchTree::Iterator::operator--() {
    if (_node == nullptr) return *this;
    if (_node->left != nullptr) {
        _node = treeMax(_node->left);
    } else {
        Node *parent = _node->parent;
        while (parent != nullptr && _node == parent->left) {
            _node = parent;
            parent = parent->parent;
        }
        _node = parent;
    }
    return *this;
}

BinarySearchTree::Iterator BinarySearchTree::Iterator::operator--(int) {
    Iterator tmp(*this);
    --(*this);
    return tmp;
}

bool BinarySearchTree::Iterator::operator==(const Iterator &other) const {
    return _node == other._node;
}

bool BinarySearchTree::Iterator::operator!=(const Iterator &other) const {
    return _node != other._node;
}

// --- ConstIterator ---

BinarySearchTree::ConstIterator::ConstIterator(const Node *node) : _node(node) {}

const std::pair<Key, Value> &BinarySearchTree::ConstIterator::operator*() const {
    return _node->keyValuePair;
}

const std::pair<Key, Value> *BinarySearchTree::ConstIterator::operator->() const {
    return &_node->keyValuePair;
}

BinarySearchTree::ConstIterator BinarySearchTree::ConstIterator::operator++() {
    if (_node == nullptr) return *this;
    if (_node->right != nullptr) {
        _node = treeMin(_node->right);
    } else {
        const Node *parent = _node->parent;
        while (parent != nullptr && _node == parent->right) {
            _node = parent;
            parent = parent->parent;
        }
        _node = parent;
    }
    return *this;
}

BinarySearchTree::ConstIterator BinarySearchTree::ConstIterator::operator++(int) {
    ConstIterator tmp(*this);
    ++(*this);
    return tmp;
}

BinarySearchTree::ConstIterator BinarySearchTree::ConstIterator::operator--() {
    if (_node == nullptr) return *this;
    if (_node->left != nullptr) {
        _node = treeMax(_node->left);
    } else {
        const Node *parent = _node->parent;
        while (parent != nullptr && _node == parent->left) {
            _node = parent;
            parent = parent->parent;
        }
        _node = parent;
    }
    return *this;
}

BinarySearchTree::ConstIterator BinarySearchTree::ConstIterator::operator--(int) {
    ConstIterator tmp(*this);
    --(*this);
    return tmp;
}

bool BinarySearchTree::ConstIterator::operator==(const ConstIterator &other) const {
    return _node == other._node;
}

bool BinarySearchTree::ConstIterator::operator!=(const ConstIterator &other) const {
    return _node != other._node;
}

// ============================================================
// Tree: begin / end / cbegin / cend
// ============================================================

BinarySearchTree::Iterator BinarySearchTree::begin() {
    return Iterator(treeMin(_root));
}

BinarySearchTree::Iterator BinarySearchTree::end() {
    return Iterator(nullptr);
}

BinarySearchTree::ConstIterator BinarySearchTree::cbegin() const {
    return ConstIterator(treeMin(_root));
}

BinarySearchTree::ConstIterator BinarySearchTree::cend() const {
    return ConstIterator(nullptr);
}

// ============================================================
// Tree: size / max_height / output_tree
// ============================================================

size_t BinarySearchTree::size() const {
    return _size;
}

size_t BinarySearchTree::max_height() const {
    // Compute max height iteratively to avoid needing a free function with Node*
    if (_root == nullptr) return 0;
    // Recursive lambda
    auto height = [](auto &&self, const Node *node) -> size_t {
        if (node == nullptr) return 0;
        return 1 + std::max(self(self, node->left), self(self, node->right));
    };
    return height(height, _root);
}

void BinarySearchTree::output_tree() {
    if (_root != nullptr) {
        _root->output_node_tree();
    }
}

// ============================================================
// Tree: Constructors, destructor, assignment
// ============================================================

BinarySearchTree::BinarySearchTree() : _size(0), _root(nullptr) {}

BinarySearchTree::BinarySearchTree(const BinarySearchTree &other)
    : _size(other._size), _root(nullptr) {
    if (other._root != nullptr) {
        _root = new Node(*other._root);
    }
}

BinarySearchTree &BinarySearchTree::operator=(const BinarySearchTree &other) {
    if (this != &other) {
        if (_root != nullptr) {
            _root->clear();
            delete _root;
        }
        _root = nullptr;
        _size = other._size;
        if (other._root != nullptr) {
            _root = new Node(*other._root);
        }
    }
    return *this;
}

BinarySearchTree::BinarySearchTree(BinarySearchTree &&other) noexcept
    : _size(other._size), _root(other._root) {
    other._size = 0;
    other._root = nullptr;
}

BinarySearchTree &BinarySearchTree::operator=(BinarySearchTree &&other) noexcept {
    if (this != &other) {
        if (_root != nullptr) {
            _root->clear();
            delete _root;
        }
        _root = other._root;
        _size = other._size;
        other._root = nullptr;
        other._size = 0;
    }
    return *this;
}

BinarySearchTree::~BinarySearchTree() {
    if (_root != nullptr) {
        _root->clear();
        delete _root;
    }
}
