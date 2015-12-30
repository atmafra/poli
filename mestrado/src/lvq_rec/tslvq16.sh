#!/bin/bash
./lvq_rec \
--input-network ../data/speech_db/networks/trained/lvq16/lvq16.trained.net \
--class-table ../data/speech_db/speakers.ctl \
--test-table ../data/speech_db/trmaps/som16/p201-210-maps.ctl \
--test-matrix ../data/speech_db/networks/trained/lvq16/lvq.test.txt
