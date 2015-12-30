#!/bin/bash
./lvq_rec \
--input-network ../data/speech_db/networks/lvq16.net \
--output-network ../data/speech_db/networks/trained/lvq16/lvq16.trained.net \
--class-table ../data/speech_db/speakers.ctl \
--test-table ../data/speech_db/frmaps/som16/p201-210-maps.ctl \
--train-table ../data/speech_db/frmaps/som16/p101-106-maps.ctl \
--train \
--randomize-set \
--error-file lvq.mse.log \
--train-matrix ../data/speech_db/networks/trained/lvq16/lvq.train.txt \
--test-matrix ../data/speech_db/networks/trained/lvq16/lvq.test.txt \
--max-epochs 200
