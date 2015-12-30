#!/bin/bash
./lvq_rec \
--input-network lvq.trained.net \
--class-table ../data/speech_db/speakers.ctl \
--test-table ../data/numeros/trmaps/p101-106-maps.ctl \
--test-matrix lvq.test.txt
