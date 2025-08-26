#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"
#include "btree_viz.h"

/**
 * @brief Write a B+ tree to a DOT file for Graphviz visualization.
 * @param tree The B+ tree to visualize
 * @param filename The output filename
 * @return 0 on success, -1 on failure
 */
int bplus_tree_write_dot(BPlusTree* tree, const char* filename) {
    if (tree == NULL || filename == NULL) {
        return -1;
    }
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return -1;
    }
    
    // Write DOT header
    fprintf(file, "digraph BPlusTree {\n");
    fprintf(file, "  rankdir=TB;\n");
    fprintf(file, "  node [shape=record, style=filled, fillcolor=lightblue];\n");
    fprintf(file, "  edge [color=blue];\n\n");
    
    // Debug: Print root properties
    printf("Debug: Root node at %p\n", (void*)tree->root);
    if (tree->root) {
        printf("Debug: Root is_leaf = %d, num_keys = %d\n", tree->root->is_leaf, tree->root->num_keys);
    }
    
    // Write tree structure
    if (tree->root != NULL) {
        write_node(file, tree->root, 0);
    }
    
    fprintf(file, "}\n");
    fclose(file);
    
    return 0;
}

/**
 * @brief Recursively write a node and its children to the DOT file.
 * @param file The output file
 * @param node The node to write
 * @param node_id The unique identifier for this node
 * @return The next available node ID
 */
int write_node(FILE* file, Node* node, int node_id) {
    if (node == NULL) {
        return node_id;
    }
    
    // Create node label
    char label[1024];
    char key_str[256];
    label[0] = '\0';
    
    strcat(label, "{");
    
    // Add keys
    for (int i = 0; i < node->num_keys; i++) {
        if (i > 0) strcat(label, "|");
        sprintf(key_str, "%d", *(int*)node->keys[i]);
        strcat(label, key_str);
    }
    
    strcat(label, "}");
    
    // Write node
    fprintf(file, "  node_%d [label=\"%s\"];\n", node_id, label);
    
    // Write edges to children (for internal nodes)
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            if (node->pointers[i] != NULL) {
                int child_id = write_node(file, (Node*)node->pointers[i], node_id + 1);
                fprintf(file, "  node_%d -> node_%d;\n", node_id, node_id + 1);
                node_id = child_id;
            }
        }
    } else {
        // For leaf nodes, add next pointer for leaf linkage
        if (node->next != NULL) {
            fprintf(file, "  node_%d -> node_%d [style=dashed, color=red];\n", node_id, node_id + 1);
        }
    }
    
    return node_id + 1;
}

/**
 * @brief Render a B+ tree as a PNG image using Graphviz.
 * @param tree The B+ tree to visualize
 * @param filename The output filename (without extension)
 * @return 0 on success, -1 on failure
 */
int bplus_tree_render_png(BPlusTree* tree, const char* filename) {
    if (tree == NULL || filename == NULL) {
        return -1;
    }
    
    // Create DOT filename
    char dot_filename[256];
    snprintf(dot_filename, sizeof(dot_filename), "%s.dot", filename);
    
    // Write DOT file
    if (bplus_tree_write_dot(tree, dot_filename) != 0) {
        return -1;
    }
    
    // Create PNG filename
    char png_filename[256];
    snprintf(png_filename, sizeof(png_filename), "%s.png", filename);
    
    // Use Graphviz dot command to generate PNG
    char command[512];
    snprintf(command, sizeof(command), "dot -Tpng %s -o %s", dot_filename, png_filename);
    
    int result = system(command);
    
    // Clean up DOT file
    remove(dot_filename);
    
    if (result == 0) {
        printf("✅ B+ tree visualization saved as %s\n", png_filename);
        return 0;
    } else {
        printf("❌ Failed to generate PNG visualization\n");
        return -1;
    }
}
