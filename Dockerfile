# Dockerfile for webserv_exp project
# Base image: Ubuntu latest
FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive  

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        valgrind \
        curl \
        net-tools \
        python3 \
        python3-requests && \
		rm -rf /var/lib/apt/lists/*

WORKDIR /mnt/project

CMD ["/bin/bash"]
