#include "bpt.h"



// GLOBALS.



/* order of tree */
int order = DEFAULT_ORDER;



// FUNCTION DEFINITIONS.

// OPEN AND EXIT.
int open_table(char* pathname) {

    if (strlen(pathname) > 20) {
        printf("Invalid length of filename: %ld (max: 20)\n", strlen(pathname));
        return -1;
    }
    return buf_open_table(pathname);
}

int init_db(int num_buf) {
    atexit(db_exit);
    return buf_init(num_buf);
}

void db_exit(void) {

    shutdown_db();
    printf("bye\n");
}



// SEARCH.

int db_find(int table_id, int64_t key, char* ret_val) {

    buffer_frame_t* tmp_h = get_buf_frame(table_id, HEADER_PAGE_OFFSET);

    // root doesn't exist.
    if (tmp_h->frame.header_page.root == 0) {
        unpin(tmp_h);
        return -1;
    }

    pagenum_t c = find_leaf(table_id, tmp_h->frame.header_page.root, key);
    //printf("header's pin: %d\n", tmp_h->is_pinned);
    unpin(tmp_h);
    buffer_frame_t* tmp_l = get_buf_frame(table_id, c);

    int i;
    for (i = 0; i < tmp_l->frame.node.num_keys; i++) {
        if (tmp_l->frame.node.records[i].key == key) break;
    }

    // key doesn't exist.
    if (i == tmp_l->frame.node.num_keys) {
        unpin(tmp_l);
        return -1;
    }

    unpin(tmp_l);
    return 0;

    /* disk-based version find. */

    /*
    if (get_header_root() == 0)
        return NULL;

    int i;
    pagenum_t c = find_leaf(key);
    page_t * tmp = (page_t*)malloc(sizeof(page_t));
    file_read_page(c, tmp);

    for (i = 0; i < tmp->node.num_keys; i++)
        if (tmp->node.records[i].key == key) break;
    if (i == tmp->node.num_keys)
        return NULL;
    else
        return tmp->node.records[i].value;
    */
}

/* return proper leaf page position to the key.
 * Notice, when using it in deletion.
 */
pagenum_t find_leaf(int table_id, pagenum_t root, int64_t key) {

    int i;
    pagenum_t c = root;
    buffer_frame_t* tmp_r = get_buf_frame(table_id, c);

    while(!tmp_r->frame.node.is_leaf) {

        i = 0;
        while (i < tmp_r->frame.node.num_keys) {

            if (key >= tmp_r->frame.node.internal[i].key) i++;
            else break;
        }

        if (i == 0) c = tmp_r->frame.node.right_or_indexing;
        else c = tmp_r->frame.node.internal[i - 1].pointer;

        unpin(tmp_r);
        tmp_r = get_buf_frame(table_id, c);
    }

    unpin(tmp_r);
    return c;
}

// INSERTION

/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
int db_insert(int table_id, int64_t key, char* value) {

    if (table_id == -1) {
        return -1;
    }

    int ret_val;

    // Case 1: The pair already exists.
    if (db_find(table_id, key, value) == 0) {
        //Nope, can't insert.
        return -1; 
    }
    //Yes. can insert.



    buffer_frame_t* header = get_buf_frame(table_id, HEADER_PAGE_OFFSET);

    // Case 2: Root page doesn't exist yet.
    //         (only header page)
    //      -> make root and write.

    if (header->frame.header_page.root == 0) {

        ret_val = start_new_tree(header, key, value);
        unpin(header);
        return ret_val;
    }

    // Case 2-1: Root page already exists.

    pagenum_t leaf = find_leaf(table_id, header->frame.header_page.root, key);
    buffer_frame_t* tmp_l = get_buf_frame(table_id, leaf);

    // Case 2-1-1: Leaf page has space for a record.
    if (tmp_l->frame.node.num_keys < order - 1) {
        ret_val = insert_into_leaf(tmp_l, key, value);
        unpin(header);
        unpin(tmp_l);
        return ret_val;
    }

    // Case 2-1-2: Leaf page must be split first.
    ret_val = insert_into_leaf_after_splitting(header, tmp_l, key, value);
    unpin(header);
    unpin(tmp_l);
    return ret_val;
}

