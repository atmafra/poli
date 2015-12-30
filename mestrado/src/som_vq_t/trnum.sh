#!/bin/bash
./som_vq \
--input-network ../data/numeros/networks/som_atm.net \
--output-network ../data/numeros/networks/trained/som_atm.trained.net \
--input-table ../data/numeros/mfccs/atm-mfccs.ctl \
--states-dir ../data/numeros/slists \
--map-dir ../data/numeros/trmaps \
--mse-error-file ../data/numeros/networks/som_atm_mse.log \
--max-epochs 100 \
--train \
--randomize-set
