FROM archlinux as dev

RUN pacman -Syu --noconfirm && pacman -S neovim clang git gcc make cmake jsoncpp util-linux zlib mariadb tree-citter-cli tmux --noconfirm

WORKDIR /deps
RUN git clone --depth 1 https://github.com/drogonframework/drogon && \
	cd drogon && git submodule update --init && \
	mkdir build && cd build && \
	cmake -DCMAKE_BUILD_TYPE=Release .. && make && make install

WORKDIR /app
COPY . .

FROM debian:trixie as prod
