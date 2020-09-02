FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /home
RUN apt-get update && apt-get -y --no-install-recommends install g++ make zip libsdl2-dev
COPY . .
CMD ["make","dist"]
