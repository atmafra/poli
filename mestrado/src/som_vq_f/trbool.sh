#!/bin/bash
./som_vq_f \
--input-network ../data/boolean/networks/som_bool.net \
--output-network ../data/boolean/networks/som_bool.trained.net \
--input-table ../data/boolean/bool.ctl \
--states-dir ../data/boolean/slists \
--map-dir ../data/boolean/trmaps \
--mse-error-file ../data/boolean/networks/som_bool.mse.log \
--max-epochs 5000 \
--train \
--randomize-set
