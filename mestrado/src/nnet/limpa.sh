#!/bin/bash
find . -name '*.c~' -exec echo rm -f '{}' ';'
