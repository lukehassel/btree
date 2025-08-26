#ifndef BTREE_VIZ_H
#define BTREE_VIZ_H

#include "btree.h"

/**
 * @brief Write a B+ tree to a DOT file for Graphviz visualization.
 * @param tree The B+ tree to visualize
 * @param filename The output filename
 * @return 0 on success, -1 on failure
 */
int bplus_tree_write_dot(BPlusTree* tree, const char* filename);

/**
 * @brief Recursively write a node and its children to the DOT file.
 * @param file The output file
 * @param node The node to write
 * @param node_id The unique identifier for this node
 * @return The next available node ID
 */
int write_node(FILE* file, Node* node, int node_id);

/**
 * @brief Render a B+ tree as a PNG image using Graphviz.
 * @param tree The B+ tree to visualize
 * @param filename The output filename (without extension)
 * @return 0 on success, -1 on failure
 */
int bplus_tree_render_png(BPlusTree* tree, const char* filename);

#endif // BTREE_VIZ_H
