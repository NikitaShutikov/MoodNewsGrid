FROM archlinux

RUN pacman -Syu --noconfirm && pacman -S neovim clang git gcc make cmake jsoncpp util-linux zlib mariadb tree-sitter-cli tmux jq tinyxml2 wl-clipboard cpp-httplib lldb yaml-cpp llama-cpp --noconfirm

WORKDIR /deps
RUN git clone --depth 1 https://github.com/drogonframework/drogon && \
	cd drogon && git submodule update --init && \
	mkdir build && cd build && \
	cmake -DCMAKE_BUILD_TYPE=Release .. && make && make install

WORKDIR /app
COPY . .

# ADD https://huggingface.co/TheBloke/saiga_mistral_7b-GGUF/resolve/main/saiga_mistral_7b.Q4_K_M.gguf /models/saiga_mistral_7b.Q4_K_M.gguf

