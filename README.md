<p align="center">
  <img width="96" height="96" src="./docs/logo.png" alt="logo">
</p>

<h1 align="center">ttyny</h1>

<p align="center">
An AI-powered game engine to play text adventures in your terminal.
</p>

`ttyny` is a lightweight engine for playing text adventures, inspired by 
classics like Infocom's [Zork](https://en.wikipedia.org/wiki/Zork).

Powered by language models, `ttyny` understands natural language input and 
generates dynamic responses, making every adventure a unique experience.

> [!IMPORTANT]
> This is heavily in developement and far from its final form. 
> Ideas and contributions are welcome!

## Usage

```sh
ttyny ./my-story.json
```

Check out [`assets`](./assets) for some example stories.

## Writing a story

Writing a story is as simple as creating a JSON file that represents the world
in which the adventure takes place.

See [`stories.md`](./docs/stories.md) for further details.

## Development

This project is written in C11 and oly targets POSIX systems.

```bash
# Clone the repository
git clone https://github.com/shikaan/ttyny.git
cd ttyny

# You need some Small Language Models to run the game
# Default configuration is with Qwen2.5-1.5B.
# This is only needed once
curl -o models/qwen2.5-1.5b-instruct-q4_k_m.gguf -LO "https://huggingface.co/Qwen/Qwen2.5-1.5B-Instruct-GGUF/resolve/main/qwen2.5-1.5b-instruct-q4_k_m.gguf?download=true"

# Run unit tests
make test

# Run timing tests
make time

# Run snapshot tests
make snap

# Build a test binary
make all

# Build with logging (2 = debug, 1 = info, 0 = error)
make LOG_LEVEL=2 all

# Build production binary
make BUILD_TYPE=release all
```
