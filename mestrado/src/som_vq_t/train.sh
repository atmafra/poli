#!/bin/bash
./som_vq \
--input-network ../data/networks/som16.net \
--output-network ../data/networks/som16.trained.net \
--input-table ../data/speech_db/mfccs/p101-106-mfccs.ctl \
--states-dir ../data/speech_db/slists \
--map-dir ../data/speech_db/trmaps \
--mse-error-file mse.log \
--max-epochs 1 \
--train \
--randomize-set
