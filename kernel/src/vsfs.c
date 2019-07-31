//Very Simple File System
#include <common.h>
#include <klib.h>
#include <devices.h>

extern inodeops_t blkfs_inode_op;

inode_t* create_inode(filesystem_t* fs,inode_t *parent_node, const char* name,int mode,int types){
    inode_t* new_inode = pmm->alloc(sizeof(inode_t));
    new_inode->parent = parent_node;
    new_inode->next = new_inode->child = NULL;
    strcpy(new_inode->name,name);
    new_inode->fs = fs;
    new_inode->content =NULL;
    new_inode->mode = mode;
    new_inode->refcnt = 0;
    new_inode->ops =  &blkfs_inode_op;
    new_inode->types = types;
    if(!strcmp(parent_node->name,"dev")){
        new_inode->ptr = (void*) dev_lookup(name);
    }
    if(parent_node->child == NULL){
        parent_node->child = new_inode;
    }else{
        inode_t* scan = parent_node->child;
        while(scan->next!=NULL){
            if(!strcmp(scan->name,name)){
                return NULL;
            }
            scan = scan->next;
        }
        //printf("%s 's next is %s\n",scan->name,new_inode->name);
        scan->next = new_inode;
    }
    return new_inode;
}
    
int free_inode(inode_t* f_inode){
    inode_t* parent_node = f_inode->parent;
    if(parent_node->child == f_inode){
        parent_node->child = f_inode->next;
        pmm->free((void*)f_inode);
        return 0;
    }
    inode_t *scan = f_inode->parent->child;
    while (scan->next!=f_inode)
    {
        if(scan->next ==  NULL){
            printf("ERROR: PLEASE CHECK THE INODE LIST\n");
            assert(0);
        }
        scan = scan->next; 
    }
    scan->next = f_inode->next;
    pmm->free((void*)f_inode);
    return 0;
}