#pragma once

#include <klibc/types.h>

#include <klibc/mem/alloc.h>
#include <klibc/mem/str.h>
#include <klibc/mem/span.h>


static constexpr size_t USTAR_BLOCK_SIZE = 512;


typedef struct ustar_header ustar_header;
struct ustar_header {
  u8 filename [100];
  u8 mode     [  8];
  u8 uid      [  8];
  u8 gid      [  8];
  u8 size     [ 12];
  u8 mtime    [ 12];
  u8 checksum [  8];
  u8 type     [  1];
  u8 linkname [100];

  u8 ustar_id [  6];
  u8 ustar_ver[  2];

  u8 username [ 32];
  u8 groupname[ 32];

  u8 devmajor [  8];
  u8 devminor [  8];

  u8 prefix   [155];

  u8 _padding_[ 12];
  u8 data     [   ];
};

static_assert(sizeof(ustar_header) == USTAR_BLOCK_SIZE);

typedef enum : u8 {
  USTAR_FILETYPE_NORMAL = 0,
  USTAR_FILETYPE_HARDLINK,
  USTAR_FILETYPE_SYMLINK,
  USTAR_FILETYPE_CHARDEV,
  USTAR_FILETYPE_BLOCKDEV,
  USTAR_FILETYPE_DIRECTORY,
  USTAR_FILETYPE_PIPE,
} ustar_filetype;


const ustar_header *ustar_lookup(const_span archive, str path);

u16            ustar_entry_mode (const ustar_header *hdr);
u32            ustar_entry_uid  (const ustar_header *hdr);
u32            ustar_entry_gid  (const ustar_header *hdr);
u64            ustar_entry_size (const ustar_header *hdr);
u64            ustar_entry_mtime(const ustar_header *hdr);
ustar_filetype ustar_entry_type (const ustar_header *hdr);
const_span     ustar_entry_data (const ustar_header *hdr);

str ustar_entry_filename (const ustar_header *hdr, allocator a);
str ustar_entry_linkname (const ustar_header *hdr, allocator a);
str ustar_entry_username (const ustar_header *hdr, allocator a);
str ustar_entry_groupname(const ustar_header *hdr, allocator a);
