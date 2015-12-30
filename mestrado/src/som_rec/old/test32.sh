#!/bin/bash
./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som32_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p201-210-mfccs.ctl
