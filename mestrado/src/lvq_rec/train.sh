#!/bin/bash
./lvq_rec \
--input-network lvq.net \
--output-network lvq.trained.net \
--class-table ../data/speech_db/speakers.ctl \
--train-table ../data/numeros/trmaps/p101-106-maps.ctl \
--test-table ../data/numeros/trmaps/p201-210-maps.ctl \
--train \
--randomize-set \
--error-file lvq.mse.log \
--train-matrix lvq.train.txt \
--test-matrix lvq.test.txt \
--max-epochs 500
