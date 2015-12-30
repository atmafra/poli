#!/bin/bash
echo
echo  NOME: MAFRA
echo
./som_vq_f \
--input-network som32l01.net \
--test-table ../data/speech_db/mfccs/nomel01.ctl

./som_vq_f \
--input-network som32l11.net \
--test-table ../data/speech_db/mfccs/nomel01.ctl

echo
echo  NOME: BETO
echo
./som_vq_f \
--input-network som32l01.net \
--test-table ../data/speech_db/mfccs/nomel11.ctl

./som_vq_f \
--input-network som32l11.net \
--test-table ../data/speech_db/mfccs/nomel11.ctl
