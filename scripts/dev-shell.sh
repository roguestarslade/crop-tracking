#!/bin/bash
docker run -it \
  -v $HOME/.ssh:/root/.ssh:ro \
  -v $(pwd):/crop-tracking \
  crop-tracking-dev bash