int insert_into_node_after_splitting(buffer_frame_t* header, buffer_frame_t* old_node, int left_index, int64_t key, pagenum_t right) {

    int split, i , j, ret_val;
    buffer_frame_t* new_node = buffer_alloc_page(header);
    //new_node->frame.node.is_leaf = 0;
    int64_t k_prime;
    
    internal_t tmp_internal[order];
    memset(&tmp_internal, 0, 16 * order);

    //left index is 'insertion point' as
    //insert_into_leaf_after_splitting.
    for (i = 0, j = 0; i < old_node->frame.node.num_keys; i++, j++) {
        if (j == left_index) j++;
        tmp_internal[j] = old_node->frame.node.internal[i];
    }

    tmp_internal[left_index].key = key;
    tmp_internal[left_index].pointer = right;

    split = cut(order - 1);
    //if order is 4, split is 2.
    //if order is 5, slpit is 2.
    old_node->frame.node.num_keys = 0;
    for (i = 0; i < split - 1; i++) {
        old_node->frame.node.internal[i]= tmp_internal[i];
        old_node->frame.node.num_keys++;
    }

    k_prime = tmp_internal[split - 1].key;
    for (++i, j = 0; i < order; i++, j++) {
        new_node->frame.node.internal[j] = tmp_internal[i];
        new_node->frame.node.num_keys++;
    }
    //in tmp_old, k_prime's pointer.
    new_node->frame.node.right_or_indexing = tmp_internal[split - 1].pointer;
    new_node->frame.node.parent = old_node->frame.node.parent;

    buffer_frame_t* new_node_left_most_child = get_buf_frame(header->table_id, new_node->frame.node.right_or_indexing);
    new_node_left_most_child->frame.node.parent = new_node->page_num;
    unpin(new_node_left_most_child);
    for (i = 0; i < new_node->frame.node.num_keys; i++) {
        buffer_frame_t* new_node_children = get_buf_frame(header->table_id, new_node->frame.node.internal[i].pointer);
        new_node_children->frame.node.parent = new_node->page_num;
        unpin(new_node_children);
    }

    old_node->is_dirty = 1;

    ret_val = insert_into_parent(header, old_node, k_prime, new_node);
    unpin(new_node);
    return ret_val;
}

int insert_into_node(buffer_frame_t* node, int left_index, int64_t key, pagenum_t right) {

    int i;
    for (i = node->frame.node.num_keys; i > left_index; i--) {
        node->frame.node.internal[i] = node->frame.node.internal[i - 1];
    }
    node->frame.node.internal[left_index].key = key;
    node->frame.node.internal[left_index].pointer = right;
    node->frame.node.num_keys++;

    node->is_dirty = 1;
    return 0;
}

int insert_into_new_root(buffer_frame_t* header, buffer_frame_t* left, int64_t key, buffer_frame_t* right) {


    buffer_frame_t* root = buffer_alloc_page(header);
    //get_buf_empty(header);
    //printf("where...\n");

    root->frame.node.parent = 0;
    root->frame.node.is_leaf = 0;
    root->frame.node.num_keys++;
    root->frame.node.right_or_indexing = left->page_num;

    root->frame.node.internal[0].key = key;
    root->frame.node.internal[0].pointer = right->page_num;
    
    header->frame.header_page.root = root->page_num;
    header->is_dirty = 1;

    left->frame.node.parent = root->page_num;
    right->frame.node.parent = root->page_num;

    unpin(root);
    return 0;
}

