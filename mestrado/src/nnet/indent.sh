#!/bin/bash
indent -nut -gnu -fc1 *.c
indent -nut -gnu -fc1 *.h
rm -f *.c~
rm -f *.h~
