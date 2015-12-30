#!/bin/bash
./som_rec \
--input-network   som16.net \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som16_alcaim3.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--train           \
--randomize-set   \
--max-epochs      1 \
--mse-error-file  mse/som16_alcaim3.mse \
--log-file        log/som16_alcaim3.log
