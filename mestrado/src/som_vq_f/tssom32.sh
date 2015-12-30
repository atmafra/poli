#!/bin/bash
./som_vq \
--input-network ../data/speech_db/networks/trained/som32/som32.trained.net \
--input-table ../data/speech_db/mfccs/silence.ctl \
--states-dir ../data/speech_db/slists/som32 \
--map-dir ../data/speech_db/frmaps/som32
