#!/bin/bash

rm -f log/test36.log

# SPOLTECH
./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_spoltech.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p001-006-mfccs.ctl \
--log-file        log/test36.log

./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_spoltech.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p101-106-mfccs.ctl \
--log-file        log/test36.log

./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_spoltech.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p201-210-mfccs.ctl \
--log-file        log/test36.log

./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_spoltech.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p301-310-mfccs.ctl \
--log-file        log/test36.log

# ALCAIM 1
./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p001-006-mfccs.ctl \
--log-file        log/test36.log

./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p101-106-mfccs.ctl \
--log-file        log/test36.log

./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p201-210-mfccs.ctl \
--log-file        log/test36.log

./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_alcaim1.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p301-310-mfccs.ctl \
--log-file        log/test36.log

# ALCAIM 2
./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_alcaim2.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p001-006-mfccs.ctl \
--log-file        log/test36.log

./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_alcaim2.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p101-106-mfccs.ctl \
--log-file        log/test36.log

./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_alcaim2.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p201-210-mfccs.ctl \
--log-file        log/test36.log

./som_rec \
--network-dir     ../data/speech_db/networks/ \
--class-table     ../data/speech_db/speakers.ctl \
--network-table   ../data/speech_db/networks/spknet_som36_alcaim2.ctl \
--train-dir       ../data/speech_db/mfccs/ \
--test-table      ../data/speech_db/mfccs/p301-310-mfccs.ctl \
--log-file        log/test36.log
