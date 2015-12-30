#!/bin/bash

# SOM 16
./som_rec \
--input-network   som16.net \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som16_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--train           \
--randomize-set   \
--max-epochs      150 \
--mse-error-file  mse/som16_alcaim1.mse \
--log-file        log/som16_alcaim1.log

# SOM 25
./som_rec \
--input-network   som25.net \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som25_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--train           \
--randomize-set   \
--max-epochs      150 \
--mse-error-file  mse/som25_alcaim1.mse \
--log-file        log/som25_alcaim1.log

# SOM 36
./som_rec \
--input-network   som36.net \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--train           \
--randomize-set   \
--max-epochs      100 \
--mse-error-file  mse/som36_alcaim1.mse \
--log-file        log/som36_alcaim1.log

# SOM 64
./som_rec \
--input-network   som64.net \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som64_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--train           \
--randomize-set   \
--max-epochs      100 \
--mse-error-file  mse/som64_alcaim1.mse \
--log-file        log/som64_alcaim1.log

