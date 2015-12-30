#!/bin/bash
./lvq_rec \
--input-network ../data/speech_db/networks/lvq64.net \
--output-network ../data/speech_db/networks/trained/lvq64/lvq64.trained.net \
--class-table ../data/speech_db/speakers.ctl \
--train-table ../data/speech_db/frmaps/som64/p101-106-maps.ctl \
--test-table ../data/speech_db/frmaps/som64/zip.ctl \
--train \
--randomize-set \
--error-file lvq.mse.log \
--train-matrix ../data/speech_db/networks/trained/lvq64/lvq.train.txt \
--test-matrix ../data/speech_db/networks/trained/lvq64/lvq.test.txt \
--max-epochs 1000
