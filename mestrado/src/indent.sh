#!/bin/bash
find . -name '*.[c,h]' -exec indent -nut '{}' ';'
find . -name '*.[c,h]~' -exec rm -f '{}' ';'
