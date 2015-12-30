#!/bin/bash
./lvq_rec \
--input-network ../data/numeros/networks/lvq_atm.net \
--output-network ../data/numeros/networks/trained/lvq_atm.trained.net \
--train-table ../data/numeros/trmaps/atm-maps.ctl \
--test-table ../data/numeros/trmaps/atm-maps.ctl \
--class-table ../data/numeros/atm.ctl \
--train-matrix lvq.train.txt \
--test-matrix lvq.test.txt \
--error-file lvq.mse.log \
--max-epochs 200 \
--randomize-set \
--train
