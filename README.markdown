# Pls, dexter

A simple cache that can be backed by a file.

**I do not recommend using it currently.**

I wanted a cache for a pastebin service satisfying the following criteria:

* Fixed memory usage,

* Optional persistence,

* A reasonable strategy for a pastebin. For example least recently used or last
  inserted.

## How does it work?

A fixed size chunk is allocated with mmap, optionally backed by a file. The
cache works like a linked list. An entry is inserted after the tail. Depending
on the state of the cache it's either

* inserted after the tail with no further work, or

* inserted after the tail after a number of entries after the tail has been
  'freed', that is, removed from the linked list.

* If there's not enough room after the tail we insert it at the beginning of
  the chunk of memory, 'freeing' at least one entry.

## What needs to be done?

Some sort of index would be nice. Currently, when looking for an entry, the
strategy is just to start at the beginning of the chunk and traverse the list.
This could be improved, for example by

* Adding an in-memory index. This needs to be rebuilt everytime the cache is opened.

* Adding a most recently used list. This is fairly simple to do, and probably
  fits well with how pastes are accessed.
