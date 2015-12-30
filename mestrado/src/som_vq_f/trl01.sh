#!/bin/bash
./som_vq_f \
--input-network som32l.net \
--output-network som32l01.net \
--train-table ../data/speech_db/mfccs/l01-p001-006-mfccs.ctl \
--mse-error-file msel01.log \
--max-epochs 400 \
--train \
--randomize-set
