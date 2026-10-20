# GNU FriBidi 1.0.12

Unmodified complete upstream source archive, including its generated Unicode
tables, build system, copyright notices and LGPL license:

https://github.com/fribidi/fribidi/releases/download/v1.0.12/fribidi-1.0.12.tar.xz

SHA256: `0cd233f97fc8c67bb3ac27ce8440def5d3ffacf516765b91c2cc654498293495`

`COPYING` is copied from that archive. No FriBidi source changes are applied.
PaperNX compiles this source into a separate static library with
`scripts/build-fribidi.sh`; it never depends on an untracked SDK copy. Use the
same script after replacing/modifying the source archive (and updating its
expected checksum) to rebuild/relink with a modified library. Application source
and relinking instructions are in this repository and its tagged source ZIP.

The source archive and this notice are included in the corresponding application
source download; the separate license ZIP also contains the LGPL notice.
