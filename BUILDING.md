# Build

**Python & virtualenv**

```shell
uv venv
source .venv/bin/activate
```

**Conan & hooks pre-commit**

```shell
uv pip install -r requirements.txt
pre-commit install
conan profile detect --force
```

**Adding the WebAssembly profile**

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

**Configure the project**

```shell
make -f Makefile.webassembly configure
```

**Building the project**

```shell
make -f Makefile.webassembly build
```

**Cloning a game**

```shell
gh repo clone willtobyte/slime sandbox
```

**Cloning the playground web application**

```shell
gh repo clone willtobyte/run
```

**Running the application**
**Note:** Ensure Docker is running. If not, install [OrbStack](https://orbstack.dev/).

```shell
make run
```

Finally, open [http://localhost:3000/playground](http://localhost:3000/playground) in your browser.
