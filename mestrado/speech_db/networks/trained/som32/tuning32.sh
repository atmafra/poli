#!/bin/bash
./som_rec \
--input-network   som32.tuning.net \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/tuning32.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--train           \
--randomize-set   \
--max-epochs      25 \
--mse-error-file  tuning32.mse \
--log-file        tuning32.log
#--test-table      ../data/speech_db/mfccs/p001-006-mfccs.ctl \
