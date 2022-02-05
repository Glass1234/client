#pragma once

template<typename T>
struct DLLIST_NODE {
	T data;
	DLLIST_NODE<T>* prev = nullptr;
	DLLIST_NODE<T>* next = nullptr;
};

template<typename T>
DLLIST_NODE<T>* __DLLIST_addNode(DLLIST_NODE<T>* prev, DLLIST_NODE<T>* next, T data) {
	DLLIST_NODE<T>* newNode = new DLLIST_NODE<T>;

	if (prev != nullptr) {
		prev->next = newNode;
		newNode->prev = prev;
	} else {
		newNode->prev = nullptr;
	}

	if (next != nullptr) {
		next->prev = newNode;
		newNode->next = next;
	} else {
		newNode->next = nullptr;
	}

	newNode->data = data;

	return newNode;
}

template<typename T>
DLLIST_NODE<T>* DLLIST_addNode(DLLIST_NODE<T>* root, T data) {
	if (root != nullptr)
		return __DLLIST_addNode<T>(root, root->next, data);
	else
		return __DLLIST_addNode<T>(nullptr, nullptr, data);
}

template<typename T>
int __DLLIST_size(DLLIST_NODE<T>* root, int size) {
	if (root->next != nullptr) {
		return __DLLIST_size(root->next, size + 1);
	} else {
		return size;
	}
}

template<typename T>
int DLLIST_size(DLLIST_NODE<T>* root) {
	return __DLLIST_size(root, 0);
}

template<typename T>
T __DLLIST_deleteNode(DLLIST_NODE<T>* node) {
	if (node == nullptr) return nullptr;

	if (node->prev != nullptr) {
		node->prev->next = node->next;
	}

	if (node->next != nullptr) {
		node->next->prev = node->prev;
	}

	return node->data;
}

template<typename T>
T DLLIST_freeNode(DLLIST_NODE<T>* node) {
	if (node == nullptr) return nullptr;

	T data = __DLLIST_deleteNode(node);
	delete node;
	return data;
}

template<typename T>
void DLLIST_freeChain(DLLIST_NODE<T>* node) {
	if (node == nullptr) return;

	if (node->next != nullptr) {
		DLLIST_freeChain(node->next);
	} else if (node->prev != nullptr) {
		DLLIST_freeChain(node->prev);
	}
	DLLIST_freeNode(node);
}