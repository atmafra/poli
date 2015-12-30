#!/bin/bash
./som_vq \
--input-network ../data/speech_db/networks/trained/som16/som16.trained.net \
--input-table ../data/speech_db/mfccs/silence.ctl \
--states-dir ../data/speech_db/slists/som16 \
--map-dir ../data/speech_db/frmaps/som16
