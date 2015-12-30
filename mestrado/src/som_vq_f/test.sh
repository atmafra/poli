#!/bin/bash
./som_vq \
--input-network ../data/networks/som16.trained.net \
--input-table ../data/speech_db/mfccs/p100-200-mfccs.ctl \
--states-dir ../data/numeros/slists \
--map-dir ../data/numeros/trmaps
