#!/bin/bash
find . -name '*.[c,h]' -exec wc '{}' ';'
