FROM ubuntu:26.04
ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /home
RUN apt-get update && apt-get -y --no-install-recommends install g++ make git libsdl2-dev libjack-jackd2-dev
COPY . .
CMD ["make","dist"]
