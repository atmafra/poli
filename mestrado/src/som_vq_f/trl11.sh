#!/bin/bash
./som_vq_f \
--input-network som32l.net \
--output-network som32l11.net \
--train-table ../data/speech_db/mfccs/l11-p001-006-mfccs.ctl \
--mse-error-file msel11.log \
--max-epochs 400 \
--train \
--randomize-set
