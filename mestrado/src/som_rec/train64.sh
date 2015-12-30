#!/bin/bash

# SPOLTECH
./som_rec \
--input-network   som64.net \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som64_spoltech.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--train           \
--randomize-set   \
--max-epochs      100 \
--mse-error-file  mse/som64_spoltech.mse \
--log-file        log/som64_spoltech.log

# ALCAIM 1
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


# ALCAIM 2
./som_rec \
--input-network   som64.net \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som64_alcaim2.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--train           \
--randomize-set   \
--max-epochs      100 \
--mse-error-file  mse/som64_alcaim2.mse \
--log-file        log/som64_alcaim2.log