int insert_into_parent(buffer_frame_t* header, buffer_frame_t* left, int64_t key, buffer_frame_t* right) {

    int ret_val;

    /* Case 1: new root. */
    if (left->frame.node.parent == 0) {
        return insert_into_new_root(header, left, key, right); //unpin left and right.
    }

    /* Case 2: insert new_key from laaf or internal to existing parent page. */
    buffer_frame_t* parent = get_buf_frame(header->table_id, left->frame.node.parent);
    int left_index = get_left_index(parent, left->page_num);

    printf("left_index = %d\n", left_index);

    /* 2-1 (simple.): the new_key fits into the order. */
    if (parent->frame.node.num_keys < order - 1) {
        ret_val = insert_into_node(parent, left_index, key, right->page_num);
        unpin(parent);
        return ret_val;
    }

    /* 2-2. (hard.): split the parent node */
    ret_val = insert_into_node_after_splitting(header, parent, left_index, key, right->page_num);
    unpin(parent);
    return ret_val;
}

int get_left_index(buffer_frame_t* parent, pagenum_t left) {

    if (parent->frame.node.right_or_indexing == left) {
        return 0;
    }

    int left_idx = 1;
    while (left_idx <= parent->frame.node.num_keys &&
           parent->frame.node.internal[left_idx - 1].pointer != left) {
        left_idx++;
    }
    return left_idx;
}

int insert_into_leaf(buffer_frame_t* leaf, int64_t key, char * value) {

    int i, ip = 0;
    while (ip < leaf->frame.node.num_keys && leaf->frame.node.records[ip].key < key) {
        ip++;
    }

    for (i = leaf->frame.node.num_keys; i > ip; i--) {
        leaf->frame.node.records[i] = leaf->frame.node.records[i - 1];
    }

    leaf->frame.node.records[ip].key = key;
    strcpy(leaf->frame.node.records[ip].value, value);
    ++leaf->frame.node.num_keys;

    leaf->is_dirty = 1;
    unpin(leaf);
    return 0;
}

/* Inserts a new key-value record
 * into the leaf page so as to exceed
 * the tree's order, causing the leaf page
 * to be splited in half.
 */
int insert_into_leaf_after_splitting(buffer_frame_t* header, buffer_frame_t* leaf, int64_t key, char* value) {

    int ip, split, i, j, ret_val;
    buffer_frame_t* new_leaf = buffer_alloc_page(header);
    //get_buf_empty(header);
    new_leaf->frame.node.is_leaf = 1;
    int64_t new_key;

    record_t tmp_records[order];
    memset(&tmp_records, 0, 128 * order);

    ip = 0;
    while (ip < order - 1 && leaf->frame.node.records[ip].key < key) {
        ip++;
    }

    for (i = 0, j = 0; i < leaf->frame.node.num_keys; i++, j++) {
        if (j == ip) {
            j++;
        }
        tmp_records[j] = leaf->frame.node.records[i];
    }
    tmp_records[ip].key = key;
    strcpy(tmp_records[ip].value, value);
    
    leaf->frame.node.num_keys = 0;

    split = cut(order - 1);

    for (i = 0; i < split; i++) {
        leaf->frame.node.records[i].key = tmp_records[i].key;
        strncpy(leaf->frame.node.records[i].value, "", 120);
        strcpy(leaf->frame.node.records[i].value, tmp_records[i].value);
        leaf->frame.node.num_keys++;
    }

    for (i = split, j = 0; i < order; i++, j++) {
        new_leaf->frame.node.records[j] = tmp_records[i];
        new_leaf->frame.node.num_keys++;
    }
    
    new_leaf->frame.node.right_or_indexing = leaf->frame.node.right_or_indexing;
    leaf->frame.node.right_or_indexing = new_leaf->page_num;

    for (i = leaf->frame.node.num_keys; i < order - 1; i++) {
        leaf->frame.node.records[i].key = 0;
        strncpy(leaf->frame.node.records[i].value, "", 120);
    }
    new_leaf->frame.node.parent = leaf->frame.node.parent;

    // TODO: dirty control...ㄷㄷ
    //new_leaf->is_dirty = 1; //는 buffer_alloc_page에서 해줌
    leaf->is_dirty = 1;

    new_key = new_leaf->frame.node.records[0].key;
    //printf("iinap -> iip\n");
    ret_val = insert_into_parent(header, leaf, new_key, new_leaf);
    unpin(new_leaf);
    return ret_val;
}

