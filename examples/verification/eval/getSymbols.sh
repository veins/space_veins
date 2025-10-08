#!/bin/sh

# Dump the symbol table without truncating the output for terminal width
# Eg. 18: 000000000000115d    10 FUNC    WEAK   DEFAULT   15 _ZN7Garbage4funcEi
readelf -sW "$1" | while read -r _ addr _ is_func _ _ _ mangled_name; do
    # We only care about function symbols
    [ "$is_func" = "FUNC" ] || continue

    # Demangle the symbol using c++filt and output the function address, mangled
    # and demangled name
    echo "$addr $mangled_name $(c++filt "$mangled_name")"
done