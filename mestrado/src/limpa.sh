#!/bin/bash
find . -name '*.[c,h]~' -exec rm -f '{}' ';'