int start_new_tree(buffer_frame_t* header, int64_t key, char* value) {
    
    buffer_frame_t* tmp_e = buffer_alloc_page(header);
    //get_buf_empty(header);

    tmp_e->frame.node.is_leaf = 1;
    tmp_e->frame.node.num_keys = 1;
    tmp_e->frame.node.records[0].key = key;
    strcpy(tmp_e->frame.node.records[0].value, value);

    if (header->frame.header_page.free == 0) {
        header->frame.header_page.root = 1;
        // header->frame.header_page.num_pages++;
    }
    else {
        header->frame.header_page.root = header->frame.header_page.free;
        buffer_frame_t* tmp_f = get_buf_frame(header->table_id, header->frame.header_page.free);
        header->frame.header_page.free = tmp_f->frame.free_page.next;
        unpin(tmp_f);
    }
    header->is_dirty = 1; 


    tmp_e->page_num = header->frame.header_page.root;
    tmp_e->is_pinned = 1;
    tmp_e->refbit = 1;

    unpin(tmp_e);
    return 0;
}



// DELETION

/* Master deletion function.
 */
int db_delete(int table_id, int64_t key) {

    if (table_id == -1) {
        return -1;
    }

    if (db_find(table_id, key, NULL) == 0) {

        buffer_frame_t* header = get_buf_frame(table_id, HEADER_PAGE_OFFSET);
        pagenum_t c = find_leaf(table_id, header->frame.header_page.root, key);

        buffer_frame_t* leaf = get_buf_frame(table_id, c);
        int ret_val = delete_entry(header, leaf, key);
        unpin(header);
        // TODO: delete_entry에서 다 뽑기 ㄱㄱ
        //unpin(leaf);
        return ret_val;
    }
    return -1;
}

int delete_entry(buffer_frame_t* header, buffer_frame_t* node, int64_t key) {

    int min_keys, ret_val;
    pagenum_t neighbor_page;
    int neighbor_index;
    int k_prime_index;
    int64_t k_prime;
    int capacity;

    // Remove key-value pair from the node.
    remove_entry_from_node(header, node, key);

    /* Case 1: deletion from the root. */
    if (node->page_num == header->frame.header_page.root) {
        unpin(node);
        return adjust_root(header, header->frame.header_page.root);
    }

    /* Case 2: deletion from a node below the root. */

    /* deleyed merge. */
    //min_keys = tmp_n->node.is_leaf ? cut(order - 1) : cut(order) - 1;
    min_keys = 1;

    /* Case 2-1: (simple.) node stays at or above minimum. */
    if (node->frame.node.num_keys >= min_keys) {
        unpin(node);
        return 0;
    }

    /* Case 2-2: (hard.) node falls below minimum,
     * so coalescence or redistribution is needed.
     */
    buffer_frame_t* parent = get_buf_frame(header->table_id, node->frame.node.parent);
    neighbor_index = get_neighbor_index(header, node, parent);

    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = parent->frame.node.internal[k_prime_index].key;

    printf("K_PRIME: %ld\n", k_prime);

    neighbor_page = neighbor_index == -1
                ? parent->frame.node.internal[0].pointer
                : parent->frame.node.internal[neighbor_index - 1].pointer;
    if (neighbor_index == 0) neighbor_page = parent->frame.node.right_or_indexing;
    unpin(parent);
    
    capacity = node->frame.node.is_leaf
             ? order
             : order - 1;

    printf("delete entry - before 'get_buf_frame'\n");
    buffer_frame_t* neighbor = get_buf_frame(header->table_id, neighbor_page);

    /* Coalescence. */
    if (neighbor->frame.node.num_keys + node->frame.node.num_keys < capacity) {
        ret_val = coalesce_nodes(header, node, neighbor, neighbor_index, k_prime);
        unpin(node);
        unpin(neighbor);
        return ret_val;
    }

    /* Redistribution. */
    else {
        printf("REDISTRIBUTION\n");
        ret_val = redistribute_nodes(header, node, neighbor, neighbor_index, k_prime_index, k_prime);
        unpin(node);
        unpin(neighbor);
        return ret_val;
    }
}

