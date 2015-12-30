#!/bin/bash
./som_vq \
--input-network ../data/numeros/networks/trained/som_atm.trained.net \
--input-table ../data/numeros/mfccs/atm-mfccs.ctl \
--states-dir ../data/numeros/slists \
--map-dir ../data/numeros/trmaps \
--randomize-set
