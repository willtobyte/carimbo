## Project Setup

This guide provides a simple walkthrough for setting up and building the project. Follow the instructions below to get started.

### Python Environment & Virtualenv

Set up the Python virtual environment:

```shell
uv venv
source .venv/bin/activate
```

### Conan

Install Conan, and configure it:

```shell
uv pip install conan
conan profile detect --force
```

### Add the WebAssembly Profile

Create a Conan profile for WebAssembly:

```shell
cat > ~/.conan2/profiles/webassembly <<EOF
include(default)

[settings]
arch=wasm
os=Emscripten

[tool_requires]
*: emsdk/3.1.73
EOF
```

### Configure & Build

On very first build

```shell
make conan profile=webassembly
```

On next builds

```shell
make build profile=webassembly
```

### Clone a Game Repository

Clone the game repository into a local folder named sandbox:

```shell
gh repo clone willtobyte/slime sandbox
```

### Clone the Playground Web Application

Clone the playground web application repository:

```shell
gh repo clone willtobyte/run
```

### Run the Application

Note: Ensure Docker is running. If Docker is not installed, consider installing [OrbStack](https://orbstack.dev/).

Start the application:

```shell
make run
```

Finally, open [localhost:3000/playground](http://localhost:3000/playground) in your browser.