void remove_entry_from_node(buffer_frame_t* header, buffer_frame_t* node, int64_t key) {

    int i = 0;
    // Remove the key and shift other keys accordingly.
    if (node->frame.node.is_leaf) { //if n is leaf...

        while (node->frame.node.records[i].key != key) {
            i++;
        }
        for (++i; i < node->frame.node.num_keys; i++) {
            node->frame.node.records[i - 1] = node->frame.node.records[i];
        }
    }

    else { //if n is internal...

        while (node->frame.node.internal[i].key != key) {
            i++;
        }
        for (++i; i < node->frame.node.num_keys; i++) {
            node->frame.node.internal[i - 1] = node->frame.node.internal[i];
        }
    }

    // One key fewer.
    node->frame.node.num_keys--;
    node->is_dirty = 1;
}

int adjust_root(buffer_frame_t* header, pagenum_t r) {

    buffer_frame_t* root = get_buf_frame(header->table_id, r);

    /* Case: nonempty root.
     * Key-value pair has already been deleted.
     * So, nothing to be done.
     */
    if (root->frame.node.num_keys > 0) {
        unpin(root);
        return 0;
    }

    /* Case: empty root. */
    
    /* If it is a internal 
     * that has a child,
     * promote the first (only) child
     * as the new root.
     */
    if (!root->frame.node.is_leaf) {

        pagenum_t only_child = root->frame.node.right_or_indexing;
        buffer_frame_t* new_root = get_buf_frame(header->table_id, only_child);

        new_root->frame.node.parent = 0;
        new_root->is_dirty = 1;
        unpin(new_root);

        header->frame.header_page.root = only_child;
    }

    /* If it is a leaf (has no children),
     * then the whole tree is empty.
     */
    
    buffer_free_page(header, root);
    unpin(root);
    return 0;
}

int get_neighbor_index(buffer_frame_t* header, buffer_frame_t* node, buffer_frame_t* parent) {

    int i;
    if (parent->frame.node.right_or_indexing == node->page_num) {
        return -1;
    }

    for (i = 0; i < parent->frame.node.num_keys; i++) {
        if (parent->frame.node.internal[i].pointer == node->page_num) {
            return i;
        }
    }
}

