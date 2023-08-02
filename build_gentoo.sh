#!/bin/bash
rm -r ksp_plugin_adapter/bin
rm -r ksp_plugin_adapter/obj
make clean
./install_deps.sh
make -j32
make -j32 adapter
make -j32 release
