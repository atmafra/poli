#!/bin/bash
./som_vq \
--input-network ../data/networks/som16.net \
--output-network ../data/networks/som16.trained.net \
--input-table ../data/speech_db/mfccs/p201-210-mfccs.ctl \
--states-dir ../data/speech_db/slists \
--map-dir ../data/speech_db/trmaps \
--mse-error-file mse.log \
--max-epochs 100 \
--train \
--randomize-set