int coalesce_nodes(buffer_frame_t* header, buffer_frame_t* node, buffer_frame_t* neighbor, int neighbor_index, int64_t k_prime) {

    int ret_val;
    int i, j, neighbor_insertion_index, n_end;
    buffer_frame_t* swap;

    buffer_frame_t* parent = get_buf_frame(header->table_id, node->frame.node.parent);

    /* Swap neighbor with node if node is on the
     * extreme left and neighbor is to its right.
     */

    if (neighbor_index == -1) {
        swap = node;
        node = neighbor;
        neighbor = swap;
    }

    neighbor_insertion_index = neighbor->frame.node.num_keys;
    /* In a nonleaf, ...
     */

    if (!node->frame.node.is_leaf) {

        /* Append k_prime.
         */

        neighbor->frame.node.internal[neighbor_insertion_index].key = k_prime;
        neighbor->frame.node.num_keys++;

        n_end = node->frame.node.num_keys;

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++)  {
            neighbor->frame.node.internal[i] = node->frame.node.internal[j];
            neighbor->frame.node.num_keys++;
            node->frame.node.num_keys--;
        }

        neighbor->frame.node.internal[neighbor_insertion_index].pointer = node->frame.node.right_or_indexing;
    }

    /* In a leaf, ...
     */

    else {
        // TODO: is it possible situation?
        // printf("IS IT POSSIBLE?\n");
        for (i = neighbor_insertion_index, j = 0; j < node->frame.node.num_keys; i++, j++) {
            neighbor->frame.node.records[i] = node->frame.node.records[j];
            neighbor->frame.node.num_keys++;
        }
        neighbor->frame.node.right_or_indexing = node->frame.node.right_or_indexing;
    }

    if (neighbor_index == -1) {
        // TODO: case check...
        // printf("COALESCE at leaf at neighbor_idx == -1");
        parent->frame.node.right_or_indexing = neighbor->page_num; // -1
    }

    buffer_free_page(header, node);
    ret_val = delete_entry(header, parent, k_prime);
    return ret_val;
}

int redistribute_nodes(buffer_frame_t* header, buffer_frame_t* node, buffer_frame_t* neighbor, int neighbor_index, int k_prime_index, int64_t k_prime) {

    // TODO: in deleyed merge, is it possible situation?
    //printf("IS IT POSSIBLE? #redistribution\n");

    int i;
    // TODO: parent를 굳이.. 코앓레쓰에서도 마찬가지 상황.
    buffer_frame_t* parent = get_buf_frame(header->table_id, node->frame.node.parent);

    /* Case 1: n has a neighbor to the left. (common...)
     * Pull the neighbor's last key-value pair over
     * from the neighbor's right end to n's left end.
     */
    if (neighbor_index != -1) {

        if (!node->frame.node.is_leaf) { //nonleaf...
            node->frame.node.internal[node->frame.node.num_keys].key = k_prime;
            node->frame.node.internal[node->frame.node.num_keys].pointer = node->frame.node.right_or_indexing;
            parent->frame.node.internal[k_prime_index].key = neighbor->frame.node.internal[parent->frame.node.num_keys - 1].key;
      
            node->frame.node.right_or_indexing = neighbor->frame.node.internal[neighbor->frame.node.num_keys - 1].pointer;
        }

        else {
            for (i = node->frame.node.num_keys; i > 0; i--)
                node->frame.node.records[i] = node->frame.node.records[i - 1];            
        
            node->frame.node.records[0] = neighbor->frame.node.records[neighbor->frame.node.num_keys - 1];
            parent->frame.node.internal[k_prime_index].key = node->frame.node.records[0].key;
        }
    }

    /* Case 2: n is the leftmost child.
     * Take the...
     */

    else {
        if (node->frame.node.is_leaf) {
            node->frame.node.records[node->frame.node.num_keys] = neighbor->frame.node.records[0];
            parent->frame.node.internal[k_prime_index].key = neighbor->frame.node.records[1].key;

            for (i = 0; i < neighbor->frame.node.num_keys - 1; i++) {
                neighbor->frame.node.records[i] = neighbor->frame.node.records[i + 1];            
            }
        }

        else { //nonleaf...
            node->frame.node.internal[node->frame.node.num_keys].key = k_prime;
            node->frame.node.internal[node->frame.node.num_keys].pointer = neighbor->frame.node.right_or_indexing;
            parent->frame.node.internal[k_prime_index].key = neighbor->frame.node.internal[0].key;
      
            neighbor->frame.node.right_or_indexing = neighbor->frame.node.internal[0].pointer;
            for (i = 0; i < neighbor->frame.node.num_keys - 1; i++) {
                neighbor->frame.node.internal[i] = neighbor->frame.node.internal[i + 1];
            }               
        }
    }
    node->frame.node.num_keys++;
    neighbor->frame.node.num_keys--;

    unpin(parent);
    return 0;
}
