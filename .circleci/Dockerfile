FROM ubuntu:18.04
RUN export DEBIAN_FRONTEND=noninteractive && apt-get update && apt-get install -y build-essential tzdata wget cmake git
RUN wget -q -O - https://files.viva64.com/etc/pubkey.txt | apt-key add - 
RUN wget -O /etc/apt/sources.list.d/viva64.list https://files.viva64.com/etc/viva64.list
RUN apt-get update -qq && apt-get install -qq pvs-studio
RUN git clone -b 2.0 https://github.com/viva64/how-to-use-pvs-studio-free
RUN cd how-to-use-pvs-studio-free && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release ..  && \
    make && \
    make install