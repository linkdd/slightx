#include <klibc/archive/ustar.h>
#include <klibc/mem/align.h>
#include <klibc/mem/bytes.h>
#include <klibc/mem/arena.h>
#include <klibc/assert.h>


const ustar_header *ustar_lookup(const_span archive, str path) {
  assert(archive.data != NULL && archive.size > 0);

  constexpr usize pathbufsz          = 512;
  char            pathbuf[pathbufsz] = {};
  arena           scratch            = {};
  allocator       a                  = arena_allocator(&scratch);
  arena_init(&scratch, make_span(pathbuf, pathbufsz));

  const ustar_header *hdr = (const ustar_header *) archive.data;

  while (
    memcmp("ustar", hdr->ustar_id,  sizeof(hdr->ustar_id))  == 0 &&
    memcmp("00",    hdr->ustar_ver, sizeof(hdr->ustar_ver)) == 0
  ) {
    arena_reset(&scratch);

    str abspath = ustar_entry_filename(hdr, a);
    if (abspath.data[abspath.length - 1] == '/') {
      abspath.length--;
    }

    if (str_equal(path, abspath)) {
      return hdr;
    }

    usize entrysize = align_size_up((usize)ustar_entry_size(hdr), USTAR_BLOCK_SIZE);
    hdr = (const ustar_header *)((uptr)hdr + sizeof(ustar_header) + entrysize);
  }

  return NULL;
}


static u64 oct2bin(usize len, const u8 s[static len]) {
  u64 n = 0;

  for (usize i = 0; i < len; ++i) {
    u8 c = s[i];

    n *= 8;
    n += c - '0';
  }

  return n;
}


u16 ustar_entry_mode(const ustar_header *hdr) {
  assert(hdr != NULL);

  return (u16)oct2bin(sizeof(hdr->mode), hdr->mode);
}


u32 ustar_entry_uid(const ustar_header *hdr) {
  assert(hdr != NULL);

  return (u32)oct2bin(sizeof(hdr->uid), hdr->uid);
}


u32 ustar_entry_gid(const ustar_header *hdr) {
  assert(hdr != NULL);

  return (u32)oct2bin(sizeof(hdr->gid), hdr->gid);
}


u64 ustar_entry_size(const ustar_header *hdr) {
  assert(hdr != NULL);

  return oct2bin(sizeof(hdr->size), hdr->size);
}


u64 ustar_entry_mtime(const ustar_header *hdr) {
  assert(hdr != NULL);

  return oct2bin(sizeof(hdr->mtime), hdr->mtime);
}


ustar_filetype ustar_entry_type(const ustar_header *hdr) {
  assert(hdr != NULL);

  return (ustar_filetype)(hdr->type[0] - '0');
}


const_span ustar_entry_data(const ustar_header *hdr) {
  assert(hdr != NULL);

  return make_const_span(
    hdr->data,
    (usize)ustar_entry_size(hdr)
  );
}


str ustar_entry_filename(const ustar_header *hdr, allocator a) {
  assert(hdr != NULL);

  str prefix   = strview_from_buffer(hdr->prefix,   sizeof(hdr->prefix));
  str filename = strview_from_buffer(hdr->filename, sizeof(hdr->filename));

  str abspath = {};
  if (prefix.length > 0) {
    abspath = str_format(a, "%s/%s", prefix, filename);
  }
  else {
    abspath = str_clone(a, filename);
  }

  return abspath;
}


str ustar_entry_linkname(const ustar_header *hdr, allocator a) {
  assert(hdr != NULL);

  return str_clone(a, strview_from_buffer(hdr->linkname, sizeof(hdr->linkname)));
}


str ustar_entry_username(const ustar_header *hdr, allocator a) {
  assert(hdr != NULL);

  return str_clone(a, strview_from_buffer(hdr->username, sizeof(hdr->username)));
}


str ustar_entry_groupname(const ustar_header *hdr, allocator a) {
  assert(hdr != NULL);

  return str_clone(a, strview_from_buffer(hdr->groupname, sizeof(hdr->groupname)));
}
