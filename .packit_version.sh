#!/bin/sh

git describe --tags --match "*.*"  --abbrev=0 | sed -ne 's/^libmodulemd-//p'
