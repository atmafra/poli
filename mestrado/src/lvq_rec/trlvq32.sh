#!/bin/bash
./lvq_rec \
--input-network ../data/speech_db/networks/lvq32.net \
--output-network ../data/speech_db/networks/trained/lvq32/lvq32.trained.net \
--class-table ../data/speech_db/speakers.ctl \
--train-table ../data/speech_db/frmaps/som32/p101-210-maps.ctl \
--test-table ../data/speech_db/frmaps/som32/p001-006-maps.ctl \
--train \
--randomize-set \
--error-file lvq.mse.log \
--train-matrix ../data/speech_db/networks/trained/lvq32/lvq.train.txt \
--test-matrix ../data/speech_db/networks/trained/lvq32/lvq.test.txt \
--max-epochs 200
