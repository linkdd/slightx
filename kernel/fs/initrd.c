#include <fs/initrd.h>
#include <mem/kheap.h>
#include <util.h>

struct initrd_header_t *initrd_header;      /* The header */
struct initrd_file_header_t *file_headers;  /* A list of file headers. */
struct fs_node *initrd_root;                /* Our root directory node. */
struct fs_node *initrd_dev;                 /* We also add a directory node for /dev, so we can mount devfs later on. */
struct fs_node *root_nodes;                 /* List of file nodes. */
int nroot_nodes;                            /* Number of files nodes. */

struct dirent dirent;

static uint32_t initrd_read (struct fs_node *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
    struct initrd_file_header_t header = file_headers[node->inode];

    if (offset > header.length)
        return 0;
    if (offset + size > header.length)
        size = header.length - offset;

    memcpy (buffer, (uint8_t *) (header.offset + offset), size);
    return size;
}

static struct dirent *initrd_readdir (struct fs_node *node, uint32_t index)
{
    if (node == initrd_root && index == 0)
    {
        strcpy (dirent.name, "dev");
        dirent.name[3] = 0;
        dirent.ino = 0;
        return &dirent;
    }

    if (index - 1 >= nroot_nodes)
        return 0;

    strcpy (dirent.name, root_nodes[index - 1].name);
    dirent.name[strlen (root_nodes[index - 1].name)] = 0;
    dirent.ino = root_nodes[index - 1].inode;
    return &dirent;
}

static struct fs_node *initrd_finddir (struct fs_node *node, char *name)
{
    int i;

    if (node == initrd_root && !strcmp (name, "dev"))
        return initrd_dev;

    for (i = 0; i < nroot_nodes; ++i)
        if (!strcmp (name, root_nodes[i].name))
            return &root_nodes[i];

    return 0;
}

struct fs_node *init_initrd (uint32_t location)
{
    int i;

    /* Initialise the main and file header pointers and populate the root directory. */
    initrd_header = (struct initrd_header_t *) location;
    file_headers = (struct initrd_file_header_t *) (location + sizeof (struct initrd_header_t));

    /* Initialise the root directory. */
    initrd_root          = (struct fs_node *) kmalloc (sizeof (struct fs_node));
    strcpy (initrd_root->name, "initrd");
    initrd_root->mask    = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->length = 0;
    initrd_root->flags   = FS_DIRECTORY;
    initrd_root->read    = 0;
    initrd_root->write   = 0;
    initrd_root->open    = 0;
    initrd_root->close   = 0;
    initrd_root->readdir = &initrd_readdir;
    initrd_root->finddir = &initrd_finddir;
    initrd_root->ptr     = 0;
    initrd_root->impl    = 0;

    /* Initialise the /dev directory (required!) */
    initrd_dev          = (struct fs_node *) kmalloc (sizeof (struct fs_node));
    strcpy(initrd_dev->name, "dev");
    initrd_dev->mask    = initrd_dev->uid = initrd_dev->gid = initrd_dev->inode = initrd_dev->length = 0;
    initrd_dev->flags   = FS_DIRECTORY;
    initrd_dev->read    = 0;
    initrd_dev->write   = 0;
    initrd_dev->open    = 0;
    initrd_dev->close   = 0;
    initrd_dev->readdir = &initrd_readdir;
    initrd_dev->finddir = &initrd_finddir;
    initrd_dev->ptr     = 0;
    initrd_dev->impl    = 0;

    root_nodes = (struct fs_node *) kmalloc (sizeof (struct fs_node ) * initrd_header->nfiles);
    nroot_nodes = initrd_header->nfiles;

    /* For every files... */
    for (i = 0; i < initrd_header->nfiles; ++i)
    {
        /* Edit the file's header - currently it holds the file offset
         * relative to the start of the ramdisk. We want it relative to the start
         * of memory.
         */

        file_headers[i].offset += location;
        /* Create a new file node. */
        strcpy (root_nodes[i].name, (char *) &file_headers[i].name);
        root_nodes[i].mask    = root_nodes[i].uid = root_nodes[i].gid = 0;
        root_nodes[i].length  = file_headers[i].length;
        root_nodes[i].inode   = i;
        root_nodes[i].flags   = FS_FILE;
        root_nodes[i].read    = &initrd_read;
        root_nodes[i].write   = 0;
        root_nodes[i].readdir = 0;
        root_nodes[i].finddir = 0;
        root_nodes[i].open    = 0;
        root_nodes[i].close   = 0;
        root_nodes[i].impl    = 0;
    }

    return initrd_root;
}
