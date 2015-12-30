#!/bin/bash
./som_rec \
--input-network   som32.net \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som32_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p001-006-mfccs.ctl \
--train           \
--randomize-set   \
--max-epochs      50 \
--mse-error-file  som32.mse \
--log-file        som32.log
