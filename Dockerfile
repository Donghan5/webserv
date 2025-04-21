FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y build-essential valgrind curl net-tools && apt-get install -y python3 && rm -rf /var/lib/apt/lists/*

WORKDIR /mnt/project

CMD ["/bin/bash"]
